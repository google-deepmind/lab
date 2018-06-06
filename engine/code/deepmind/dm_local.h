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

#ifndef DML_ENGINE_CODE_DEEPMIND_DM_LOCAL_H_
#define DML_ENGINE_CODE_DEEPMIND_DM_LOCAL_H_

#include "../qcommon/q_shared.h"
#include "../renderercommon/tr_types.h"

int trap_DeepmindCallback(int dm_callnum, intptr_t a1, intptr_t a2, intptr_t a3,
                          intptr_t a4, intptr_t a5, intptr_t a6, intptr_t a7,
                          intptr_t a8, intptr_t a9, intptr_t a10, intptr_t a11,
                          intptr_t a12);

// Client library goes here (implementation in dm_local.c).

// Hook to update an entity's spawn vars on the fly.
// 'spawn_var_chars' is pointing at the memory holding the strings.
// '*num_spawn_var_chars' is the total length of all strings and nulls.
// 'spawn_var_offsets' is the  key, value offsets in 'spawn_var_chars'.
// '*num_spawn_vars' is the number of valid spawn_vars_offsets.
// Returns whether the spawn actually needs to take place.
int dmlab_update_spawn_vars(char* spawn_var_chars,       //
                            int* num_spawn_var_chars,    //
                            int spawn_var_offsets[][2],  //
                            int* num_spawn_vars);

// Hook to create a number of entities externally. Returns the number of
// entities created.
int dmlab_make_extra_entities(void);

// Hook to read an entity at a particular 'entity_id'. 'entity_id' shall be in
// the range [0, dmlab_make_extra_entities()) and the remaining arguments match
// 'dmlab_update_spawn_vars'.
int dmlab_read_extra_entity(int entity_id,               //
                            char* spawn_var_chars,       //
                            int* num_spawn_var_chars,    //
                            int spawn_var_offsets[][2],  //
                            int* num_spawn_vars);

// Find item with particular class_name.
// Returns true if we found the item, and the deepmind_item index.
int dmlab_finditem(const char* classname, int* index);

// Get the current number of registered items.
int dmlab_itemcount(void);

// Get an item at a partiucular index and fill in the various buffers provided.
// Returns true if we successfully filled in the various arrays.
int dmlab_item(int index, char* item_name, int max_item_name,  //
               char* classname, int max_classname,             //
               char* model_name, int max_model_name,           //
               int* quantity, int* type, int* tag, int* move_type);

// Clear the current list of registered items. Called just before loading a new
// map.
void dmlab_clearitems(void);

// Sets whether the current map should finish.
void dmlab_set_map_finished(int map_finished);

// Returns if we can pickup the specified entity.
qboolean dmlab_can_pickup(int entity_id, const playerState_t* ps);

// Hook to allow customization of the entity's pickup behaviour. Users are
// expected to implement their own game logic for picking up the specified item.
// Can also return the default respawn time for the entity.
// Returns true if the pickup behaviour has been overridden.
qboolean dmlab_override_pickup(int entity_id, int* respawn,
                               const playerState_t* ps);

// Returns if the specified entity can trigger.
qboolean dmlab_can_trigger(int entity_id, const char* target_name,
                           const playerState_t* ps);

// Hook to allow customization of the entity's trigger behaviour. Users are
// expected to implement their own game logic for triggering the specified item.
// Returns whether the trigger behaviour has been overridden.
// If the default trigger behaviour has been overridden then the custom trigger
// logic will execute instead.
qboolean dmlab_override_trigger(int entity_id, const char* target_name,
                                const playerState_t* ps);

// Hook to allow customization of the entity's lookat behaviour.
void dmlab_trigger_lookat(int entity_id, qboolean looked_at, vec3_t position,
                          const playerState_t* ps);

// Customization point for overriding the value of a reward and consuming reward
// stored in external context for player 'player_id'.
int dmlab_reward_override(const char* reason_opt, int player_id, int team,
                          const int* other_player_id_opt,
                          const vec3_t origin_opt, int score);

// Get the current player score.
int dmlab_player_score(void);

// Sets the player info.
void dmlab_player_state(const playerState_t* ps, const float eyePos[3],
                        int team_score, int other_team_score);

