// Copyright (C) 2016-2017 Google Inc.
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
//
// Functions to call into the engine.
// All the calls should go though this C interface.

#ifndef DML_DEEPMIND_INCLUDE_DEEPMIND_CALLS_H_
#define DML_DEEPMIND_INCLUDE_DEEPMIND_CALLS_H_

#include <stdbool.h>

#include "deepmind/include/deepmind_model_getters.h"
#include "deepmind/include/deepmind_model_setters.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DeepmindCalls_s DeepmindCalls;

// Collection of function pointers supplied by the engine to allow scripts to
// modify gameplay.
struct DeepmindCalls_s {
  // Adds reward to player player_id via the server. The server and client run
  // at different frame rates, so the effect may not be immediately visible.
  void (*add_score)(int player_id, double score);

  // Get desired screen shape.
  void (*screen_shape)(int* width, int* height, int* buff_width,
                       int* buff_height);

  // Executes a Quake console command. Must not be called during a callback.
  void (*execute_console_command)(const char* cmd);

  // Engine time between the start of two consecutive frames, in milliseconds.
  int (*engine_frame_period_msec)(void);

  // Get time stamp for current episode.
  int (*total_engine_time_msec)(void);

  // Get time in seconds for current episode.
  double (*total_time_seconds)(void);

  // Adds a bot to game.
  // 'name' - Name of bot. Must be one named in bots.txt.
  // 'skill' - The skill of this bot. Must be in the range [1.0 5.0].
  // 'team' - The bots team. Must be one of ("red", "blue", "free").
  void (*add_bot)(const char* name, double skill, const char* team);

  // Get the player score.
  int (*player_score)(void* context);

  // Attempt to load a serialised model from the MD3 data in 'buffer', invoking
  // the relevant callbacks in 'model_mutators' as the data is traversed.
  // Returns whether valid model data was found in the buffer.
  bool (*deserialise_model)(                       //
      const void* buffer,                          //
      const DeepmindModelSetters* model_mutators,  //
      void* model_data);

  // Attempt to load a model from a MD3 file at the given path, invoking the
  // relevant callbacks in 'model_mutators' as the file is traversed.
  // Returns whether a valid model file was found at the given path.
  bool (*load_model)(                              //
      const char* model_path,                      //
      const DeepmindModelSetters* model_mutators,  //
      void* model_data);

  // Returns the buffer size required to serialise the model described by
  // 'model_getters' in MD3 format.
  size_t (*serialised_model_size)(                //
      const DeepmindModelGetters* model_getters,  //
      void* model_data);

  // Attempt to serialise the model described by 'model_getters' in MD3 format
  // onto 'buffer'.
  void (*serialise_model)(                        //
      const DeepmindModelGetters* model_getters,  //
      void* model_data,                           //
      void* buffer);

  // Attempt to save the model described by 'model_getters' as a MD3 file onto
  // the given path.
  // Returns whether the save operation succeeded.
  bool (*save_model)(                             //
      const DeepmindModelGetters* model_getters,  //
      void* model_data,                           //
      const char* model_path);

  // Update the texture 'name' with the raw RGBA stream in 'data', using the
  // specified 'width' and 'height'.
  bool (*update_rgba_texture)(  //
      const char* name,         //
      int width,                //
      int height,               //
      const unsigned char* data);

  float (*raycast)(const float start[3], const float end[3]);

  bool (*in_fov)(const float start[3],   //
                 const float end[3],     //
                 const float angles[3],  //
                 float fov);

  void (*render_custom_view)(int width, int height, unsigned char* buffer);

  bool (*is_map_loading)();
};

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // DML_DEEPMIND_INCLUDE_DEEPMIND_CALLS_H_
