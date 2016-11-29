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

#include "dm_local.h"
#include "dm_public.h"

int dmlab_update_spawn_vars(char* spawn_var_chars,
                            int* num_spawn_var_chars,
                            int spawn_var_offsets[][2],
                            int* num_spawn_vars) {
  return trap_DeepmindCallback(DEEPMIND_UPDATE_SPAWN_VARS,     //
                               (intptr_t)spawn_var_chars,      //
                               (intptr_t)num_spawn_var_chars,  //
                               (intptr_t)spawn_var_offsets,    //
                               (intptr_t)num_spawn_vars, 0, 0, 0, 0, 0, 0, 0,
                               0);
}

int dmlab_finditem(const char* classname, int* index) {
  return trap_DeepmindCallback(DEEPMIND_FIND_ITEM, (intptr_t)classname,
                               (intptr_t)index, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

int dmlab_itemcount(void) {
  return trap_DeepmindCallback(DEEPMIND_ITEM_COUNT,
                               0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

int dmlab_item(int index, char* item_name, int max_item_name,  //
               char* classname, int max_classname,             //
               char* model_name, int max_model_name,           //
               int* quantity, int* type, int* tag) {
  return trap_DeepmindCallback(
      DEEPMIND_ITEM, (intptr_t)index, (intptr_t)item_name,
      (intptr_t)max_item_name,                         //
      (intptr_t)classname, (intptr_t)max_classname,    //
      (intptr_t)model_name, (intptr_t)max_model_name,  //
      (intptr_t)quantity, (intptr_t)type, (intptr_t)tag, 0, 0);
}

void dmlab_clearitems(void) {
  trap_DeepmindCallback(DEEPMIND_CLEAR_ITEMS,
                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

void dmlab_set_map_finished(int map_finished) {
  trap_DeepmindCallback(DEEPMIND_FINISH_MAP, (intptr_t)map_finished,
                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

qboolean dmlab_can_pickup(int entity_id) {
  return trap_DeepmindCallback(DEEPMIND_CAN_PICKUP, (intptr_t)entity_id,
                               0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

qboolean dmlab_override_pickup(int entity_id, int* respawn) {
  return trap_DeepmindCallback(DEEPMIND_OVERRIDE_PICKUP, (intptr_t)entity_id,
                               (intptr_t)respawn, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

int dmlab_external_reward(int player_id) {
  return trap_DeepmindCallback(DEEPMIND_EXTERNAL_REWARD, (intptr_t)player_id,
                               0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

void dmlab_predicted_player_state(const playerState_t* ps) {
  trap_DeepmindCallback(DEEPMIND_SET_PREDICTED_PLAYER_STATE, (intptr_t)ps, 0, 0,
                        0, 0, 0, 0, 0, 0, 0, 0, 0);
}

int dmlab_make_screen_messages(int screen_width, int screen_height,
                               int line_height, int string_buffer_size) {
  return trap_DeepmindCallback(DEEPMIND_MAKE_SCREEN_MESSAGES,
                               (intptr_t)screen_width, (intptr_t)screen_height,
                               (intptr_t)line_height,
                               (intptr_t)string_buffer_size, 0, 0, 0, 0, 0, 0,
                               0, 0);
}

void dmlab_get_screen_message(int message_id, char* buffer, int* x, int* y,
                              int* align_l0_r1_c2) {
  trap_DeepmindCallback(DEEPMIND_GET_SCREEN_MESSAGE, (intptr_t)message_id,
                        (intptr_t)buffer, (intptr_t)x, (intptr_t)y,
                        (intptr_t)align_l0_r1_c2, 0, 0, 0, 0, 0, 0, 0);
}

int dmlab_player_score(void) {
  return trap_DeepmindCallback(DEEPMIND_PLAYER_SCORE, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                               0, 0, 0);
}
