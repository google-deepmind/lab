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
//
// Functions to be used as callbacks within the engine.
// All the hooks should go though this C interface.

#ifndef DML_DEEPMIND_INCLUDE_DEEPMIND_HOOKS_H_
#define DML_DEEPMIND_INCLUDE_DEEPMIND_HOOKS_H_

#include <stdbool.h>

#include "deepmind/include/deepmind_model_getters.h"
#include "public/level_cache_types.h"
#include "third_party/rl_api/env_c_api.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DeepmindHooks_s DeepmindHooks;

typedef struct DeepmindEventsHooks_s DeepmindEventsHooks;

typedef struct DeepmindEntitiesHooks_s DeepmindEntitiesHooks;

typedef struct DeepmindPickupHooks_s DeepmindPickupHooks;

typedef struct DeepmindPropertyHooks_s DeepmindPropertyHooks;

struct DeepmindEventsHooks_s {
  // Returns number of event types.
  int (*type_count)(void* userdata);

  // Returns name of an event. 'type_idx' shall be on range [0, 'type_count').
  const char* (*type_name)(void* userdata, int type_idx);

  // Clears all events.
  void (*clear)(void* userdata);

  // Returns the number of events generated since last call to 'clear'.
  int (*count)(void* userdata);

  // Exports an event at 'idx', which must be in range [0, count()), to an
  // EnvCApi_Event structure.
  void (*export_event)(void* userdata, int idx, EnvCApi_Event* event);
};

struct DeepmindEntitiesHooks_s {
  // Clears all entities at start of entity update.
  void (*clear)(void* userdata);

  // Called with each active entity during entity update.
  void (*add)(void* userdata, int entity_id, int user_id, int type, int flags,
              float position[3], const char* classname);
};

struct DeepmindPickupHooks_s {
  // This is a C-style dictionary of settings; the memory is pointed to by
  // spawn_var_chars.
  int (*update_spawn_vars)(void* userdata,               //
                           char* spawn_var_chars,        //
                           int* num_spawn_var_chars,     //
                           int spawn_vars_offsets[][2],  //
                           int* num_spawn_vars);

  // Hook to create a number of entities externally. Returns the number of
  // entities created.
  int (*make_extra_entities)(void* userdata);

  // Hook to read an entity at a particular 'entity_id'. 'entity_id' shall be in
  // the range [0, dmlab_make_extra_entities()) and the remaining arguments
  // match 'dmlab_update_spawn_vars'.
  void (*read_extra_entity)(void* userdata,              //
                            int entity_id,               //
                            char* spawn_var_chars,       //
                            int* num_spawn_var_chars,    //
                            int spawn_var_offsets[][2],  //
                            int* num_spawn_vars);        //

  // Finds item with the given class_name.
  // Returns whether the item was found, and if so, writes the index of the
  // found item to *index.
  bool (*find_item)(void* userdata, const char* class_name, int* index);

  // Gets the current number of registered items.
  int (*item_count)(void* userdata);

  // Gets an item at a particular index, and fills in the various buffers.
  // Returns whether the operation succeeded.
  bool (*item)(void* userdata, int index, char* item_name, int max_item_name,
               char* class_name, int max_class_name, char* model_name,
               int max_model_name, int* quantity, int* type, int* tag,
               int* move_type);

  // Clears the current list of custom items. This is called on each
  // initialization.
  void (*clear_items)(void* userdata);

  // Return how many items to spawn this frame.
  int (*dynamic_spawn_entity_count)(void* userdata);

  // Read specific spawn var from extra_spawn_vars_. Shall be called after
  // with entity_index in range [0, dynamic_spawn_entity_count()).
  void (*read_dynamic_spawn_entity)(void* userdata, int entity_index,
                                    char* spawn_var_chars,
                                    int* num_spawn_var_chars,
                                    int spawn_var_offsets[][2],
                                    int* num_spawn_vars);

  // Clear spawn entities to use next frame.
  void (*clear_dynamic_spawn_entities)(void* userdata);

