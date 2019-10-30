// Copyright (C) 2016-2018 Google Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
////////////////////////////////////////////////////////////////////////////////

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dmlab_load_model.h"
#include "dmlab_recording.h"
#include "dmlab_save_model.h"
#include "../../../deepmind/include/deepmind_context.h"
#include "../../../public/dmlab.h"
#include "../client/client.h"
#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"
#include "../renderercommon/qgl.h"
#include "../renderercommon/tr_common.h"
#include "../sys/sys_local.h"

#define GLE(ret, name, ...) extern name##proc * qgl##name;
QGL_1_1_PROCS;
QGL_1_1_FIXED_FUNCTION_PROCS;
QGL_DESKTOP_1_1_PROCS;
QGL_DESKTOP_1_1_FIXED_FUNCTION_PROCS;
QGL_1_3_PROCS;
QGL_1_5_PROCS;
QGL_2_0_PROCS;
QGL_3_0_PROCS;
#undef GLE

static const double kPixelsPerFrameToDegreesPerMilliseconds = 0.11 * 60 / 1000;

// We define a notion of "external time" for convenience and to avoid rounding
// errors when talking about frame rates. This is because the ioquake3 engine
// counts time increments ("engine time") in integral multiples of milliseconds,
// which cannot represent common reciprocals like 60 fps accurately. For each
// external second passing, 0.96 seconds of engine time pass, leading to the
// following correspondence between frame rate (with respect to "external
// seconds") and frame length (with respect to "engine seconds").
// 120 external fps = 8 engine milliseconds per frame.
// 60 external fps = 16 engine milliseconds per frame.
// 30 external fps = 32 engine milliseconds per frame.
// 20 external fps = 48 engine milliseconds per frame.
// 15 external fps = 64 engine milliseconds per frame.
// Note: Frame rates that are exactly representable (and thus avoid rounding
// errors) are of the form 960 / n for n > 0.
static const double kEngineTimePerExternalTime = 0.96;

static const int kMaxLookAngularVelocity = 512;  // Pixels per frame.

static const char* const kReservedEnginePropertyList = "engine";

enum ActionsEnum {
  kActions_LookLeftRight,
  kActions_LookDownUp,
  kActions_StrafeLeftRight,
  kActions_MoveBackForward,
  kActions_Fire,
  kActions_Jump,
  kActions_Crouch,
};

const char* const kActionNames[] = {
    "LOOK_LEFT_RIGHT_PIXELS_PER_FRAME",  // Angular velocity.
    "LOOK_DOWN_UP_PIXELS_PER_FRAME",     // Angular velocity.
    "STRAFE_LEFT_RIGHT",
    "MOVE_BACK_FORWARD",
    "FIRE",
    "JUMP",
    "CROUCH",
};

enum ObservationsEnum {
  kObservations_RgbInterleaved,
  kObservations_RgbdInterleaved,
  kObservations_RgbPlanar,
  kObservations_RgbdPlanar,
  kObservations_BgrInterleaved,
  kObservations_BgrdInterleaved,
  kObservations_MapFrameNumber,
  kObservations_RgbInterlaced,    // Deprecated.
  kObservations_RgbdInterlaced,   // Deprecated.
};

const char* const kObservationNames[] = {
    "RGB_INTERLEAVED",   //
    "RGBD_INTERLEAVED",  //
    "RGB",               //
    "RGBD",              //
    "BGR_INTERLEAVED",   //
    "BGRD_INTERLEAVED",  //
    "MAP_FRAME_NUMBER",  //
    "RGB_INTERLACED",    //
    "RGBD_INTERLACED",   //
};

typedef enum PixelBufferTypeEnum_e {
  kPixelBufferTypeEnum_Rgb,
  kPixelBufferTypeEnum_Bgr,
  kPixelBufferTypeEnum_Depth,
} PixelBufferTypeEnum;

typedef struct PboData_s {
  GLuint id;  // PBO buffer id allocated by glGenBuffers.
  int size;   // Current size in bytes of assigned data buffer.
} PboData;

typedef struct GamePixelBufferObjects_s {
  PboData rgb, depth, custom_view;  // PBO information storage.
  bool supported;  // If PBO's are supported with the current GL backend.
  bool enabled;    // If PBO rendering is enabled.
} GamePixelBufferObjects;

typedef struct GameContext_s {
  DeepmindContext* dm_ctx;
  int width;
  int height;
  int alt_camera_width;
  int alt_camera_height;
  int image_shape[3];
  unsigned char* image_buffer;
  unsigned char* temp_buffer;  // Holds result from glReadPixels.
  GamePixelBufferObjects pbos;
  char command_line[MAX_STRING_CHARS];
  char runfiles_path[MAX_STRING_CHARS];
  bool first_start;
  bool init_called;
  bool map_loaded;
  int engine_frame_period_msec;  // This is the number of milliseconds to
                                 // advance the engine each frame. If set to
                                 // zero the wall clock is used.
  int step;
  int map_start_frame;            // First frame after warm-up.
  double total_engine_time_msec;  // This is step * engine_frame_period_msec.
  double score;
  dmlabRecordingContext* recording_ctx;
  vmInterpret_t vm_mode;  // Selected VM mode for the game, ui, and client.
  bool is_server;         // Whether this environment acts as a server.
  bool is_client_only;    // Whether this environment is attached to an external
                          // server.
  bool is_connecting;     // Whether the environment is connecting to a client
                          // or server.
  int server_port;        // The port a client will attach to on the server.
  int port;               // The port that this environment advertises on.

  bool use_local_level_cache;
  bool use_global_level_cache;

  DeepMindLabLevelCacheParams level_cache_params;

  int map_frame_number_shape[1];
  double map_frame_number_observation;
  bool is_map_loading;
  bool current_screen_rendered;
} GameContext;

// **** Local helper functions and data **** //

static void* realloc_or_die(void* ptr, size_t n) {
  void* result = realloc(ptr, n);
  if (n > 0 && result == NULL) {
    fputs("Reallocation failure, aborting.\n", stderr);
    abort();
  }
  return result;
}

static bool use_pbo_rendering(GameContext* gc) {
  return gc->pbos.supported && gc->pbos.enabled;
}

// Helper function for creating or updating PBO's based on the provided game
// context. Aborts on error.
static void create_update_pbo_or_die(GameContext* gc) {
  if (!gc->pbos.rgb.id) {
    qglGenBuffers(1, &gc->pbos.rgb.id);
  }
  if (!gc->pbos.depth.id) {
    qglGenBuffers(1, &gc->pbos.depth.id);
  }

  // Check that we successfully created RGB and Depth buffers.
  if (qglGetError() != GL_NO_ERROR) {
    fputs("GL Error creating PBO buffers.\n", stderr);
    abort();
  }

  int rgb_pbo_size = gc->width * gc->height * 3;
  if (gc->pbos.rgb.size < rgb_pbo_size) {
    qglBindBuffer(GL_PIXEL_PACK_BUFFER, gc->pbos.rgb.id);
    qglBufferData(GL_PIXEL_PACK_BUFFER, rgb_pbo_size, NULL, GL_STREAM_READ);
    if (qglGetError() != GL_NO_ERROR) {
      fputs("Failed to generate PBO data buffer.\n", stderr);
      abort();
    }
    gc->pbos.rgb.size = rgb_pbo_size;
    qglBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
  }

  int depth_pbo_size = gc->width * gc->height;
  if (gc->pbos.depth.size < depth_pbo_size) {
    qglBindBuffer(GL_PIXEL_PACK_BUFFER, gc->pbos.depth.id);
    qglBufferData(GL_PIXEL_PACK_BUFFER, depth_pbo_size, NULL, GL_STREAM_READ);

    if (qglGetError() != GL_NO_ERROR) {
      fputs("Failed to generate PBO data buffer.\n", stderr);
      abort();
    }
    gc->pbos.depth.size = depth_pbo_size;
    qglBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
  }
}

