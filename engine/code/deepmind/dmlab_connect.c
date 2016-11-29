// Copyright (C) 2016 Google Inc.
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
#include <math.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../../deepmind/include/deepmind_context.h"
#include "../../../public/dmlab.h"
#include "../client/client.h"
#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"
#include "../renderercommon/qgl.h"
#include "../sys/sys_local.h"

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
  kObservations_RgbInterlaced,
  kObservations_RgbdInterlaced,
  kObservations_RgbPlanar,
  kObservations_RgbdPlanar,
};

const char* const kObservationNames[] = {
    "RGB_INTERLACED",   //
    "RGBD_INTERLACED",  //
    "RGB",              //
    "RGBD",             //
};

typedef struct GameContext_s {
  DeepmindContext* dm_ctx;
  int width;
  int height;
  int image_shape[3];
  unsigned char* image_buffer;
  unsigned char* temp_buffer;  // Holds result from glReadPixels.
  char script_name[MAX_STRING_CHARS];
  char command_line[MAX_STRING_CHARS];
  char runfiles_path[MAX_STRING_CHARS];
  bool first_start;
  bool init_called;
  int engine_frame_period_msec;   // This is the number of milliseconds to
                                  // advance the engine each frame. If set to
                                  // zero the wall clock is used.
  int step;
  double total_engine_time_msec;  // This is step * engine_frame_period_msec.
  double score;
} GameContext;


// **** Local helper functions and data **** //

static int first_start(GameContext* gc) {
  DeepmindContext* ctx = gc->dm_ctx;

  Sys_SetDefaultInstallPath(gc->runfiles_path);

  Sys_PlatformInit();
  Sys_Milliseconds();

  const char* modifiedCommandLine =
      ctx->hooks.replace_command_line(ctx->userdata, gc->command_line);

  Com_Init((char*)modifiedCommandLine);

  NET_Init();
  CON_Init();

  return 0;
}

static void load_map(GameContext* gc) {
  DeepmindContext* ctx = gc->dm_ctx;
  const char* next_map = ctx->hooks.next_map(ctx->userdata);
  Cmd_ExecuteString(va("devmap %s", next_map));
  Cvar_Set("fixedtime", va("%d", gc->engine_frame_period_msec));
  Com_Frame();
  while (clc.state < CA_ACTIVE) {
    IN_Frame();
    Com_Frame();
  }
  ctx->hooks.add_bots(ctx->userdata);
  printf("Map loaded: '%s'\n", next_map);
  fflush(stdout);
}

static int engine_frame_period_msec(void* context) {
  GameContext* gc = context;
  return gc->engine_frame_period_msec;
}

static int total_engine_time_msec(void* context) {
  GameContext* gc = context;
  return gc->total_engine_time_msec;
}

// Return 0 iff successful.
static int parse_int(const char* s, long int* out) {
  errno = 0;
  char* e;
  long int val = strtol(s, &e, 0);
  if (e != s && *e == '\0' && errno == 0) {
    *out = val;
    return 0;
  } else {
    return -1;
  }
}

