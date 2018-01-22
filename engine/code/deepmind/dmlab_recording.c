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
#include <sys/stat.h>
#include <stdio.h>
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

  char* file = va("demos/%s/%05d.%s%s", demo_name, demo_number,
                  demoext, protocol);
  return homefiles_file(file);
}

static const char* homefiles_video_path(
    const char* video_name, int demo_number) {
  const char* videoext = "avi";

  char* file = va("videos/%s/%05d.%s", video_name, demo_number, videoext);
  return homefiles_file(file);
}

static const char* demofiles_demo_path(
    const char* demofiles_path, const char* demo_name, int demo_number) {
  static char path[MAX_STRING_CHARS];
  const char* demoext = "dm_";  // DEMOEXT, from qshared.h
  const char* protocol = Cvar_VariableString("com_protocol");

  char* file = va("demos/%s/%05d.%s%s",
                  demo_name, demo_number, demoext, protocol);
  Q_strncpyz(path, FS_BuildOSPath(demofiles_path, ".", file),
             sizeof(path));
  return path;
}

static const char* demofiles_video_path(
  const char* demofiles_path, const char* video_name, int demo_number) {
  static char path[MAX_STRING_CHARS];

  const char* videoext = "avi";
  char* file = va("videos/%s/%05d.%s", video_name, demo_number, videoext);

  Q_strncpyz(path, FS_BuildOSPath(demofiles_path, ".", file), sizeof(path));
  return path;
}