// Begins request for provided pixel observation. For PBO rendering, this sends
// async GL command to begin reading pixels off card.
static void request_pixel_observations(GameContext* gc,
                                       PixelBufferTypeEnum type) {
  if (use_pbo_rendering(gc)) {
    create_update_pbo_or_die(gc);
    switch (type) {
      case kPixelBufferTypeEnum_Rgb:
        qglBindBuffer(GL_PIXEL_PACK_BUFFER, gc->pbos.rgb.id);
        qglReadPixels(0, 0, gc->width, gc->height, GL_RGB, GL_UNSIGNED_BYTE, 0);
        break;
      case kPixelBufferTypeEnum_Bgr:
        qglBindBuffer(GL_PIXEL_PACK_BUFFER, gc->pbos.rgb.id);
        qglReadPixels(0, 0, gc->width, gc->height, GL_BGR_EXT, GL_UNSIGNED_BYTE,
                      0);
        break;
      case kPixelBufferTypeEnum_Depth:
        qglBindBuffer(GL_PIXEL_PACK_BUFFER, gc->pbos.depth.id);
        qglReadPixels(0, 0, gc->width, gc->height, GL_DEPTH_COMPONENT,
                      GL_UNSIGNED_BYTE, 0);
        break;
    }
    qglBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
  }
}

// Returns a pointer to the requested pixel observations.
static void* bind_pixel_observation(GameContext* gc, PixelBufferTypeEnum type) {
  if (use_pbo_rendering(gc)) {
    switch (type) {
      case kPixelBufferTypeEnum_Rgb:
      case kPixelBufferTypeEnum_Bgr:
        qglBindBuffer(GL_PIXEL_PACK_BUFFER, gc->pbos.rgb.id);
        break;
      case kPixelBufferTypeEnum_Depth:
        qglBindBuffer(GL_PIXEL_PACK_BUFFER, gc->pbos.depth.id);
        break;
    }
    void* pixel_buffer = qglMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);

    return pixel_buffer;
  } else {
    gc->temp_buffer =
        realloc_or_die(gc->temp_buffer, gc->width * gc->height * 3);
    switch (type) {
      case kPixelBufferTypeEnum_Rgb:
        qglReadPixels(0, 0, gc->width, gc->height, GL_RGB, GL_UNSIGNED_BYTE,
                      gc->temp_buffer);
        break;
      case kPixelBufferTypeEnum_Bgr:
        qglReadPixels(0, 0, gc->width, gc->height, GL_BGR_EXT, GL_UNSIGNED_BYTE,
                      gc->temp_buffer);
        break;
      case kPixelBufferTypeEnum_Depth:
        qglReadPixels(0, 0, gc->width, gc->height, GL_DEPTH_COMPONENT,
                      GL_UNSIGNED_BYTE, gc->temp_buffer);
        break;
    }

    return gc->temp_buffer;
  }
}

static void unbind_pixel_observation(GameContext* gc) {
  if (use_pbo_rendering(gc)) {
    qglUnmapBuffer(GL_PIXEL_PACK_BUFFER);
    qglBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
  }
}

static int first_start(GameContext* gc) {
  DeepmindContext* ctx = gc->dm_ctx;

  Sys_SetDefaultInstallPath(gc->runfiles_path);

  Sys_PlatformInit();
  Sys_Milliseconds();

  const char* dynamic_path = ctx->hooks.get_temporary_folder(ctx->userdata);
  Q_strcat(gc->command_line, sizeof(gc->command_line),
           va(" +set fs_temporarypath \"%s\"", dynamic_path));
  Q_strcat(gc->command_line, sizeof(gc->command_line),
           va(" +set fs_homepath \"%s\"", dynamic_path));

  Q_strcat(gc->command_line, sizeof(gc->command_line),
           va(" +set g_gametype \"%d\"", ctx->hooks.game_type(ctx->userdata)));

  const char* modifiedCommandLine =
      ctx->hooks.replace_command_line(ctx->userdata, gc->command_line);

  Com_Init((char*)modifiedCommandLine);

  NET_Init();
  CON_Init();

  // We assume PBO's are supported iff we can load the glGenBuffers function.
  gc->pbos.supported = qglGenBuffers;

  return 0;
}

static bool make_map(GameContext* gc, const char* next_map) {
  DeepmindContext* ctx = gc->dm_ctx;
  char fullPath[MAX_QPATH];
  fileHandle_t f;
  int len;
  // See if <next_map> already exists.
  FS_Restart(0);
  Com_sprintf(fullPath, sizeof(fullPath), "maps/%s.bsp", next_map);
  len = FS_FOpenFileRead(fullPath, &f, qfalse);
  FS_FCloseFile(f);
  if (len > 0) {
    return true;
  }

  // Look for a source map for generating bsp.
  bool gen_aas = true;
  Com_sprintf(fullPath, sizeof(fullPath), BASEGAME "/maps/%s.map", next_map);
  len = FS_SV_FOpenFileRead(fullPath, &f);

  if (len <= 0) {
    // Try maps_no_ai instead.
    gen_aas = false;
    FS_FCloseFile(f);
    Com_sprintf(fullPath, sizeof(fullPath), BASEGAME "/maps_no_ai/%s.map",
                next_map);
    len = FS_SV_FOpenFileRead(fullPath, &f);
  }
  FS_FCloseFile(f);

  if (len <= 0) {
    // Not found!
    return false;
  }

  // Successfully found a map file.

  // Generate a BSP (and AAS file if requested) and wrap in a PK3.
  ctx->hooks.make_pk3_from_map(ctx->userdata, fullPath, next_map, gen_aas);

  // File sytem needs to know about the new map file.
  FS_Restart(0);
  return true;
}

static void dev_map(GameContext* gc) {
  DeepmindContext* ctx = gc->dm_ctx;
  Cvar_Set("fixedtime", va("%d", gc->engine_frame_period_msec));
  const char* next_map = ctx->hooks.next_map(ctx->userdata);
  if (next_map[0] == '\0') {
    Cmd_ExecuteString("map_restart 0");
    Cmd_ExecuteString("updatecustomitems");
    Com_Frame();
  } else {
    if (!make_map(gc, next_map)) {
      perror(va("Didn't find map '%s'\n", next_map));
      exit(1);
    }
    Cmd_ExecuteString(va("devmap \"%s\"", next_map));
    Com_Frame();
    ctx->hooks.add_bots(ctx->userdata);
  }
}

