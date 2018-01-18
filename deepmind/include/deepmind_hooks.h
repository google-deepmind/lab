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
// Functions to be used as callbacks within the engine.
// All the hooks should go though this C interface.

#ifndef DML_DEEPMIND_INCLUDE_DEEPMIND_HOOKS_H_
#define DML_DEEPMIND_INCLUDE_DEEPMIND_HOOKS_H_

#include <stdbool.h>

#include "public/level_cache_types.h"
#include "third_party/rl_api/env_c_api.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DeepmindHooks_s DeepmindHooks;

struct DeepmindHooks_s {
  // A script will be passed these settings via params when the script is ran.
  // Calls after first call of init will be ignored.
  void (*add_setting)(void* userdata, const char* key, const char* value);

  // Sets which level caches to use when building levels.
  void (*set_level_cache_settings)(
      void* userdata, bool local, bool global,
      DeepMindLabLevelCacheParams level_cache_params);

  // Sets the name of the script that is ran during the first init.
  // Must be called before the first init.
  // Returns zero if successful and non-zero on error.
  int (*set_script_name)(void* userdata, const char* script_name);

  // Sets the path to where DeepMind Lab assets are stored.
  void (*set_executable_runfiles)(void* userdata,
                                  const char* executable_runfiles);

  // Gets the path to the temporary folder where dynamic maps are generated.
  const char* (*get_temporary_folder)(void* userdata);

  // Called after all settings have been applied.
  // Returns zero if successful and non-zero on error.
  int (*init)(void* userdata);

  // Called at the start of each episode.
  // Returns zero if successful and non-zero on error.
  int (*start)(void* userdata, int episode, int random_seed);

  // Called when the map has finished loading.
  // Returns zero if successful and non-zero on error.
  int (*map_loaded)(void* userdata);

  // The Quake3 engine makes heavy use of the command line. This function allows
  // us to intercept and replace it.
  // Must be called after first init.
  const char* (*replace_command_line)(void* userdata,
                                      const char* old_commandline);

  // Called whenever a map needs to be loaded.
  // Must return name of map discoverable by the engine.
  const char* (*next_map)(void* userdata);

  // Called when game starts to establish game type. See gametype_t from
  // bg_public.h
  int (*game_type)(void* userdata);

  // Runs a Lua snippet, using the Lua VM of the userdata.
  // If the script returns an integer, then this function
  // will return that value; otherwise it returns 0.
  int (*run_lua_snippet)(void* userdata, const char* command);

  // This is set if we are running a native app and the internal controller is
  // allowed to set actions.
  void (*set_native_app)(void* userdata, bool v);

  // Returns whether we are running a native app and the internal controller is
  // allowed to set actions.
  bool (*get_native_app)(void* userdata);

  // Sets the actions of the player.
  void (*set_actions)(void* userdata,                 //
                      double look_down_up,            //
                      double look_left_right,         //
                      signed char move_back_forward,  //
                      signed char strafe_left_right,  //
                      signed char crouch_jump,        //
                      int buttons_down);

  // Retrieves the actions of the player.
  void (*get_actions)(void* userdata,                  //
                      double* look_down_up,            //
                      double* look_left_right,         //
                      signed char* move_back_forward,  //
                      signed char* strafe_left_right,  //
                      signed char* crouch_jump,        //
                      int* buttons_down);

  // This is a C-style dictionary of settings the memory is pointed to by
  // spawn_var_chars.
  int (*update_spawn_vars)(void* userdata,               //
                           char* spawn_var_chars,        //
                           int* num_spawn_var_chars,     //
                           int spawn_vars_offsets[][2],  //
                           int* num_spawn_vars);

  // Find item with the given class_name.
  // Returns whether the item was found, and if so, writes the index of the
  // found item to *index.
  bool (*find_item)(void* userdata, const char* class_name, int* index);

  // Gets the current number of registered items.
  int (*item_count)(void* userdata);

