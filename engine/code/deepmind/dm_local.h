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

#ifndef DML_ENGINE_CODE_DEEPMIND_DM_LOCAL_H_
#define DML_ENGINE_CODE_DEEPMIND_DM_LOCAL_H_

#include "../qcommon/q_shared.h"

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
               int* quantity, int* type, int* tag);

// Clear the current list of registered items. Called just before loading a new
// map.
void dmlab_clearitems(void);

// Sets whether the current map should finish.
void dmlab_set_map_finished(int map_finished);

// Returns if we can pickup the specified entity.
qboolean dmlab_can_pickup(int entity_id);

// Hook to allow customization of the entity's pickup behaviour. Users are
// expected to implement their own game logic for picking up the specified item.
// Can also return the default respawn time for the entity.
// Returns true if the pickup behaviour has been overridden.
qboolean dmlab_override_pickup(int entity_id, int* respawn);

// Consume reward stored in external context for player 'player_id'.
int dmlab_external_reward(int player_id);

// Get the current player score.
int dmlab_player_score(void);

// Sets the predicted player info.
void dmlab_predicted_player_state(const playerState_t* ps);

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
// null terminator.
void dmlab_get_screen_message(int message_id, char* buffer, int* x, int* y,
                              int* align_l0_r1_c2);

#endif  // DML_ENGINE_CODE_DEEPMIND_DM_LOCAL_H_