static int connecting(GameContext* gc) {
  int err = !gc->is_server && clc.state < CA_ACTIVE ? EAGAIN : 0;
  IN_Frame();
  Com_Frame();
  return err;
}

static bool load_map(GameContext* gc) {
  gc->is_map_loading = true;
  dev_map(gc);
  if (!gc->recording_ctx->is_demo) {
    while (connecting(gc) == EAGAIN) {
    }
    // Players join team games in spectator mode. Leave 3 frames for player to
    // join the correct team.
    for (int i = 0; i < 3; ++i) {
      IN_Frame();
      Com_Frame();
    }
  }
  gc->map_start_frame = cls.framecount;

  bool demo_ok = true;
  if (gc->recording_ctx->is_recording) {
    demo_ok &= dmlab_start_recording(gc->recording_ctx);
  }
  if (gc->recording_ctx->is_demo) {
    demo_ok &= dmlab_start_demo(gc->recording_ctx);
  }
  if (demo_ok && gc->recording_ctx->is_video) {
    demo_ok &= dmlab_start_video(gc->recording_ctx);
  }
  if (gc->recording_ctx->error != DMLAB_RECORDING_ERROR_NONE) {
    gc->dm_ctx->hooks.set_error_message(gc->dm_ctx->userdata,
                                        gc->recording_ctx->error_message);
  }
  fflush(stdout);
  return demo_ok;
}

// Return 0 iff successful.
static int parse_int(const char* s, long int* out, DeepmindContext* ctx) {
  errno = 0;
  char* e;
  long int val = strtol(s, &e, 0);
  if (e != s && *e == '\0' && errno == 0) {
    *out = val;
    return 0;
  } else {
    ctx->hooks.set_error_message(ctx->userdata,
                                 va("Invalid int setting %s\n", s));
    return -1;
  }
}

// Return 0 iff successful.
static int parse_double(const char* s, double* out, DeepmindContext* ctx) {
  errno = 0;
  char* e;
  long int val = strtod(s, &e);
  if (e != s && *e == '\0' && errno == 0) {
    *out = val;
    return 0;
  } else {
    ctx->hooks.set_error_message(ctx->userdata,
                                 va("Invalid double arg %s\n", s));
    return -1;
  }
}

// Return 0 iff successful.
static int parse_bool(const char* s, bool* out, DeepmindContext* ctx) {
  if (strcmp(s, "true") == 0) {
    *out = true;
    return 0;
  } else if (strcmp(s, "false") == 0) {
    *out = false;
    return 0;
  } else {
    ctx->hooks.set_error_message(ctx->userdata,
                                 va("Invalid boolean arg must be either "
                                    "\"true\" or \"false\"; actual \"%s\"\n",
                                    s));
    return -1;
  }
}

static DeepmindContext dmlab_context_impl = {};

static DeepmindContext* get_context_once(void) {
  static bool have_context = false;
  if (!have_context) {
    have_context = true;
    return &dmlab_context_impl;
  } else {
    return NULL;
  }
}

// **** DeepmindContext **** //

DeepmindContext* dmlab_context(void) {
  return &dmlab_context_impl;
}

// **** RL Environment implementation **** //

static const char* dmlab_error_message(void* context) {
  GameContext* gc = context;
  DeepmindContext* ctx = gc->dm_ctx;
  return ctx->hooks.error_message(ctx->userdata);
}