  // Gets an item at a particular index, and fills in the various buffers.
  // Returns whether the operation succeeded.
  bool (*item)(void* userdata, int index, char* item_name, int max_item_name,
               char* class_name, int max_class_name,
               char* model_name, int max_model_name,
               int* quantity, int* type, int* tag);

  // Clears the current list of custom items. This is called on each
  // initialization.
  void (*clear_items)(void* userdata);

  // Returns whether we should finish the current map.
  bool (*map_finished)(void* userdata);

  // Sets whether the current map should finish.
  void (*set_map_finished)(void* userdata, bool map_finished);

  // Returns if the specified entity id can be picked up.
  bool (*can_pickup)(void* userdata, int entity_id);

  // Hook to determine if we're overriding the entity's default pickup
  // behaviour. Optionally sets the default respawn time.
  bool (*override_pickup)(void* userdata, int entity_id, int* respawn);

  // Get reward override for player.
  int (*reward_override)(void* userdata, const char* reason_opt, int player_id,
                         int team, const int* other_player_id_opt,
                         const float* origin_opt, int score);

  // Add score to a player. (This is picked up by the server on the next
  // update.)
  void (*add_score)(void* userdata, int player_id, double reward);

  // Returns a new random seed on each call.
  int (*make_random_seed)(void* userdata);

  double (*has_episode_finished)(void* userdata, double episode_time_seconds);

  // Calls script when bots should be loaded.
  void (*add_bots)(void* userdata);

  // Enables game to modify a texture after load.
  void (*modify_rgba_texture)(void* userdata, const char* name,
                              unsigned char* data, int width, int height);

  // Level-specific observations
  ///////////////

  // The number of level-specific observations.
  int (*custom_observation_count)(void* userdata);

  // The name associated with a level-specific observation at the given index.
  // 'idx' shall be in range [0, custom_observation_count()).
  const char* (*custom_observation_name)(void* userdata, int idx);

  // The shape of level-specific observation at the given index.
  //
  // 'idx' shall be in range [0, custom_observation_count()).
  //
  // If dims is zero in the resulting spec, the shape of this observation has to
  // be taken from the spec field of each individual observation. Otherwise, the
  // shape of every individual observation shall be identical to the one
  // returned from this call.
  void (*custom_observation_spec)(void* userdata, int idx,
                                  EnvCApi_ObservationSpec* spec);

  // Writes the level-specific observation at the given index to '*obs'
  //
  // 'idx' shall be in range [0, custom_observation_count()).
  // 'obs' is invalidated by any other calls to this function.
  void (*custom_observation)(void* userdata, int idx,
                             EnvCApi_Observation* observation);

  // Called from client game with current players state.
  void (*player_state)(void* userdata, const float origin[3],
                       const float velocity[3], const float viewangles[3],
                       float height, int team_score, int other_team_score,
                       int player_id, int timestamp_msec);

  // See MakeScreenMessages in deepmind/engine/context.h.
  int (*make_screen_messages)(void* userdata, int width, int height,
                              int line_height, int string_buffer_size);

  // See GetScreenMessage in deepmind/engine/context.h.
  void (*get_screen_message)(void* userdata, int message_id, char* buffer,
                             int* x, int* y, int* shadow, int* align_l0_r1_c2,
                             float rgba[4]);

  // See MakePk3FromMap in deepmind/engine/context.h.
  void (*make_pk3_from_map)(void* userdata, const char* map_path,
                            const char* map_name, bool gen_aas);

  // See TeamSelect in deepmind/engine/context.h.
  char (*team_select)(void* userdata, int player_id, const char* player_name);

  // Set and retrieve error message. 'error_message' shall be a null terminated
  // string.
  void (*set_error_message)(void* userdata, const char* error_message);
  const char* (*error_message)(void* userdata);
};

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // DML_DEEPMIND_INCLUDE_DEEPMIND_HOOKS_H_
