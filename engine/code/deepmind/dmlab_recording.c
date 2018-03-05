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

#include "dmlab_recording.h"

#include "../qcommon/qcommon.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

static const char* homefiles_file(const char* file_name) {
  static char path[MAX_STRING_CHARS];
  const char* homepath = Cvar_VariableString("fs_homepath");
  const char* gamedir = Cvar_VariableString("fs_game");

  Q_strncpyz(path, FS_BuildOSPath(homepath, gamedir, file_name), sizeof(path));
  return path;
}

static const char* homefiles_demo_path(const char* demo_name, int demo_number) {
  const char* demoext = "dm_";  // DEMOEXT, from qshared.h
  const char* protocol = Cvar_VariableString("com_protocol");

  char* file =
      va("demos/%s/%05d.%s%s", demo_name, demo_number, demoext, protocol);
  return homefiles_file(file);
}

static const char* homefiles_video_path(const char* video_name,
                                        int demo_number) {
  const char* videoext = "avi";

  char* file = va("videos/%s/%05d.%s", video_name, demo_number, videoext);
  return homefiles_file(file);
}

static const char* demofiles_demo_path(const char* demofiles_path,
                                       const char* demo_name, int demo_number) {
  static char path[MAX_STRING_CHARS];
  const char* demoext = "dm_";  // DEMOEXT, from qshared.h
  const char* protocol = Cvar_VariableString("com_protocol");

  char* file =
      va("demos/%s/%05d.%s%s", demo_name, demo_number, demoext, protocol);
  Q_strncpyz(path, FS_BuildOSPath(demofiles_path, ".", file), sizeof(path));
  return path;
}

static const char* demofiles_video_path(const char* demofiles_path,
                                        const char* video_name,
                                        int demo_number) {
  static char path[MAX_STRING_CHARS];

  const char* videoext = "avi";
  char* file = va("videos/%s/%05d.%s", video_name, demo_number, videoext);

  Q_strncpyz(path, FS_BuildOSPath(demofiles_path, ".", file), sizeof(path));
  return path;
}

static bool file_exists(const char* path) {
  struct stat path_stat;
  return stat(path, &path_stat) == 0 && S_ISREG(path_stat.st_mode);
}

static bool dir_exists(const char* path) {
  struct stat path_stat;
  return stat(path, &path_stat) == 0 && S_ISDIR(path_stat.st_mode);
}

// Copies the file contents from one file pointer to another.
// Returns 0 on success, 1 otherwise.
static int copy_fp(FILE* src_file, FILE* dest_file) {
  static char buf[MAX_STRING_CHARS];
  size_t len;

  while ((len = fread(buf, 1, MAX_STRING_CHARS, src_file))) {
    if (len != fwrite(buf, 1, len, dest_file)) {
      return 1;  // errno might be set.
    }
  }

  return ferror(src_file) != 0;
}

// Moves a file from the src location to the dest location potentially across
// storage devices.
// Returns whether the operation was a success.
static bool move_file(const char* src, const char* dest) {
  FILE* src_file;
  FILE* dest_file;

  int rename_code = rename(src, dest);
  if (rename_code == 0) {
    return true;
  }

  // Continue only if it's an invalid cross-device link error.
  if (errno != EXDEV) {
    return rename_code;
  }

  // Rename failed, copy the file and delete the original.
  if (!(src_file = fopen(src, "r"))) {
    return false;  // 'fopen' sets errno.
  }

  if (!(dest_file = fopen(dest, "w"))) {
    fclose(src_file);
    return false;  // 'fopen' sets errno.
  }

  int copy_fp_code = copy_fp(src_file, dest_file);
  fclose(src_file);
  fclose(dest_file);

  if (copy_fp_code == 0) {
    return unlink(src) == 0;
  } else {
    unlink(dest);
    return copy_fp_code == 0;
  }
}