static int dmlab_setting(void* context, const char* key, const char* value) {
  GameContext* gc = context;
  DeepmindContext* ctx = gc->dm_ctx;

  if (gc->init_called) {
    ctx->hooks.set_error_message(ctx->userdata,
                                 "'init' has already been called. No further "
                                 "settings can be applied.\n");

    return 1;
  }

  long int v;
  double v_double;
  bool v_bool;

  if (strcmp(key, "levelName") == 0) {
    ctx->hooks.set_level_name(ctx->userdata, value);
    return 0;
  } else if (strcmp(key, "levelDirectory") == 0) {
    ctx->hooks.set_level_directory(ctx->userdata, value);
    return 0;
  } else if (strcmp(key, "width") == 0) {
    int res = parse_int(value, &v, ctx);
    if (res != 0) return res;
    if (v < 4 || v % 4 != 0) {
      gc->dm_ctx->hooks.set_error_message(
          ctx->userdata,
          va("'width' must be a positive multiple of 4. Actual %ld", v));
      return 1;
    }
    gc->width = v;
  } else if (strcmp(key, "height") == 0) {
    int res = parse_int(value, &v, ctx);
    if (res != 0) return res;
    if (v < 4 || v % 4 != 0) {
      gc->dm_ctx->hooks.set_error_message(
          ctx->userdata,
          va("'height' must be a positive multiple of 4. Actual %ld", v));
      return 1;
    }
    gc->height = v;
  } else if (strcmp(key, "server") == 0) {
    int res = parse_bool(value, &v_bool, ctx);
    if (res != 0) return res;
    gc->is_server = v_bool;
  } else if (strcmp(key, "client") == 0) {
    int res = parse_bool(value, &v_bool, ctx);
    if (res != 0) return res;
    gc->is_client_only = v_bool;
  } else if (strcmp(key, "hasAltCameras") == 0) {
    int res = parse_bool(value, &v_bool, ctx);
    if (res != 0) return res;
    ctx->hooks.set_has_alt_cameras(ctx->userdata, v_bool);
    return 0;
  } else if (strcmp(key, "maxAltCameraWidth") == 0) {
    int res = parse_int(value, &v, ctx);
    if (res != 0) return res;
    if (v >= 0) gc->alt_camera_width = v;
    return 0;
  } else if (strcmp(key, "maxAltCameraHeight") == 0) {
    int res = parse_int(value, &v, ctx);
    if (res != 0) return res;
    if (v >= 0) gc->alt_camera_height = v;
    return 0;
  } else if (strcmp(key, "localLevelCache") == 0) {
    int res = parse_bool(value, &v_bool, ctx);
    if (res != 0) return res;
    gc->use_local_level_cache = v_bool;
  } else if (strcmp(key, "globalLevelCache") == 0) {
    int res = parse_bool(value, &v_bool, ctx);
    if (res != 0) return res;
    gc->use_global_level_cache = v_bool;
  } else if (strcmp(key, "serverPort") == 0) {
    int res = parse_int(value, &v, ctx);
    if (res != 0) return res;
    gc->server_port = v;
    return 0;
  } else if (strcmp(key, "port") == 0) {
    int res = parse_int(value, &v, ctx);
    if (res != 0) return res;
    gc->port = v;
    Q_strcat(gc->command_line, sizeof(gc->command_line),
             va(" +set net_port6 %ld", v));
    return 0;
  } else if (strcmp(key, "vmMode") == 0) {
    if (strcmp(value, "interpreted") == 0) {
      gc->vm_mode = VMI_BYTECODE;
    } else if (strcmp(value, "compiled") == 0) {
      gc->vm_mode = VMI_COMPILED;
    } else if (strcmp(value, "native") == 0) {
      gc->vm_mode = VMI_NATIVE;
    } else {
      ctx->hooks.set_error_message(
          ctx->userdata,
          va("vmMode must be either: "
             "\"interpreted\",  \"compiled\", or \"native\"; actual: \"%s\"\n",
             value));
      return 1;
    }
  } else if (strcmp(key, "fps") == 0) {
    int res = parse_double(value, &v_double, ctx);
    if (res != 0) return res;
    if (v_double > 0) {
      gc->engine_frame_period_msec =
          (int)((kEngineTimePerExternalTime * 1000.0 / v_double) + 0.5);
    }
  } else if (strcmp(key, "logToStdErr") == 0) {
    int res = parse_bool(value, &v_bool, ctx);
    if (res != 0) return res;
    if (v_bool) {
      fputs("logToStdErr: \"true\"\n", stderr);
      Q_strcat(gc->command_line, sizeof(gc->command_line),
               " +set com_logToStdErr 1");
    } else {
      Q_strcat(gc->command_line, sizeof(gc->command_line),
               " +set com_logToStdErr 0");
    }
  } else if (strcmp(key, "minimalUI") == 0) {
    int res = parse_bool(value, &v_bool, ctx);
    if (res != 0) return res;
    if (v_bool) {
      Q_strcat(gc->command_line, sizeof(gc->command_line),
               " +set cg_draw2D 0 +set cg_drawCrosshairAlways 1");
    } else {
      Q_strcat(gc->command_line, sizeof(gc->command_line),
               " +set cg_draw2D 1 +set cg_drawCrosshairAlways 0");
    }
  } else if (strcmp(key, "reducedUI") == 0) {
    int res = parse_bool(value, &v_bool, ctx);
    if (res != 0) return res;
    if (v_bool) {
      Q_strcat(gc->command_line, sizeof(gc->command_line),
               " +set cg_drawReducedUI 1 +set cg_drawCrosshairAlways 1");
    } else {
      Q_strcat(gc->command_line, sizeof(gc->command_line),
               " +set cg_drawReducedUI 0 +set cg_drawCrosshairAlways 0");
    }
  } else if (strcmp(key, "nativeApp") == 0) {
    int res = parse_bool(value, &v_bool, ctx);
    if (res != 0) return res;
    ctx->hooks.set_native_app(ctx->userdata, v_bool);
  } else if (strcmp(key, "appendCommand") == 0) {
    Q_strcat(gc->command_line, sizeof(gc->command_line), " ");
    Q_strcat(gc->command_line, sizeof(gc->command_line), value);
  } else if (strcmp(key, "record") == 0) {
    if (!dmlab_set_recording_name(gc->recording_ctx, value)) {
      gc->dm_ctx->hooks.set_error_message(gc->dm_ctx->userdata,
                                          gc->recording_ctx->error_message);
      return 1;
    }
  } else if (strcmp(key, "demo") == 0) {
    if (!dmlab_set_demo_name(gc->recording_ctx, value)) {
      gc->dm_ctx->hooks.set_error_message(gc->dm_ctx->userdata,
                                          gc->recording_ctx->error_message);
      return 1;
    }
  } else if (strcmp(key, "video") == 0) {
    if (!dmlab_set_video_name(gc->recording_ctx, value)) {
      gc->dm_ctx->hooks.set_error_message(gc->dm_ctx->userdata,
                                          gc->recording_ctx->error_message);
      return 1;
    }
  } else if (strcmp(key, "demofiles") == 0) {
    dmlab_set_demofiles_path(gc->recording_ctx, value);
  } else if (strcmp(key, "use_pbos") == 0) {
    int res = parse_bool(value, &v_bool, ctx);
    if (res != 0) return res;
    gc->pbos.enabled = v_bool;
  } else if (strcmp(key, "gpuDeviceIndex") == 0) {
    int res = parse_int(value, &v, ctx);
    if (res != 0) return res;
    Q_strcat(gc->command_line, sizeof(gc->command_line),
             va(" +set r_gpuDeviceIndex %ld", v));
  } else if (strcmp(key, "playerName") == 0) {
    if (strlen(value) >= MAX_CVAR_VALUE_STRING) {
      ctx->hooks.set_error_message(ctx->userdata,
                                   va("Invalid playerName is must be shorter "
                                      "than, '%d' characters.",
                                      MAX_CVAR_VALUE_STRING));
      return 1;
    }
    Q_strcat(gc->command_line, sizeof(gc->command_line),
             va(" +set name \"%s\"", value));
  } else if (strcmp(key, "mixerSeed") == 0) {
    int res = parse_int(value, &v, ctx);
    if (res != 0) return res;
    if (v < 0 || v > UINT32_MAX) {
      ctx->hooks.set_error_message(ctx->userdata,
                                   va("Invalid mixerSeed value, must be a "
                                      "positive integer not greater than '%"
                                      PRIu32 "'.", UINT32_MAX));
      return 1;
    }
    ctx->hooks.set_mixer_seed(ctx->userdata, (uint32_t)v);
  } else {
    ctx->hooks.add_setting(ctx->userdata, key, value);
  }

  return 0;
}

static int dmlab_init(void* context) {
  GameContext* gc = context;
  DeepmindContext* ctx = gc->dm_ctx;
  if (ctx->hooks.get_native_app(ctx->userdata)) {
    SCR_SkipRendering(false);
    SCR_RenderOrigin(RO_BOTTOM_LEFT);
  } else {
    SCR_SkipRendering(true);
    SCR_RenderOrigin(RO_TOP_LEFT);
  }

  if (gc->vm_mode != VMI_NATIVE) {
    Q_strcat(gc->command_line, sizeof(gc->command_line),
             va(" +set vm_cgame \"%d\""
                " +set vm_game \"%d\""
                " +set vm_ui \"%d\"",
                gc->vm_mode, gc->vm_mode, gc->vm_mode));
  }
  if (gc->is_server) {
    Q_strcat(gc->command_line, sizeof(gc->command_line),
             " +set sv_hostname \"server\""
             " +set sv_fps 20"
             " +set dedicated 1"
             " +set sv_host server"
             " +set sv_allowDownload 1");
  }
  if (gc->init_called) {
    ctx->hooks.set_error_message(
        ctx->userdata, "'init' has already been called previously.\n");
    return 1;
  }
  gc->init_called = true;
  ctx->hooks.set_level_cache_settings(ctx->userdata, gc->use_local_level_cache,
                                      gc->use_global_level_cache,
                                      gc->level_cache_params);
  return ctx->hooks.init(ctx->userdata);
}

static void connect_client(GameContext* gc) {
  Cmd_ExecuteString(va("connect -6 [::1]:%d\n", gc->server_port));
  Cvar_Set("fixedtime", va("%d", gc->engine_frame_period_msec));
  Com_Frame();
  gc->is_connecting = true;
  gc->map_loaded = false;
}

static bool start_server(GameContext* gc) {
  dev_map(gc);
  if (gc->recording_ctx->is_recording) {
    dmlab_start_recording(gc->recording_ctx);
    if (gc->recording_ctx->error != DMLAB_RECORDING_ERROR_NONE) {
      gc->dm_ctx->hooks.set_error_message(gc->dm_ctx->userdata,
                                          gc->recording_ctx->error_message);
      return false;
    }
  }
  gc->is_connecting = true;
  gc->map_loaded = false;
  return true;
}