// Make screen messages. Returns the number of messages that were made.
// 'screen_width' and 'screen_height' are the virtual screen size and the top
// left is the origin.
// 'line_height' can be used if text is required to be on multiple lines. All
// messages created must be smaller than 'string_buffer_size'.
int dmlab_make_screen_messages(int screen_width, int screen_height,
                               int line_height, int string_buffer_size);

// Get a screen message for rendering.
// Shall be called after dmlab_make_screen_messages with a
// 'message_id' greater than or equal to zero and less than that returned by
// dmlab_make_screen_messages.
// Coordinates 'x' and 'y' are in screen space where top left is 0, 0 and
// bottom right is width, height from dmlab_make_screen_messages.
// 'buffer' must be large enough to hold string_buffer_size characters including
// null terminator. 'shadow' is whether to render a black offset drop shadow.
// 'rgba' is the color and alpha of the text.
void dmlab_get_screen_message(int message_id, char* buffer, int* x, int* y,
                              int* align_l0_r1_c2, int* shadow, float rgba[4]);

// Make filled rectangles. Returns the number of rectangles that were made.
// 'screen_width' and 'screen_height' are the virtual screen size and the top
// left is the origin.
int dmlab_make_filled_rectangles(int screen_width, int screen_height);

// Get a filled rectangle for rendering.
// Shall be called after dmlab_make_filled_rectangles with a
// 'rectangle_id' greater than or equal to zero and less than that returned by
// dmlab_make_screen_messages.
// Coordinates 'x', 'y', 'width' and 'height' are in screen space where top left
// is 0, 0 and bottom right is width, height from dmlab_make_filled_rectangles.
void dmlab_get_filled_rectangle(int rectangle_id, int* x, int* y, int* width,
                                int* height, float rgba[4]);

// Calls script to retrieve player position and velocity deltas.
// 'entity_id' is the ID of the triggering entity.
// 'entityPosition' is the position of the triggering entity.
// 'playerPosition' is the current position of the player.
// 'playerVelocity' is the current velocity of the player.
// 'playerPositionDelta' retrieves the position delta for the player.
// 'playerVelocityDelta' retrieves the velocity delta for the player.
void dmlab_lua_mover(int entity_id, const vec3_t entityPosition,
                     const vec3_t playerPosition, const vec3_t playerVelocity,
                     vec3_t playerPositionDelta, vec3_t playerVelocityDelta);

// Calls script with a game event and associated data.
// 'event_name' is the name of the event.
// 'count' is then number of vals.
// 'vals' is data associated with the event.
void dmlab_game_event(const char* event_name, int count, const float vals[]);

// Called on the spawning of each player.
void dmlab_spawn_inventory(playerState_t* player_state, int is_bot);

// Called on the update of each player.
void dmlab_update_inventory(playerState_t* player_state, int is_bot);

// Called during G_InitSessionData.
char dmlab_select_team(int player_id, const char* player_name);

// Called during SV_ClientEnterWorld. 'info' contains a '\' separated dictionary
// of player settings. This function is called so that the game can override any
// of these settings.
qboolean dmlab_update_player_info(int clientId, char* info, int info_size);

// Called at the start of entity update.
void dmlab_entities_clear(void);

// Called on each active entity during entity update.
void dmlab_entities_add(int entity_id, int user_id, int type, int flags,
                        float position[3], const char* classname);

qboolean dmlab_custom_view(refdef_t* camera);

int dmlab_dynamic_spawn_entity_count(void);

void dmlab_read_dynamic_spawn_entity(int entity_index, char* spawn_var_chars,
                                     int* num_spawn_var_chars,
                                     int spawn_var_offsets[][2],
                                     int* num_spawn_vars);

void dmlab_clear_dynamic_spawn_entities(void);

int dmlab_register_dynamic_items(void);
void dmlab_read_dynamic_item_name(int item_index, char* item_name);
void dmlab_new_client_info(int player_idx, const char* player_name,
                           const char* player_model);

#endif  // DML_ENGINE_CODE_DEEPMIND_DM_LOCAL_H_
