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
#ifndef DML_ENGINE_CODE_DEEPMIND_DMLAB_RECORDING_H_
#define DML_ENGINE_CODE_DEEPMIND_DMLAB_RECORDING_H_

#include <stdbool.h>

#include "../qcommon/q_shared.h"

enum dmlabRecordingError {
  DMLAB_RECORDING_ERROR_NONE,
  DMLAB_RECORDING_ERROR_DEMOFILES_NOT_SPECIFIED,
  DMLAB_RECORDING_ERROR_DEMOFILES_NOT_FOUND,

  DMLAB_RECORDING_ERROR_CREATE_DEMO_PATH,
  DMLAB_RECORDING_ERROR_MOVE_DEMO_FILE,
  DMLAB_RECORDING_ERROR_OVERWRITE_DEMO_FILE,

  DMLAB_RECORDING_ERROR_CREATE_VIDEO_PATH,
  DMLAB_RECORDING_ERROR_MOVE_VIDEO_FILE,
  DMLAB_RECORDING_ERROR_OVERWRITE_VIDEO_FILE,

  DMLAB_RECORDING_ERROR_FLUSH_OUTPUT_STREAM,
  DMLAB_RECORDING_ERROR_INVALID_FLAGS,
};

typedef struct dmlabRecordingContext_s {
  // The name of the recording to be stored in the demos directory in runfiles.
  // [runfiles_dir]/demos/[recording_name]/[recording_number].dm_71
  char recording_name[MAX_STRING_CHARS];

  // The name of the demo to be played back, same as the structure of
  // recording_name.
  char demo_name[MAX_STRING_CHARS];

  // The name of the video directory to be created.
  // [runfiles_dir]/videos/[video_name]/[recording_number].avi
  char video_name[MAX_STRING_CHARS];

  // True when configured to create a recording, otherwise false.
  bool is_recording;

  // True when configured to play back a demo, otherwise false.
  bool is_demo;

  // True when configured to generate a video, otherwise false.
  bool is_video;

  // The numerical index of the recording, increments each time a new map is
  // loaded in the episode.
  int demo_number;

  // The path where the demo files are located when is_demo or the path to
  // where the files should be moved when is_recording.
  char demofiles_path[MAX_STRING_CHARS];

  // The error field is set by calls to recording functions using the
  // dmlabRecordingContext to indicate what went wrong.
  enum dmlabRecordingError error;

  // The error_message field is set by calls to recording functions using the
  // dmlabRecordingContext to provide a detailed description of what went wrong.
  // It is only valid when the error field is not DMLAB_RECORDING_ERROR_NONE.
  char error_message[MAX_STRING_CHARS];
} dmlabRecordingContext;

// Copies |name| into the context as the recording name.
// Returns whether it succeeded.
bool dmlab_set_recording_name(dmlabRecordingContext* context, const char* name);

// Copies |name| into the context as the video name.
// Returns whether it succeeded.
bool dmlab_set_video_name(dmlabRecordingContext* context, const char* name);

// Copies |name| into the context as the demo name.
// Returns whether it succeeded.
bool dmlab_set_demo_name(dmlabRecordingContext* context, const char* name);

// Copies |path| into the context as the demofiles_path.
void dmlab_set_demofiles_path(dmlabRecordingContext* context, const char* path);

// Starts recording to the demo directory. If the demo with the specified name
// already exists, recording does not start and false is returned. Otherwise,
// true is returned.
bool dmlab_start_recording(dmlabRecordingContext* context);

// Ends the recording and moves any demo files from the home directory to
// the demo path.
bool dmlab_stop_recording(dmlabRecordingContext* context);

// Starts recording to the video directory. If the video with the specified name
// already exists, recording does not start and false is returned. Otherwise,
// true is returned.
bool dmlab_start_video(dmlabRecordingContext* context);

// Ends the video recording and moves any video files from the home directory to
// the demo path.
bool dmlab_stop_video(dmlabRecordingContext* context);

// Starts playback of the recorded demo. Moves files from the demo path to the
// home path if any exist. If there are no more demos to be played, returns
// false. Otherwise returns true.
bool dmlab_start_demo(dmlabRecordingContext* context);

#endif  // DML_ENGINE_CODE_DEEPMIND_DMLAB_RECORDING_H_