static int dmlab_start(void* context, int episode_id, int seed) {
  // Make seed a non-negative integer.
  seed = (seed < 0) ? seed + 1 + INT_MAX : seed;
  GameContext* gc = context;
  DeepmindContext* ctx = gc->dm_ctx;
  if (!gc->init_called) {
    ctx->hooks.set_error_message(ctx->userdata,
                                 "'init' must be called before 'start. See "
                                 "documentation in env_c_api.h'\n");

    return 1;
  }
  gc->current_screen_rendered = false;
  if (gc->is_connecting) {
    re.MakeCurrent();
    int err = connecting(gc);
    if (err == 0 && !gc->map_loaded) {
      err = ctx->hooks.map_loaded(ctx->userdata);
      gc->map_loaded = true;
    }
    return err;
  }
  ctx->hooks.events.clear(ctx->userdata);
  gc->step = 0;
  gc->total_engine_time_msec = 0.0;
  gc->score = 0.0;

  int err = ctx->hooks.start(ctx->userdata, episode_id, seed);
  if (err != 0) {
    return err;
  }

  if (!gc->first_start) {
    int err = first_start(gc);
    if (err != 0) {
      return err;
    }
    gc->first_start = true;
  }

  re.MakeCurrent();
  if (gc->is_client_only) {
    connect_client(gc);
  } else if (gc->is_server) {
    if (!start_server(gc)) {
      return 1;
    }
  } else {
    if (!load_map(gc)) {
      return 1;
    }
    if (ctx->hooks.map_loaded(ctx->userdata) != 0) {
      return 1;
    }
  }

  gc->is_map_loading = false;
  if (gc->is_client_only) {
    return clc.state < CA_ACTIVE ? EAGAIN : 0;
  }
  return 0;
}

static const char* dmlab_environment_name(void* context) {
  return "deepmind_lab";
}

static int dmlab_action_discrete_count(void* context) {
  GameContext* gc = context;
  DeepmindContext* ctx = gc->dm_ctx;
  return gc->is_server
             ? 0
             : ARRAY_LEN(kActionNames) +
                   ctx->hooks.custom_action_discrete_count(ctx->userdata);
}

static const char* dmlab_action_discrete_name(void* context, int discrete_idx) {
  if (discrete_idx < ARRAY_LEN(kActionNames)) {
    return kActionNames[discrete_idx];
  } else {
    GameContext* gc = context;
    DeepmindContext* ctx = gc->dm_ctx;
    return ctx->hooks.custom_action_discrete_name(
        ctx->userdata, discrete_idx - ARRAY_LEN(kActionNames));
  }
}

static void dmlab_action_discrete_bounds(
    void* context,
    int discrete_idx, int* min_value_out, int* max_value_out) {
  if (discrete_idx < kActions_StrafeLeftRight) {
    *min_value_out = -kMaxLookAngularVelocity;
    *max_value_out = kMaxLookAngularVelocity;
  } else if (discrete_idx < kActions_Fire) {
    *min_value_out = -1;
    *max_value_out = 1;
  } else if (discrete_idx < ARRAY_LEN(kActionNames)) {
    *min_value_out = 0;
    *max_value_out = 1;
  } else {
    GameContext* gc = context;
    DeepmindContext* ctx = gc->dm_ctx;
    ctx->hooks.custom_action_discrete_bounds(
        ctx->userdata, discrete_idx - ARRAY_LEN(kActionNames), min_value_out,
        max_value_out);
  }
}

static int dmlab_action_continuous_count(void* context) {
  return 0;
}

static const char* dmlab_action_continuous_name(
    void* context, int continuous_idx) {
  return NULL;
}

static int dmlab_action_text_count(void* context) {
  return 0;
}

static const char* dmlab_action_text_name(
    void* context, int continuous_idx) {
  return NULL;
}

static void dmlab_action_continuous_bounds(
    void* context,
    int continuous_idx, double* min_value_out, double* max_value_out) {}

static int dmlab_observation_count(void* context) {
  GameContext* gc = context;
  DeepmindContext* ctx = gc->dm_ctx;
  return ARRAY_LEN(kObservationNames) +
         ctx->hooks.custom_observation_count(ctx->userdata);
}

static const char* dmlab_observation_name(void* context, int observation_idx) {
  GameContext* gc = context;
  DeepmindContext* ctx = gc->dm_ctx;
  if (observation_idx < ARRAY_LEN(kObservationNames)) {
    return kObservationNames[observation_idx];
  } else {
    return ctx->hooks.custom_observation_name(
        ctx->userdata, observation_idx - ARRAY_LEN(kObservationNames));
  }
}

static void dmlab_observation_spec(
    void* context, int observation_idx, EnvCApi_ObservationSpec* spec) {
  GameContext* gc = context;

  if (observation_idx == kObservations_MapFrameNumber) {
    spec->type = EnvCApi_ObservationDoubles;
    spec->dims = 1;
    spec->shape = gc->map_frame_number_shape;
  } else if (observation_idx < ARRAY_LEN(kObservationNames)) {
    spec->type = EnvCApi_ObservationBytes;
    spec->dims = 3;
    spec->shape = gc->image_shape;

    switch (observation_idx) {
      case kObservations_RgbInterlaced:
        fprintf(stderr, "Using deprecated observation format: '%s'\n",
                kObservationNames[observation_idx]);
        // FALLTHROUGH_INTENDED
      case kObservations_RgbInterleaved:
      case kObservations_BgrInterleaved:
        gc->image_shape[0] = gc->height;
        gc->image_shape[1] = gc->width;
        gc->image_shape[2] = 3;
        break;
      case kObservations_RgbdInterlaced:
        fprintf(stderr, "Using deprecated observation format: '%s'\n",
                kObservationNames[observation_idx]);
        // FALLTHROUGH_INTENDED
      case kObservations_RgbdInterleaved:
      case kObservations_BgrdInterleaved:
        gc->image_shape[0] = gc->height;
        gc->image_shape[1] = gc->width;
        gc->image_shape[2] = 4;
        break;
      case kObservations_RgbPlanar:
        gc->image_shape[0] = 3;
        gc->image_shape[1] = gc->height;
        gc->image_shape[2] = gc->width;
        break;
      case kObservations_RgbdPlanar:
        gc->image_shape[0] = 4;
        gc->image_shape[1] = gc->height;
        gc->image_shape[2] = gc->width;
        break;
    }
  } else {
    DeepmindContext* ctx = gc->dm_ctx;
    ctx->hooks.custom_observation_spec(
        ctx->userdata, observation_idx - ARRAY_LEN(kObservationNames), spec);
  }
}

static int dmlab_fps(void* context) {
  GameContext* gc = context;
  if (gc->engine_frame_period_msec > 0) {
    return (1000.0 * kEngineTimePerExternalTime) / gc->engine_frame_period_msec;
  } else {
    return 0;
  }
}