  int (*register_dynamic_items)(void* userdata);
  void (*read_dynamic_item_name)(void* userdata, int item_index,
                                 char* item_name);
};

struct DeepmindPropertyHooks_s {
  EnvCApi_PropertyResult (*write)(void* userdata, const char* key,
                                  const char* value);

  EnvCApi_PropertyResult (*read)(void* userdata, const char* key,
                                 const char** value);

  EnvCApi_PropertyResult (*list)(
      void* ctx_userdata, void* userdata, const char* group,
      void (*prop_callback)(void* userdata, const char* key,
                            EnvCApi_PropertyAttributes flags));
};

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
  void (*set_level_name)(void* userdata, const char* level_name);

  // Sets the name of the script that is ran during the first init.
  // Must be called before the first init.
  void (*set_level_directory)(void* userdata, const char* level_directory);

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

  // This is a bit toggle sequence applied to the most significant bits of the
  // seed.
  void (*set_mixer_seed)(void* userdata, int v);

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

  // Issue console commands generated by the script.
  void (*issue_console_commands)(void* userdata);

  // Retrieves model with the given model_name.
  // Returns whether the model was found and loads it within the context.
  bool (*find_model)(void* userdata, const char* model_name);

  // Return the accessor API for currently selected model.
  void (*model_getters)(void* userdata, DeepmindModelGetters* model,
                        void** model_data);

  // Clears the currently selected model. This is called on each initialization.
  void (*clear_model)(void* userdata);

  // Returns whether we should finish the current map.
  bool (*map_finished)(void* userdata);

  // Sets whether the current map should finish.
  void (*set_map_finished)(void* userdata, bool map_finished);

  // Returns if the specified entity id can be picked up.
  bool (*can_pickup)(void* userdata, int entity_id, int player_id);

  // Hook to determine if we're overriding the entity's default pickup
  // behaviour. Optionally sets the default respawn time.
  bool (*override_pickup)(void* userdata, int entity_id, int* respawn,
                          int player_id);

  // Returns if the specified entity has a trigger.
  bool (*can_trigger)(void* userdata, int entity_id, const char* target_name,
                      int player_id);

  // Hook to determine if we're overriding the entity's default trigger
  // behaviour.
  bool (*override_trigger)(void* userdata, int entity_id,
                           const char* target_name, int player_id);

  // Hook which is invoked on reply to a trigger lookat.
  void (*trigger_lookat)(void* userdata, int entity_id, bool looked_at,
                         const float position[3], int player_id);

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

  // Returns whether to replace texture named 'name' with new_name.
  // 'new_name' shall be a null terminated with length in [0, max_size).
  bool (*replace_texture_name)(void* userdata, const char* name, char* new_name,
                               int max_size);

  // Returns whether the texture named 'name' has been loaded externally.
  // If texture is loaded externally: 'pixels' will be a buffer sized *width *
  // *height * 4 and allocated using 'allocator'.
  // Otherwise 'pixels' 'width' and 'height' are left unchanged.
  bool (*load_texture)(void* userdata, const char* name, unsigned char** pixels,
                       int* width, int* height, void* (*allocator)(int size));

  // Enables game to modify a texture on load.
  bool (*modify_rgba_texture)(void* userdata, const char* name,
                              unsigned char* data, int width, int height);

  // Returns whether to replace model named 'name' with 'new_name' and prefix
  // the textures used with the model with 'texture_prefix'. The buffers pointed
  // to by new_name and texture_prefix shall have space for at least
  // new_name_size and texture_prefix_size, respectively, and on success they
  // are filled with null-terminated strings.
  bool (*replace_model_name)(void* userdata, const char* name, char* new_name,
                             int new_name_size, char* texture_prefix,
                             int texture_prefix_size);

  // Level-specific observations
  ///////////////

  // The number of level-specific actions.
  int (*custom_action_discrete_count)(void* userdata);

  // The name associated with a level-specific action at the given index.
  // 'idx' shall be in range [0, custom_action_discrete_count()).
  const char* (*custom_action_discrete_name)(void* userdata, int idx);

  // Bounds of level-specific action at the given index.
  // 'idx' shall be in range [0, custom_action_discrete_count()).
  void (*custom_action_discrete_bounds)(void* userdata, int idx,
                                        int* min_value_out, int* max_value_out);

  // Send custom actions to Lua script api. 'actions' shall be an array of
  // custom_action_discrete_count() integers each within the bounds described by
  // custom_action_discrete_bounds.
  void (*custom_action_discrete_apply)(void* userdata, const int* actions);


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
                       float height,  const float eyePos[3],
                       int team_score, int other_team_score,
                       int player_id, bool teleporter_flip, int timestamp_msec);

  // See MakeScreenMessages in deepmind/engine/context.h.
  int (*make_screen_messages)(void* userdata, int width, int height,
                              int line_height, int string_buffer_size);

  // See GetScreenMessage in deepmind/engine/context.h.
  void (*get_screen_message)(void* userdata, int message_id, char* buffer,
                             int* x, int* y, int* shadow, int* align_l0_r1_c2,
                             float rgba[4]);

  // See MakeFilledRectangles in deepmind/engine/context.h.
  int (*make_filled_rectangles)(void* userdata, int screen_width,
                                int screen_height);

  // See GetFilledRectangle in deepmind/engine/context.h.
  void (*get_filled_rectangle)(void* userdata, int rectangle_id, int* x, int* y,
                               int* width, int* height, float rgba[4]);

  // See MakePk3FromMap in deepmind/engine/context.h.
  void (*make_pk3_from_map)(void* userdata, const char* map_path,
                            const char* map_name, bool gen_aas);

  // See CustomPlayerMovement in deepmind/engine/context.h.
  void (*lua_mover)(void* userdata, int entity_id, const float entity_pos[3],
                    const float player_pos[3], const float player_vel[3],
                    float player_pos_delta[3], float player_vel_delta[3]);

  // See GameEvent in deepmind/engine/context.h.
  void (*game_event)(void* userdata, const char* event_name, int count,
                const float* data);

  // See UpdateInventory in deepmind/engine/context.h.
  void (*update_inventory)(void* userdata, bool is_spawning, bool is_bot,
                           int player_id, int gadget_count,
                           int gadget_inventory[], int persistent_count,
                           int persistents[], int stat_count,
                           int stats_inventory[], int powerups,
                           int powerups_time[], int gadget_held, float height,
                           float position[3], float velocity[3],
                           float view_angles[3]);

  // See TeamSelect in deepmind/engine/context.h.
  char (*team_select)(void* userdata, int player_id, const char* player_name);

  // See UpdatePlayerInfo in deepmind/engine/context.h.
  bool (*update_player_info)(void* userdata, int player_id, char* info,
                             int info_size);

  // See SetHasAltCameras in deepmind/engine/context.h.
  void (*set_has_alt_cameras)(void* userdata, bool has_alt_cameras);

  // See HasAltCameras in deepmind/engine/context.h.
  bool (*has_alt_cameras)(void* userdata);

  // See GetCustomView in deepmind/engine/context_game.h.
  void (*custom_view)(void* userdata, int* width, int* height,
                      float position[3], float view_angles[3],
                      bool* render_player);

  // See NewClientInfo in in deepmind/engine/context.h
  void (*new_client_info)(void* userdata, int player_id,
                          const char* player_name, const char* player_model);

  // Set and retrieve error message. 'error_message' shall be a null terminated
  // string.
  void (*set_error_message)(void* userdata, const char* error_message);
  const char* (*error_message)(void* userdata);

  DeepmindEventsHooks events;
  DeepmindEntitiesHooks entities;
  DeepmindPickupHooks pickups;
  DeepmindPropertyHooks properties;
};

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // DML_DEEPMIND_INCLUDE_DEEPMIND_HOOKS_H_