// Return 0 iff successful.
static int parse_double(const char* s, double* out) {
  errno = 0;
  char* e;
  long int val = strtod(s, &e);
  if (e != s && *e == '\0' && errno == 0) {
    *out = val;
    return 0;
  } else {
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

static void* realloc_or_die(void* ptr, size_t n) {
  void* result = realloc(ptr, n);
  if (n > 0 && result == NULL) {
    fputs("Reallocation failure, aborting.\n", stderr);
    abort();
  }
  return result;
}

// **** DeepmindContext **** //

DeepmindContext* dmlab_context(void) {
  return &dmlab_context_impl;
}


// **** RL Environment implementation **** //

static int dmlab_setting(void* context, const char* key, const char* value) {
  GameContext* gc = context;

  if (gc->init_called) {
    fputs(
        "'init' has already been called. No further settings can be applied.\n",
        stderr);
    return 1;
  }

  DeepmindContext* ctx = gc->dm_ctx;
  long int v;
  double v_double;

  if (strcmp(key, "levelName") == 0) {
    return ctx->hooks.set_script_name(ctx->userdata, value);
  } else if (strcmp(key, "width") == 0) {
    int res = parse_int(value, &v);
    if (res != 0) return res;
    gc->width = v;
  } else if (strcmp(key, "height") == 0) {
    int res = parse_int(value, &v);
    if (res != 0) return res;
    gc->height = v;
  } else if (strcmp(key, "fps") == 0) {
    int res = parse_double(value, &v_double);
    if (res != 0) return res;
    if (v_double > 0) {
      gc->engine_frame_period_msec =
          (int)((kEngineTimePerExternalTime * 1000.0 / v_double) + 0.5);
    }
  } else if(strcmp(key, "logToStdErr") == 0) {
    if (strcmp(value, "true") == 0) {
      fputs("logToStdErr: \"true\"\n", stderr);
      Q_strcat(gc->command_line, sizeof(gc->command_line),
               " +set com_logToStdErr 1");
    } else if (strcmp(value, "false") == 0) {
      Q_strcat(gc->command_line, sizeof(gc->command_line),
               " +set com_logToStdErr 0");
    } else {
      fputs("logToStdErr must be either \"true\" or \"false\"\n", stderr);
      return 1;
    }
  } else if (strcmp(key, "controls") == 0) {
    if (strcmp(value, "internal") == 0) {
      ctx->hooks.set_use_internal_controls(ctx->userdata, true);
    } else if (strcmp(value, "external") == 0) {
      ctx->hooks.set_use_internal_controls(ctx->userdata, false);
    }
  } else if (strcmp(key, "appendCommand") == 0) {
    bool quote = strchr(value, '0') != NULL;
    if (quote) {
      Q_strcat(gc->command_line, sizeof(gc->command_line), "\"");
    }
    Q_strcat(gc->command_line, sizeof(gc->command_line), value);
    if (quote) {
      Q_strcat(gc->command_line, sizeof(gc->command_line), "\"");
    }
  } else {
    ctx->hooks.add_setting(ctx->userdata, key, value);
  }

  return 0;
}

static int dmlab_init(void* context) {
  GameContext* gc = context;
  DeepmindContext* ctx = gc->dm_ctx;
  if (gc->init_called) {
    fputs("'init' has already been called previously.\n", stderr);
    return 1;
  }
  gc->init_called = true;
  return ctx->hooks.init(ctx->userdata);
}

static int dmlab_start(void* context, int episode_id, int seed) {
  // Make seed a non-negative integer.
  seed = (seed < 0) ? seed + 1 + INT_MAX : seed;
  GameContext* gc = context;
  DeepmindContext* ctx = gc->dm_ctx;
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

  load_map(gc);
  return 0;
}

static const char* dmlab_environment_name(void* context) {
  return "deepmind_lab";
}

static int dmlab_action_discrete_count(void* context) {
  return ARRAY_LEN(kActionNames);
}

static const char* dmlab_action_discrete_name(void* context, int discrete_idx) {
  return kActionNames[discrete_idx];
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
  } else {
    *min_value_out = 0;
    *max_value_out = 1;
  }
}

static int dmlab_action_continuous_count(void* context) {
  return 0;
}

static const char* dmlab_action_continuous_name(
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

  if (observation_idx < ARRAY_LEN(kObservationNames)) {
    spec->type = EnvCApi_ObservationBytes;
    spec->dims = 3;
    spec->shape = gc->image_shape;

    switch (observation_idx) {
      case kObservations_RgbInterlaced:
        gc->image_shape[0] = gc->height;
        gc->image_shape[1] = gc->width;
        gc->image_shape[2] = 3;
        break;
      case kObservations_RgbdInterlaced:
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

static int dmlab_event_type_count(void* context) {
  return 0;
}

const char* dmlab_event_type_name(void* context, int event_type) {
  return 0;
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
    int window_size = gc->height * gc->width;

    dmlab_observation_spec(context, observation_idx, &obs->spec);
    gc->temp_buffer = realloc_or_die(gc->temp_buffer, window_size * 3);
    re.MakeCurrent();
    qglReadPixels(0, 0, gc->width, gc->height, GL_RGB, GL_UNSIGNED_BYTE,
                 gc->temp_buffer);

    switch (observation_idx) {
      case kObservations_RgbInterlaced:
        gc->image_buffer = realloc_or_die(gc->image_buffer, window_size * 3);
        for (int i = 0; i < gc->height; ++i) {
          for (int j = 0; j < gc->width; ++j) {
            int loc = (i * gc->width + j) * 3;
            int invy = (gc->height - i - 1) * gc->width + j;
            gc->image_buffer[invy * 3 + 0] = gc->temp_buffer[loc + 0];
            gc->image_buffer[invy * 3 + 1] = gc->temp_buffer[loc + 1];
            gc->image_buffer[invy * 3 + 2] = gc->temp_buffer[loc + 2];
          }
        }
        break;
      case kObservations_RgbdInterlaced:
        gc->image_buffer = realloc_or_die(gc->image_buffer, window_size * 4);
        for (int i = 0; i < gc->height; ++i) {
          for (int j = 0; j < gc->width; ++j) {
            int loc = (i * gc->width + j) * 3;
            int invy = (gc->height - i - 1) * gc->width + j;
            gc->image_buffer[invy * 4 + 0] = gc->temp_buffer[loc + 0];
            gc->image_buffer[invy * 4 + 1] = gc->temp_buffer[loc + 1];
            gc->image_buffer[invy * 4 + 2] = gc->temp_buffer[loc + 2];
          }
        }
        break;
      case kObservations_RgbPlanar:
        gc->image_buffer = realloc_or_die(gc->image_buffer, window_size * 3);
        for (int i = 0; i < gc->height; ++i) {
          for (int j = 0; j < gc->width; ++j) {
            int loc = (i * gc->width + j) * 3;
            int invy = (gc->height - i - 1) * gc->width + j;
            gc->image_buffer[invy + window_size * 0] = gc->temp_buffer[loc + 0];
            gc->image_buffer[invy + window_size * 1] = gc->temp_buffer[loc + 1];
            gc->image_buffer[invy + window_size * 2] = gc->temp_buffer[loc + 2];
          }
        }
        break;
      case kObservations_RgbdPlanar:
        gc->image_buffer = realloc_or_die(gc->image_buffer, window_size * 4);
        for (int i = 0; i < gc->height; ++i) {
          for (int j = 0; j < gc->width; ++j) {
            int loc = (i * gc->width + j) * 3;
            int invy = (gc->height - i - 1) * gc->width + j;
            gc->image_buffer[invy + window_size * 0] = gc->temp_buffer[loc + 0];
            gc->image_buffer[invy + window_size * 1] = gc->temp_buffer[loc + 1];
            gc->image_buffer[invy + window_size * 2] = gc->temp_buffer[loc + 2];
          }
        }
        break;
    }
    if (observation_idx == kObservations_RgbdInterlaced ||
        observation_idx == kObservations_RgbdPlanar) {
      qglReadPixels(0, 0, gc->width, gc->height, GL_DEPTH_COMPONENT,
                   GL_UNSIGNED_BYTE, gc->temp_buffer);

      for (int i = 0; i < gc->height; ++i) {
        for (int j = 0; j < gc->width; ++j) {
          int loc = i * gc->width + j;
          int invy = (gc->height - i - 1) * gc->width + j;
          if (observation_idx == kObservations_RgbdInterlaced) {
            gc->image_buffer[invy * 4 + 3] = gc->temp_buffer[loc];
          } else {
            gc->image_buffer[invy + window_size * 3] = gc->temp_buffer[loc];
          }
        }
      }
    }
    obs->payload.bytes = gc->image_buffer;
  } else {
    DeepmindContext* ctx = gc->dm_ctx;
    ctx->hooks.custom_observation(
        ctx->userdata, observation_idx - ARRAY_LEN(kObservationNames), obs);
  }
}

static int dmlab_event_count(void* context) {
  return 0;
}

static void dmlab_event(void* context, int event_idx, EnvCApi_Event* event) {}

static void dmlab_act(void* context, const int act_d[], const double act_c[]) {
  GameContext* gc = context;
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
}

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
  *reward = 0;
  bool episode_ended = false;
  for (int i = 0; i < num_steps && !episode_ended; ++i) {
    double reward_before = get_engine_score();
    if (ctx->hooks.map_finished(ctx->userdata)) {
      // Capture any rewards given during map_finished().
      double final_reward_score = get_engine_score();
      load_map(gc);
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
    double reward_after = get_engine_score();
    double delta_score = reward_after - reward_before;
    gc->score += delta_score;
    *reward += delta_score;
  }

  return episode_ended ? EnvCApi_EnvironmentStatus_Terminated
                       : EnvCApi_EnvironmentStatus_Running;
}

static void dmlab_destroy_context(void* context) {
  GameContext* gc = context;
  DeepmindContext* ctx = gc->dm_ctx;

  dmlab_release_context(ctx);
  free(gc->temp_buffer);
  free(gc->image_buffer);
  free(gc);
}

static void call_add_score(int player_id, double score) {
  DeepmindContext* ctx = dmlab_context();
  ctx->hooks.add_score(ctx->userdata, player_id, score);
}

static void screen_shape(void* context, int* width, int* height) {
  GameContext* gc = context;
  *width = gc->width;
  *height = gc->height;
}

static void add_bot(const char* name, double skill,  const char* team) {
  Cbuf_AddText(va("addbot %s %f %s\n", name, skill, team));
}

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

  *context = gc;

  Q_strncpyz(gc->runfiles_path, params->runfiles_path,
             sizeof(gc->runfiles_path));
  gc->width = 320;
  gc->height = 240;
  gc->dm_ctx = dm_ctx;

  memset(env_c_api, 0, sizeof(EnvCApi));

  env_c_api->setting = dmlab_setting;
  env_c_api->init = dmlab_init;
  env_c_api->start = dmlab_start;
  env_c_api->environment_name = dmlab_environment_name;
  env_c_api->action_discrete_count = dmlab_action_discrete_count;
  env_c_api->action_discrete_name = dmlab_action_discrete_name;
  env_c_api->action_discrete_bounds = dmlab_action_discrete_bounds;
  env_c_api->action_continuous_count = dmlab_action_continuous_count;
  env_c_api->action_continuous_name = dmlab_action_continuous_name;
  env_c_api->action_continuous_bounds = dmlab_action_continuous_bounds;
  env_c_api->observation_count = dmlab_observation_count;
  env_c_api->observation_name = dmlab_observation_name;
  env_c_api->observation_spec = dmlab_observation_spec;
  env_c_api->event_type_count = dmlab_event_type_count;
  env_c_api->event_type_name = dmlab_event_type_name;
  env_c_api->fps = dmlab_fps;
  env_c_api->observation = dmlab_observation;
  env_c_api->event_count = dmlab_event_count;
  env_c_api->event = dmlab_event;
  env_c_api->act = dmlab_act;
  env_c_api->advance = dmlab_advance;
  env_c_api->release_context = dmlab_destroy_context;

  gc->dm_ctx->calls.player_score = player_score;
  gc->dm_ctx->calls.add_score = call_add_score;
  gc->dm_ctx->calls.screen_shape = screen_shape;
  gc->dm_ctx->calls.add_bot = add_bot;
  gc->dm_ctx->calls.engine_frame_period_msec = engine_frame_period_msec;
  gc->dm_ctx->calls.total_engine_time_msec = total_engine_time_msec;
  gc->dm_ctx->context = gc;

  return dmlab_create_context(gc->runfiles_path, gc->dm_ctx);
}