static void dmlab_observation(
    void* context, int observation_idx, EnvCApi_Observation* obs) {
  GameContext* gc = context;
  if (observation_idx < ARRAY_LEN(kObservationNames)) {
    dmlab_observation_spec(context, observation_idx, &obs->spec);

    if (observation_idx == kObservations_MapFrameNumber) {
      gc->map_frame_number_observation = cls.framecount - gc->map_start_frame;
      obs->payload.doubles = &gc->map_frame_number_observation;
      return;
    }

    re.MakeCurrent();

    if (!gc->current_screen_rendered) {
      SCR_SkipRendering(false);
      SCR_RenderOrigin(RO_TOP_LEFT);
      SCR_UpdateScreen();
      gc->current_screen_rendered = true;
    }

    bool render_depth = observation_idx == kObservations_RgbdInterlaced ||
                        observation_idx == kObservations_RgbdInterleaved ||
                        observation_idx == kObservations_RgbdPlanar ||
                        observation_idx == kObservations_BgrdInterleaved;

    const int width = gc->width;
    const int height = gc->height;
    const int window_size = height * width;
    const enum PixelBufferTypeEnum_e pixelBufferType =
        (observation_idx == kObservations_BgrdInterleaved ||
         observation_idx == kObservations_BgrInterleaved) ?
            kPixelBufferTypeEnum_Bgr : kPixelBufferTypeEnum_Rgb;

    request_pixel_observations(gc, pixelBufferType);
    if (render_depth) {
      request_pixel_observations(gc, kPixelBufferTypeEnum_Depth);
    }

    byte* temp_buffer = bind_pixel_observation(gc, pixelBufferType);

    switch (observation_idx) {
      case kObservations_RgbInterlaced:
      case kObservations_RgbInterleaved:
      case kObservations_BgrInterleaved: {
        gc->image_buffer = realloc_or_die(gc->image_buffer, window_size * 3);
        memcpy(gc->image_buffer, temp_buffer, window_size * 3);
        break;
      }
      case kObservations_RgbdInterlaced:
      case kObservations_RgbdInterleaved:
      case kObservations_BgrdInterleaved: {
        gc->image_buffer = realloc_or_die(gc->image_buffer, window_size * 4);
        unsigned char* const image_buffer = gc->image_buffer;
        for (int i = 0; i < height; ++i) {
          for (int j = 0; j < width; ++j) {
            int loc = (i * width + j) * 3;
            int y = i * width + j;
            image_buffer[y * 4 + 0] = temp_buffer[loc + 0];
            image_buffer[y * 4 + 1] = temp_buffer[loc + 1];
            image_buffer[y * 4 + 2] = temp_buffer[loc + 2];
          }
        }
        break;
      }
      case kObservations_RgbPlanar: {
        gc->image_buffer = realloc_or_die(gc->image_buffer, window_size * 3);
        unsigned char* const image_buffer = gc->image_buffer;
        for (int i = 0; i < height; ++i) {
          for (int j = 0; j < width; ++j) {
            int loc = (i * width + j) * 3;
            int y = i * width + j;
            image_buffer[y + window_size * 0] = temp_buffer[loc + 0];
            image_buffer[y + window_size * 1] = temp_buffer[loc + 1];
            image_buffer[y + window_size * 2] = temp_buffer[loc + 2];
          }
        }
        break;
      }
      case kObservations_RgbdPlanar: {
        gc->image_buffer = realloc_or_die(gc->image_buffer, window_size * 4);
        unsigned char* const image_buffer = gc->image_buffer;
        for (int i = 0; i < height; ++i) {
          for (int j = 0; j < width; ++j) {
            int loc = (i * width + j) * 3;
            int y = i * width + j;
            image_buffer[y + window_size * 0] = temp_buffer[loc + 0];
            image_buffer[y + window_size * 1] = temp_buffer[loc + 1];
            image_buffer[y + window_size * 2] = temp_buffer[loc + 2];
          }
        }
        break;
      }
    }
    unbind_pixel_observation(gc);

    if (render_depth) {
      unsigned char* const image_buffer = gc->image_buffer;
      temp_buffer = bind_pixel_observation(gc, kPixelBufferTypeEnum_Depth);
      switch (observation_idx) {
        case kObservations_RgbdInterlaced:
        case kObservations_RgbdInterleaved:
        case kObservations_BgrdInterleaved:
          for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
              int loc = i * width + j;
              int y = i * width + j;
              image_buffer[y * 4 + 3] = temp_buffer[loc];
            }
          }
          break;
        default:
          memcpy(image_buffer + window_size * 3, temp_buffer, window_size);
          break;
      }
      unbind_pixel_observation(gc);
    }
    obs->payload.bytes = gc->image_buffer;
  } else {
    DeepmindContext* ctx = gc->dm_ctx;
    ctx->hooks.custom_observation(
        ctx->userdata, observation_idx - ARRAY_LEN(kObservationNames), obs);
  }
}

static int dmlab_event_type_count(void* context) {
  GameContext* gc = context;
  DeepmindContext* ctx = gc->dm_ctx;
  return ctx->hooks.events.type_count(ctx->userdata);
}

static const char* dmlab_event_type_name(void* context, int event_type) {
  GameContext* gc = context;
  DeepmindContext* ctx = gc->dm_ctx;
  return ctx->hooks.events.type_name(ctx->userdata, event_type);
}

static int dmlab_event_count(void* context) {
  GameContext* gc = context;
  DeepmindContext* ctx = gc->dm_ctx;
  return ctx->hooks.events.count(ctx->userdata);
}

static void dmlab_event(void* context, int event_idx, EnvCApi_Event* event) {
  GameContext* gc = context;
  DeepmindContext* ctx = gc->dm_ctx;
  return ctx->hooks.events.export_event(ctx->userdata, event_idx, event);
}

static void dmlab_act_discrete(void* context, const int act_d[]) {
  GameContext* gc = context;
  gc->is_connecting = false;
  if (gc->is_server) return;
  DeepmindContext* ctx = gc->dm_ctx;
  int rightmove = act_d[kActions_StrafeLeftRight] * 127;
  int forwardmove = act_d[kActions_MoveBackForward] * 127;
  float yaw =
      -act_d[kActions_LookLeftRight] * kPixelsPerFrameToDegreesPerMilliseconds;
  float pitch =
      act_d[kActions_LookDownUp] * kPixelsPerFrameToDegreesPerMilliseconds;
  int upmove = (act_d[kActions_Jump] - act_d[kActions_Crouch]) * 127;
  int buttons = act_d[kActions_Fire];

  ctx->hooks.set_actions(ctx->userdata, pitch, yaw, forwardmove, rightmove,
                         upmove, buttons);
  ctx->hooks.custom_action_discrete_apply(ctx->userdata,
                                          act_d + ARRAY_LEN(kActionNames));
}

static void dmlab_act_continuous(void* context, const double act_c[]) {}
static void dmlab_act_text(void* context, const EnvCApi_TextAction act_c[]) {}

static double get_engine_score(void) {
  return cl.snap.ps.persistant[PERS_SCORE];
}

static int player_score(void* context) {
  GameContext* gc = context;
  return gc->score;
}