bool dmlab_set_recording_name(dmlabRecordingContext* ctx, const char* name) {
  ctx->error = DMLAB_RECORDING_ERROR_NONE;

  if (name == NULL || name[0] == '\0') {
    ctx->is_recording = false;
  } else if (ctx->is_demo || ctx->is_video) {
    ctx->error = DMLAB_RECORDING_ERROR_INVALID_FLAGS;
    Q_strncpyz(
        ctx->error_message,
        "The flags 'recording' and 'demo' may not both be specified.\n",
        sizeof(ctx->error_message));
    return false;
  } else {
    Q_strncpyz(ctx->recording_name, name, sizeof(ctx->recording_name));
    ctx->is_recording = true;
    ctx->demo_number = 0;
  }
  return true;
}

bool dmlab_set_demo_name(dmlabRecordingContext* ctx, const char* name) {
  ctx->error = DMLAB_RECORDING_ERROR_NONE;

  if (name == NULL || name[0] == '\0') {
    ctx->is_recording = false;
  } else if (ctx->is_recording) {
    ctx->error = DMLAB_RECORDING_ERROR_INVALID_FLAGS;
    Q_strncpyz(
        ctx->error_message,
        "The flags 'recording' and 'demo' may not both be specified.\n",
        sizeof(ctx->error_message));
    return false;
  } else {
    Q_strncpyz(ctx->demo_name, name, sizeof(ctx->demo_name));
    ctx->is_demo = true;
    ctx->demo_number = 0;
  }
  return true;
}

bool dmlab_set_video_name(dmlabRecordingContext* ctx, const char* name) {
  ctx->error = DMLAB_RECORDING_ERROR_NONE;

  if (name == NULL || name[0] == '\0') {
    ctx->is_video = false;
  } else if (ctx->is_recording) {
    ctx->error = DMLAB_RECORDING_ERROR_INVALID_FLAGS;
    Q_strncpyz(
        ctx->error_message,
        "The flags 'recording' and 'video' may not both be specified.\n",
        sizeof(ctx->error_message));
    return false;
  } else {
    Q_strncpyz(ctx->video_name, name, sizeof(ctx->video_name));
    ctx->is_video = true;
  }
  return true;
}

void dmlab_set_demofiles_path(dmlabRecordingContext* context,
                              const char* path) {
  context->error = DMLAB_RECORDING_ERROR_NONE;

  Q_strncpyz(context->demofiles_path, path, sizeof(context->demofiles_path));
}

bool dmlab_start_recording(dmlabRecordingContext* ctx) {
  ctx->error = DMLAB_RECORDING_ERROR_NONE;

  // Demofiles path is needed to know where to store demos.
  if (ctx->demofiles_path[0] == '\0') {
    ctx->error = DMLAB_RECORDING_ERROR_DEMOFILES_NOT_SPECIFIED;
    Q_strncpyz(ctx->error_message,
               va("Recording failed: demofiles path not specified.\n"),
               sizeof(ctx->error_message));
    return false;
  }

  // Begin recording the next run of the episode.
  ctx->demo_number++;
  if (file_exists(demofiles_demo_path(ctx->demofiles_path, ctx->recording_name,
                                      ctx->demo_number)) ||
      file_exists(homefiles_demo_path(ctx->recording_name, ctx->demo_number))) {
    ctx->error = DMLAB_RECORDING_ERROR_OVERWRITE_DEMO_FILE;
    Q_strncpyz(
        ctx->error_message,
        va("Recording failed: '%s' already exists.\n", ctx->recording_name),
        sizeof(ctx->error_message));

    return false;
  } else {
    Cvar_Set("ui_recordSPDemo", "1");
    Cvar_Set("g_synchronousClients", "1");
    Cbuf_AddText(
        va("record \"%s/%05d\"\n", ctx->recording_name, ctx->demo_number));
    return true;
  }
}