static bool file_exists(const char *path) {
  struct stat path_stat;
  return stat(path, &path_stat) == 0 && S_ISREG(path_stat.st_mode);
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
static int move_file(const char* src, const char* dest) {
  FILE* src_file;
  FILE* dest_file;

  int rename_code = rename(src, dest);
  if (rename_code == 0) {
    return 0;
  }

  // Continue only if it's an invalid cross-device link error.
  if (errno != EXDEV) {
    return rename_code;
  }

  // Rename failed, copy the file and delete the original.
  if (!(src_file = fopen(src, "r"))) {
    return 1;  // 'fopen' sets errno.
  }

  if (!(dest_file = fopen(dest, "w"))) {
    fclose(src_file);
    return 1;  // 'fopen' sets errno.
  }

  int copy_fp_code = copy_fp(src_file, dest_file);
  fclose(src_file);
  fclose(dest_file);

  if (copy_fp_code == 0) {
    return unlink(src);
  } else {
    unlink(dest);
    return copy_fp_code;
  }
}

void dmlab_set_recording_name(dmlabRecordingContext* ctx, const char* name) {
  if (name == NULL || name[0] == '\0') {
    ctx->is_recording = false;
  } else {
    Q_strncpyz(ctx->recording_name, name, sizeof(ctx->recording_name));
    ctx->is_recording = true;
    ctx->demo_number = 0;
  }
}

void dmlab_set_demo_name(dmlabRecordingContext* ctx, const char* name) {
  if (name == NULL || name[0] == '\0') {
    ctx->is_recording = false;
  } else {
    Q_strncpyz(ctx->demo_name, name, sizeof(ctx->demo_name));
    ctx->is_demo = true;
    ctx->demo_number = 0;
  }
}

void dmlab_set_video_name(dmlabRecordingContext *ctx, const char* name) {
  if (name == NULL || name[0] == '\0') {
    ctx->is_video = false;
  } else {
    Q_strncpyz(ctx->video_name, name, sizeof(ctx->video_name));
    ctx->is_video = true;
  }
}

void dmlab_set_demofiles_path(
    dmlabRecordingContext* context, const char* path) {
  Q_strncpyz(context->demofiles_path, path, sizeof(context->demofiles_path));
}

bool dmlab_start_recording(dmlabRecordingContext* ctx) {
  if (ctx->demofiles_path[0] == '\0') {
    fprintf(stderr, "Recording failed: demofiles_path not specified.\n");
    return false;
  }
  ctx->demo_number++;
  if (file_exists(demofiles_demo_path(ctx->demofiles_path, ctx->recording_name,
                                      ctx->demo_number)) ||
      file_exists(homefiles_demo_path(ctx->recording_name, ctx->demo_number))) {
    fprintf(stderr, "Recording failed: '%s' already exists.\n",
            ctx->recording_name);
    return false;
  } else {
    Cvar_Set("ui_recordSPDemo", "1");
    Cvar_Set("g_synchronousClients", "1");
    Cbuf_AddText(va("record \"%s/%05d\"\n", ctx->recording_name,
                    ctx->demo_number));
    return true;
  }
}

bool dmlab_stop_recording(dmlabRecordingContext* ctx) {
  // Make sure recordings are completely written out at shutdown.
  if (fflush(NULL) != 0) {
    fprintf(stderr, "Error flushing output streams: %s\n", strerror(errno));
  }

  FS_CreatePath(
      (char *)demofiles_demo_path(ctx->demofiles_path, ctx->recording_name, 0));

  for (int demo_number = 1; demo_number <= ctx->demo_number; demo_number++) {
    const char* demofiles_path = demofiles_demo_path(
        ctx->demofiles_path, ctx->recording_name, demo_number);
    const char* homefiles_path = homefiles_demo_path(
        ctx->recording_name, demo_number);
    if (move_file(homefiles_path, demofiles_path)) {
      fprintf(stderr, "Moving demo file failed: %s %s %s\n",
              strerror(errno), homefiles_path, demofiles_path);
      return false;
    }
  }
  return true;
}

bool dmlab_start_demo(dmlabRecordingContext* ctx) {
  if (ctx->demofiles_path[0] == '\0') {
    fprintf(stderr, "Demo playback failed: demofiles_path not specified.\n");
    return false;
  }
  if (ctx->demo_number == 0) {
    FS_CreatePath((char *)homefiles_demo_path(ctx->demo_name, 0));

    int demo_number = 1;
    while (file_exists(demofiles_demo_path(
        ctx->demofiles_path, ctx->demo_name, demo_number))) {
      const char* demofiles_path = demofiles_demo_path(
          ctx->demofiles_path, ctx->demo_name, demo_number);
      const char* homefiles_path = homefiles_demo_path(
          ctx->demo_name, demo_number);
      if (move_file(demofiles_path, homefiles_path)) {
        fprintf(stderr, "Moving demo file failed: %s %s %s\n",
                strerror(errno), homefiles_path, demofiles_path);
        return false;
      }
      demo_number++;
    }
  }

  ctx->demo_number++;
  const char* homefiles_path = homefiles_demo_path(
      ctx->demo_name, ctx->demo_number);
  if (file_exists(homefiles_path)) {
    Cbuf_AddText(va("demo \"%s/%05d\"\n", ctx->demo_name, ctx->demo_number));
    return true;
  } else {
    return false;
  }
}

bool dmlab_start_video(dmlabRecordingContext* ctx) {
  if (ctx->demofiles_path[0] == '\0') {
    fprintf(stderr, "Video recording failed: demofiles_path not specified.\n");
    return false;
  }
  if (file_exists(homefiles_video_path(ctx->video_name, ctx->demo_number))) {
    fprintf(stderr, "Video recording failed: '%s' already exists.\n",
            ctx->video_name);
    return false;
  } else {
    // Demo number is incremented by dmlab_start_demo (since video must be
    // invoked with demo).
    Cbuf_AddText(va("video \"%s/%05d\"\n", ctx->video_name, ctx->demo_number));
    return true;
  }
}

bool dmlab_stop_video(dmlabRecordingContext* ctx) {
  FS_CreatePath((char *)demofiles_video_path(
      ctx->demofiles_path, ctx->video_name, 0));
  for (int demo_number = 1; demo_number <= ctx->demo_number; demo_number++) {
    const char* demofiles_path = demofiles_video_path(
        ctx->demofiles_path, ctx->video_name, demo_number);
    const char* homefiles_path = homefiles_video_path(
        ctx->video_name, demo_number);
    if (move_file(homefiles_path, demofiles_path)) {
      fprintf(stderr, "Moving video file failed: %s %s %s\n",
              strerror(errno), homefiles_path, demofiles_path);
      return false;
    }
  }
  return true;
}