static EnvCApi_EnvironmentStatus dmlab_advance(
    void* context, int num_steps, double* reward) {
  re.MakeCurrent();
  GameContext* gc = context;
  DeepmindContext* ctx = gc->dm_ctx;
  if (ctx->hooks.get_native_app(ctx->userdata)) {
    SCR_SkipRendering(false);
    SCR_RenderOrigin(RO_BOTTOM_LEFT);
  } else {
    SCR_SkipRendering(true);
    SCR_RenderOrigin(RO_TOP_LEFT);
  }
  gc->current_screen_rendered = false;
  ctx->hooks.events.clear(ctx->userdata);
  *reward = 0;
  bool episode_ended = false;
  for (int i = 0; i < num_steps && !episode_ended; ++i) {
    double reward_before = get_engine_score();
    if (ctx->hooks.map_finished(ctx->userdata)) {
      // Capture any rewards given during map_finished().
      double final_reward_score = get_engine_score();

      bool map_loaded = load_map(gc);
      if (gc->recording_ctx->error != DMLAB_RECORDING_ERROR_NONE) {
        return EnvCApi_EnvironmentStatus_Error;
      }
      if (!map_loaded) {
        return EnvCApi_EnvironmentStatus_Terminated;
      }
      if (ctx->hooks.map_loaded(ctx->userdata) != 0) {
        return EnvCApi_EnvironmentStatus_Error;
      }
      ctx->hooks.set_map_finished(ctx->userdata, false);
      // TODO: Update player score to keep from previous map.
      double start_reward = get_engine_score();
      // Avoid getting large negative score at the start of a new map.
      reward_before = start_reward - (final_reward_score - reward_before);
    }

    gc->step += 1;
    int time_before = cl.serverTime;
    IN_Frame();
    Com_Frame();
    int time_after = cl.serverTime;

    if (gc->engine_frame_period_msec == 0) {
      gc->total_engine_time_msec += time_after - time_before;
    } else {
      gc->total_engine_time_msec = gc->step * gc->engine_frame_period_msec;
    }

    episode_ended = ctx->hooks.has_episode_finished(
        ctx->userdata,
        gc->total_engine_time_msec / (kEngineTimePerExternalTime * 1000.0));
    // The last frame of demos wipe the game state, effectively erasing the
    // score. By checking the state for active we only accumulate the score if
    // it has not been wiped. This is a workaround for an issue where server
    // game script methods are not invoked during demos (i.e. set_map_finished
    // is not triggered.)

    // Tick for one extra frame to gather any outstanding rewards.
    if (episode_ended) {
      Com_Frame();
    }

    if (clc.state == CA_ACTIVE) {
      double reward_after = get_engine_score();
      double delta_score = reward_after - reward_before;
      gc->score += delta_score;
      *reward += delta_score;
    }
    gc->is_map_loading = false;
  }
  gc->current_screen_rendered = false;
  return episode_ended ? EnvCApi_EnvironmentStatus_Terminated
                       : EnvCApi_EnvironmentStatus_Running;
}

static void dmlab_destroy_context(void* context) {
  GameContext* gc = context;
  DeepmindContext* ctx = gc->dm_ctx;

  if (gc->recording_ctx->is_recording) {
    dmlab_stop_recording(gc->recording_ctx);
  }
  if (gc->recording_ctx->is_video) {
    dmlab_stop_video(gc->recording_ctx);
  }
  if (gc->recording_ctx->error != DMLAB_RECORDING_ERROR_NONE) {
    fprintf(stderr, "ERROR: %s", gc->recording_ctx->error_message);
  }

  if (gc->pbos.rgb.id || gc->pbos.depth.id || gc->pbos.custom_view.id) {
    re.MakeCurrent();
    if (gc->pbos.rgb.id) {
      qglDeleteBuffers(1, &gc->pbos.rgb.id);
    }

    if (gc->pbos.depth.id) {
      qglDeleteBuffers(1, &gc->pbos.depth.id);
    }

    if (gc->pbos.custom_view.id) {
      qglDeleteBuffers(1, &gc->pbos.custom_view.id);
    }
  }

  dmlab_release_context(ctx);
  free(gc->recording_ctx);
  free(gc->temp_buffer);
  free(gc->image_buffer);
  free(gc);
  GLimp_Shutdown();
  DMLabUnloadIOQ3Module();
}

EnvCApi_PropertyResult dmlab_write_property(void* context, const char* key,
                                            const char* value) {
  if (strncmp(key, kReservedEnginePropertyList,
              strlen(kReservedEnginePropertyList)) == 0) {
    const char* sub_key = key + strlen(kReservedEnginePropertyList);
    if (sub_key[0] == '.') {
      ++sub_key;
      if (strcmp(sub_key, "fps") == 0) {
        return EnvCApi_PropertyResult_PermissionDenied;
      } else {
        return EnvCApi_PropertyResult_NotFound;
      }
    } else if (sub_key[0] == '\0') {
      return EnvCApi_PropertyResult_PermissionDenied;
    }
  }
  DeepmindContext* ctx = dmlab_context();
  return ctx->hooks.properties.write(ctx->userdata, key, value);
}

EnvCApi_PropertyResult dmlab_read_property(void* context, const char* key,
                                           const char** value) {
  if (strncmp(key, kReservedEnginePropertyList,
              strlen(kReservedEnginePropertyList)) == 0) {
    const char* sub_key = key + strlen(kReservedEnginePropertyList);
    if (sub_key[0] == '.') {
      ++sub_key;
      if (strcmp(sub_key, "fps") == 0) {
        *value = va("%d", dmlab_fps(context));
        return EnvCApi_PropertyResult_Success;
      }
      return EnvCApi_PropertyResult_NotFound;
    } else if (sub_key[0] == '\0') {
      return EnvCApi_PropertyResult_PermissionDenied;
    }
  }
  DeepmindContext* ctx = dmlab_context();
  return ctx->hooks.properties.read(ctx->userdata, key, value);
}

EnvCApi_PropertyResult dmlab_list_property(
    void* context, void* userdata, const char* key,
    void (*prop_callback)(void* userdata, const char* key,
                          EnvCApi_PropertyAttributes flags)) {
  if (strncmp(key, kReservedEnginePropertyList,
              strlen(kReservedEnginePropertyList)) == 0) {
    const char* sub_key = key + strlen(kReservedEnginePropertyList);
    if (sub_key[0] == '\0') {
      prop_callback(userdata, va("%s.%s", kReservedEnginePropertyList, "fps"),
                    EnvCApi_PropertyAttributes_Readable);
      return EnvCApi_PropertyResult_Success;
    } else if (sub_key[0] == '.') {
      ++sub_key;
      if (strcmp(sub_key, "fps") == 0) {
        return EnvCApi_PropertyResult_PermissionDenied;
      } else {
        return EnvCApi_PropertyResult_NotFound;
      }
    }
  }

  DeepmindContext* ctx = dmlab_context();
  EnvCApi_PropertyResult result =
      ctx->hooks.properties.list(ctx->userdata, userdata, key, prop_callback);
  if (key[0] == '\0') {
    // Advertise engine property list if empty key.
    prop_callback(userdata, kReservedEnginePropertyList,
                  EnvCApi_PropertyAttributes_Listable);
    result = EnvCApi_PropertyResult_Success;
  }
  return result;
}

static void call_add_score(int player_id, double score) {
  DeepmindContext* ctx = dmlab_context();
  ctx->hooks.add_score(ctx->userdata, player_id, score);
}