bool dmlab_stop_recording(dmlabRecordingContext* ctx) {
  ctx->error = DMLAB_RECORDING_ERROR_NONE;

  // Make sure recordings are completely written out at shutdown.
  if (fflush(NULL) != 0) {
    ctx->error = DMLAB_RECORDING_ERROR_FLUSH_OUTPUT_STREAM;
    Q_strncpyz(ctx->error_message,
               va("Error flushing output streams: %s\n", strerror(errno)),
               sizeof(ctx->error_message));
    return false;
  }

  // Attempt to create the path to where the demos should be moved.
  const char* demofiles_dir =
      demofiles_demo_path(ctx->demofiles_path, ctx->recording_name, 0);
  if (!FS_CreatePath((char*)demofiles_dir) && errno != EEXIST) {
    ctx->error = DMLAB_RECORDING_ERROR_CREATE_DEMO_PATH;
    Q_strncpyz(ctx->error_message,
               va("Creating demo path failed: %s %s\n", strerror(errno),
                  demofiles_dir),
               sizeof(ctx->error_message));
    return false;
  }

  // Attempt to move demo files.
  for (int demo_number = 1; demo_number <= ctx->demo_number; demo_number++) {
    const char* demofiles_path = demofiles_demo_path(
        ctx->demofiles_path, ctx->recording_name, demo_number);
    const char* homefiles_path =
        homefiles_demo_path(ctx->recording_name, demo_number);
    if (!move_file(homefiles_path, demofiles_path)) {
      ctx->error = DMLAB_RECORDING_ERROR_MOVE_DEMO_FILE;
      Q_strncpyz(ctx->error_message,
                 va("Moving demo file failed: %s %s %s\n", strerror(errno),
                    homefiles_path, demofiles_path),
                 sizeof(ctx->error_message));
      return false;
    }
  }
  return true;
}

bool dmlab_start_demo(dmlabRecordingContext* ctx) {
  ctx->error = DMLAB_RECORDING_ERROR_NONE;

  // Demofiles path is needed to know where to load demos from.
  if (ctx->demofiles_path[0] == '\0') {
    ctx->error = DMLAB_RECORDING_ERROR_DEMOFILES_NOT_SPECIFIED;
    Q_strncpyz(ctx->error_message,
               va("Demo playback failed: demofiles path not specified.\n"),
               sizeof(ctx->error_message));
    return false;
  }
  if (!dir_exists(ctx->demofiles_path)) {
    ctx->error = DMLAB_RECORDING_ERROR_DEMOFILES_NOT_FOUND;
    Q_strncpyz(
        ctx->error_message,
        va("Demo playback failed: demofiles path '%s' could not be found.\n",
           ctx->demofiles_path),
        sizeof(ctx->error_message));
    return false;
  }

  // Move all demos from the specified demofiles dir to the homepath required
  // for the engine to read.
  if (ctx->demo_number == 0) {
    const char* first_demofile =
        demofiles_demo_path(ctx->demofiles_path, ctx->demo_name, 1);
    if (!file_exists(first_demofile)) {
      ctx->error = DMLAB_RECORDING_ERROR_DEMOFILES_NOT_FOUND;
      Q_strncpyz(ctx->error_message,
                 va("Demo playback failed: Demo '%s' could not be found.\n",
                    first_demofile),
                 sizeof(ctx->error_message));
      return false;
    }

    const char* demofiles_dir = homefiles_demo_path(ctx->demo_name, 0);
    if (!FS_CreatePath((char*)demofiles_dir) && errno != EEXIST) {
      ctx->error = DMLAB_RECORDING_ERROR_CREATE_DEMO_PATH;
      Q_strncpyz(ctx->error_message,
                 va("Creating demo path failed: %s %s\n", strerror(errno),
                    demofiles_dir),
                 sizeof(ctx->error_message));
      return false;
    }

    int demo_number = 1;
    while (file_exists(demofiles_demo_path(ctx->demofiles_path, ctx->demo_name,
                                           demo_number))) {
      const char* demofiles_path =
          demofiles_demo_path(ctx->demofiles_path, ctx->demo_name, demo_number);
      const char* homefiles_path =
          homefiles_demo_path(ctx->demo_name, demo_number);
      if (!move_file(demofiles_path, homefiles_path)) {
        ctx->error = DMLAB_RECORDING_ERROR_MOVE_DEMO_FILE;
        Q_strncpyz(ctx->error_message,
                   va("Moving demo file failed: %s %s %s\n", strerror(errno),
                      demofiles_path, homefiles_path),
                   sizeof(ctx->error_message));
        return false;
      }
      demo_number++;
    }
  }

  ctx->demo_number++;
  const char* homefiles_path =
      homefiles_demo_path(ctx->demo_name, ctx->demo_number);
  if (file_exists(homefiles_path)) {
    Cbuf_AddText(va("demo \"%s/%05d\"\n", ctx->demo_name, ctx->demo_number));
    return true;
  } else {
    return false;
  }
}

