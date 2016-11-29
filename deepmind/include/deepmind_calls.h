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
//
// Functions to call into the engine.
// All the calls should go though this C interface.

#ifndef DML_DEEPMIND_INCLUDE_DEEPMIND_CALLS_H_
#define DML_DEEPMIND_INCLUDE_DEEPMIND_CALLS_H_

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
  void (*screen_shape)(void* context, int* width, int* height);

  // Engine time between the start of two consecutive frames, in milliseconds.
  int (*engine_frame_period_msec)(void* context);

  // Get time stamp for current episode.
  int (*total_engine_time_msec)(void* context);

  // Adds a bot to game.
  // 'name' - Name of bot. Must be one named in bots.txt.
  // 'skill' - The skill of this bot. Must be in the range [1.0 5.0].
  // 'team' - The bots team. Must be one of ("red", "blue", "free").
  void (*add_bot)(const char* name, double skill, const char* team);

  // Get the player score.
  int (*player_score)(void* context);
};

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // DML_DEEPMIND_INCLUDE_DEEPMIND_CALLS_H_