static void screen_shape(int* width, int* height, int* buff_width,
                         int* buff_height) {
  DeepmindContext* ctx = dmlab_context();
  GameContext* gc = ctx->context;
  *buff_width =
      gc->width >= gc->alt_camera_width ? gc->width : gc->alt_camera_width;
  *buff_height =
      gc->height >= gc->alt_camera_height ? gc->height : gc->alt_camera_height;
  *width = gc->width;
  *height = gc->height;
}

static void execute_console_command(const char* cmd) {
  Cmd_ExecuteString(cmd);
}

static void add_bot(const char* name, double skill, const char* team) {
  Cbuf_AddText(va("addbot %s %f %s\n", name, skill, team));
}

static int engine_frame_period_msec() {
  DeepmindContext* ctx = dmlab_context();
  GameContext* gc = ctx->context;
  return gc->engine_frame_period_msec;
}

static int total_engine_time_msec() {
  DeepmindContext* ctx = dmlab_context();
  GameContext* gc = ctx->context;
  return gc->total_engine_time_msec;
}

static double total_time_seconds() {
  DeepmindContext* ctx = dmlab_context();
  GameContext* gc = ctx->context;
  return gc->total_engine_time_msec / (kEngineTimePerExternalTime * 1000.0);
}

static bool dmlab_is_map_loading(void* context) {
  DeepmindContext* ctx = dmlab_context();
  GameContext* gc = ctx->context;
  return gc->is_map_loading;
}

float dmlab_raycast(const float start[3], const float end[3]);

static bool dmlab_in_fov(const float start[3], const float end[3],
                         const float angles[3], float fov) {
  return InFov(start, end, angles, fov) == qtrue;
}

static void dmlab_render_custom_view(
    int width, int height, unsigned char* buffer) {
  re.MakeCurrent();
  DeepmindContext* ctx = dmlab_context();
  GameContext* gc = ctx->context;

  SCR_RenderCustomView();

  gc->current_screen_rendered = false;

  if (!gc->pbos.custom_view.id) {
    qglGenBuffers(1, &gc->pbos.custom_view.id);
  }
  qglBindBuffer(GL_PIXEL_PACK_BUFFER, gc->pbos.custom_view.id);
  int size = width * height * 3;
  if (gc->pbos.custom_view.size < size) {
    gc->pbos.custom_view.size = size;
    qglBufferData(GL_PIXEL_PACK_BUFFER, size, NULL, GL_STREAM_READ);
  }

  qglReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, 0);
  const void* pixel_buffer = qglMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
  memcpy(buffer, pixel_buffer, size);
  qglUnmapBuffer(GL_PIXEL_PACK_BUFFER);
  qglBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
}

bool dmlab_update_rgba_texture(const char* name, int width, int height,
                               const unsigned char* data);

int dmlab_connect(const DeepMindLabLaunchParams* params, EnvCApi* env_c_api,
                  void** context) {
  DeepmindContext* dm_ctx = get_context_once();
  if (dm_ctx == NULL) {
    return 1;
  }

  GameContext* gc = calloc(1, sizeof(GameContext));
  if (gc == NULL) {
    return 2;
  }

  if (params == NULL) {
    return 3;
  }

  if (params->runfiles_path == NULL || params->runfiles_path[0] == '\0') {
    return 4;
  }

  dmlabRecordingContext* rcxt = calloc(1, sizeof(dmlabRecordingContext));
  if (rcxt == NULL) {
    return 1;
  }

  *context = gc;

  Q_strncpyz(gc->runfiles_path, params->runfiles_path,
             sizeof(gc->runfiles_path));

  // Disable local level cache by default.
  gc->use_local_level_cache = false;
  gc->use_global_level_cache = true;
  gc->level_cache_params = params->level_cache_params;
  gc->width = 320;
  gc->height = 240;
  gc->dm_ctx = dm_ctx;
  gc->recording_ctx = rcxt;
  gc->map_frame_number_shape[0] = 1;
  gc->map_frame_number_observation = 0;
  gc->pbos.enabled = true;

  memset(env_c_api, 0, sizeof(EnvCApi));

  env_c_api->setting = dmlab_setting;
  env_c_api->init = dmlab_init;
  env_c_api->write_property = dmlab_write_property;
  env_c_api->read_property = dmlab_read_property;
  env_c_api->list_property = dmlab_list_property;
  env_c_api->start = dmlab_start;
  env_c_api->error_message = dmlab_error_message;
  env_c_api->environment_name = dmlab_environment_name;
  env_c_api->action_discrete_count = dmlab_action_discrete_count;
  env_c_api->action_discrete_name = dmlab_action_discrete_name;
  env_c_api->action_discrete_bounds = dmlab_action_discrete_bounds;
  env_c_api->action_continuous_count = dmlab_action_continuous_count;
  env_c_api->action_continuous_name = dmlab_action_continuous_name;
  env_c_api->action_continuous_bounds = dmlab_action_continuous_bounds;
  env_c_api->action_text_count = dmlab_action_text_count;
  env_c_api->action_text_name = dmlab_action_text_name;
  env_c_api->observation_count = dmlab_observation_count;
  env_c_api->observation_name = dmlab_observation_name;
  env_c_api->observation_spec = dmlab_observation_spec;
  env_c_api->event_type_count = dmlab_event_type_count;
  env_c_api->event_type_name = dmlab_event_type_name;
  env_c_api->observation = dmlab_observation;
  env_c_api->event_count = dmlab_event_count;
  env_c_api->event = dmlab_event;
  env_c_api->act_discrete = dmlab_act_discrete;
  env_c_api->act_continuous = dmlab_act_continuous;
  env_c_api->act_text = dmlab_act_text;
  env_c_api->advance = dmlab_advance;
  env_c_api->release_context = dmlab_destroy_context;
  gc->dm_ctx->calls.player_score = player_score;
  gc->dm_ctx->calls.add_score = call_add_score;
  gc->dm_ctx->calls.screen_shape = screen_shape;
  gc->dm_ctx->calls.add_bot = add_bot;
  gc->dm_ctx->calls.execute_console_command = execute_console_command;
  gc->dm_ctx->calls.engine_frame_period_msec = engine_frame_period_msec;
  gc->dm_ctx->calls.total_engine_time_msec = total_engine_time_msec;
  gc->dm_ctx->calls.total_time_seconds = total_time_seconds;
  gc->dm_ctx->calls.deserialise_model = dmlab_deserialise_model;
  gc->dm_ctx->calls.load_model = dmlab_load_model;
  gc->dm_ctx->calls.serialised_model_size = dmlab_serialised_model_size;
  gc->dm_ctx->calls.serialise_model = dmlab_serialise_model;
  gc->dm_ctx->calls.save_model = dmlab_save_model;
  gc->dm_ctx->calls.update_rgba_texture = dmlab_update_rgba_texture;
  gc->dm_ctx->calls.raycast = dmlab_raycast;
  gc->dm_ctx->calls.in_fov = dmlab_in_fov;
  gc->dm_ctx->calls.is_map_loading = dmlab_is_map_loading;
  gc->dm_ctx->calls.render_custom_view = dmlab_render_custom_view;
  gc->dm_ctx->context = gc;
  return dmlab_create_context(gc->runfiles_path, gc->dm_ctx,
                              params->file_reader_override,
                              params->read_only_file_system,
                              params->optional_temp_folder);
}