bool dmlab_start_video(dmlabRecordingContext* ctx) {
  ctx->error = DMLAB_RECORDING_ERROR_NONE;

  // Demofiles path is needed to know where to store videos.
  if (ctx->demofiles_path[0] == '\0') {
    ctx->error = DMLAB_RECORDING_ERROR_DEMOFILES_NOT_SPECIFIED;
    Q_strncpyz(ctx->error_message,
               va("Video recording failed: demofiles path not specified.\n"),
               sizeof(ctx->error_message));
    return false;
  }
  if (!dir_exists(ctx->demofiles_path)) {
    ctx->error = DMLAB_RECORDING_ERROR_DEMOFILES_NOT_FOUND;
    Q_strncpyz(
        ctx->error_message,
        va("Video recording failed: demofiles path '%s' could not be found.\n",
           ctx->demofiles_path),
        sizeof(ctx->error_message));
    return false;
  }

  // Attempt recording a video.
  if (file_exists(homefiles_video_path(ctx->video_name, ctx->demo_number))) {
    ctx->error = DMLAB_RECORDING_ERROR_OVERWRITE_VIDEO_FILE;
    Q_strncpyz(
        ctx->error_message,
        va("Video recording failed: '%s' already exists.\n", ctx->video_name),
        sizeof(ctx->error_message));
    return false;
  } else {
    // Demo number is incremented by dmlab_start_demo (since video must be
    // invoked with demo).
    Cbuf_AddText(va("video \"%s/%05d\"\n", ctx->video_name, ctx->demo_number));
    return true;
  }
}

bool dmlab_stop_video(dmlabRecordingContext* ctx) {
  ctx->error = DMLAB_RECORDING_ERROR_NONE;

  // Attempt to create the path to where the videos should be copied.
  const char* video_files_path =
      demofiles_video_path(ctx->demofiles_path, ctx->video_name, 0);
  if (!FS_CreatePath((char*)video_files_path) && errno != EEXIST) {
    ctx->error = DMLAB_RECORDING_ERROR_CREATE_VIDEO_PATH;
    Q_strncpyz(ctx->error_message,
               va("Creating video path failed: %s %s\n", strerror(errno),
                  video_files_path),
               sizeof(ctx->error_message));
    return false;
  }

  // Copy each of the demo files to the destination.
  for (int demo_number = 1; demo_number <= ctx->demo_number; demo_number++) {
    const char* demofiles_path =
        demofiles_video_path(ctx->demofiles_path, ctx->video_name, demo_number);
    const char* homefiles_path =
        homefiles_video_path(ctx->video_name, demo_number);
    if (!move_file(homefiles_path, demofiles_path)) {
      ctx->error = DMLAB_RECORDING_ERROR_MOVE_VIDEO_FILE;
      Q_strncpyz(ctx->error_message,
                 va("Moving video file failed: %s %s %s\n", strerror(errno),
                    homefiles_path, demofiles_path),
                 sizeof(ctx->error_message));
      return false;
    }
  }
  return true;
}
