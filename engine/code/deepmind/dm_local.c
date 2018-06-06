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

#include "dm_local.h"
#include "dm_public.h"

int dmlab_dynamic_spawn_entity_count(void) {
  return trap_DeepmindCallback(DEEPMIND_DYNAMIC_SPAWN_ENTITY_COUNT, 0, 0, 0, 0,
                               0, 0, 0, 0, 0, 0, 0, 0);
}

void dmlab_read_dynamic_spawn_entity(int entity_index, char* spawn_var_chars,
                                     int* num_spawn_var_chars,
                                     int spawn_var_offsets[][2],
                                     int* num_spawn_vars) {
  trap_DeepmindCallback(DEEPMIND_READ_DYNAMIC_SPAWN_ENTITY,  //
                        (intptr_t)entity_index,
                        (intptr_t)spawn_var_chars,      //
                        (intptr_t)num_spawn_var_chars,  //
                        (intptr_t)spawn_var_offsets,    //
                        (intptr_t)num_spawn_vars, 0, 0, 0, 0, 0, 0, 0);
}

void dmlab_clear_dynamic_spawn_entities(void) {
  trap_DeepmindCallback(DEEPMIND_CLEAR_DYNAMIC_SPAWN_ENTITIES, 0, 0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0, 0);
}

int dmlab_register_dynamic_items(void) {
  return trap_DeepmindCallback(DEEPMIND_REGISTER_DYNAMIC_ITEMS, 0, 0, 0, 0, 0,
                               0, 0, 0, 0, 0, 0, 0);
}
void dmlab_read_dynamic_item_name(int item_index, char* item_name) {
  trap_DeepmindCallback(DEEPMIND_READ_DYNAMIC_ITEM_NAME,  //
                        (intptr_t)item_index,
                        (intptr_t)item_name,  //
                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

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

int dmlab_make_extra_entities(void) {
  return trap_DeepmindCallback(DEEPMIND_MAKE_EXTRA_ENTITIES,
                               0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

int dmlab_read_extra_entity(int entity_id,
                            char* spawn_var_chars,
                            int* num_spawn_var_chars,
                            int spawn_var_offsets[][2],
                            int* num_spawn_vars) {
  return trap_DeepmindCallback(DEEPMIND_READ_EXTRA_ENTITY,          //
                               (intptr_t)entity_id,            //
                               (intptr_t)spawn_var_chars,      //
                               (intptr_t)num_spawn_var_chars,  //
                               (intptr_t)spawn_var_offsets,    //
                               (intptr_t)num_spawn_vars, 0, 0, 0, 0, 0, 0, 0);
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
               int* quantity, int* type, int* tag, int* move_type) {
  return trap_DeepmindCallback(
      DEEPMIND_ITEM, (intptr_t)index, (intptr_t)item_name,
      (intptr_t)max_item_name,                         //
      (intptr_t)classname, (intptr_t)max_classname,    //
      (intptr_t)model_name, (intptr_t)max_model_name,  //
      (intptr_t)quantity, (intptr_t)type, (intptr_t)tag, (intptr_t)move_type,
      0);
}

void dmlab_clearitems(void) {
  trap_DeepmindCallback(DEEPMIND_CLEAR_ITEMS,
                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

void dmlab_set_map_finished(int map_finished) {
  trap_DeepmindCallback(DEEPMIND_FINISH_MAP, (intptr_t)map_finished,
                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

qboolean dmlab_can_pickup(int entity_id, const playerState_t* ps) {
  return trap_DeepmindCallback(DEEPMIND_CAN_PICKUP, (intptr_t)entity_id,
                               (intptr_t)ps, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

qboolean dmlab_override_pickup(int entity_id, int* respawn,
                               const playerState_t* ps) {
  return trap_DeepmindCallback(DEEPMIND_OVERRIDE_PICKUP, (intptr_t)entity_id,
                               (intptr_t)respawn, (intptr_t)ps, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

qboolean dmlab_can_trigger(int entity_id, const char* target_name,
                           const playerState_t* ps) {
  return trap_DeepmindCallback(DEEPMIND_CAN_TRIGGER,
                               (intptr_t)entity_id, (intptr_t)target_name,
                               (intptr_t)ps, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

qboolean dmlab_override_trigger(int entity_id, const char* target_name,
                                const playerState_t* ps) {
  return trap_DeepmindCallback(DEEPMIND_OVERRIDE_TRIGGER,
                               (intptr_t)entity_id, (intptr_t)target_name,
                               (intptr_t)ps, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

void dmlab_trigger_lookat(int entity_id, qboolean looked_at, vec3_t position,
                          const playerState_t* ps) {
  trap_DeepmindCallback(DEEPMIND_OVERRIDE_LOOKAT,
                        (intptr_t)entity_id, (intptr_t)looked_at,
                        (intptr_t)position,
                        (intptr_t)ps, 0, 0, 0, 0, 0, 0, 0, 0);
}

int dmlab_reward_override(const char* reason_opt, int player_id, int team,
                          const int* other_player_id_opt,
                          const vec3_t origin_opt, int score) {
  return trap_DeepmindCallback(
      DEEPMIND_REWARD_OVERRIDE, (intptr_t)reason_opt, (intptr_t)player_id,
      (intptr_t)team, (intptr_t)other_player_id_opt, (intptr_t)origin_opt,
      (intptr_t)score, 0, 0, 0, 0, 0, 0);
}

void dmlab_player_state(const playerState_t* ps, const float eyePos[3],
                        int team_score, int other_team_score) {
  trap_DeepmindCallback(DEEPMIND_SET_PLAYER_STATE, (intptr_t)ps,
                        (intptr_t)eyePos, (intptr_t) team_score,
                        (intptr_t)other_team_score, 0, 0, 0, 0, 0, 0, 0, 0);
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
                              int* align_l0_r1_c2, int* shadow, float rgba[4]) {
  trap_DeepmindCallback(DEEPMIND_GET_SCREEN_MESSAGE, (intptr_t)message_id,
                        (intptr_t)buffer, (intptr_t)x, (intptr_t)y,
                        (intptr_t)align_l0_r1_c2, (intptr_t)shadow,
                        (intptr_t)rgba, 0, 0, 0, 0, 0);
}

int dmlab_make_filled_rectangles(int screen_width, int screen_height) {
  return trap_DeepmindCallback(DEEPMIND_MAKE_FILLED_RECTANGLES,
                               (intptr_t)screen_width, (intptr_t)screen_height,
                               0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

void dmlab_get_filled_rectangle(int rectangle_id, int* x, int* y, int* width,
                                int* height, float rgba[4]) {
  trap_DeepmindCallback(DEEPMIND_GET_FILLED_RECTANGLE, (intptr_t)rectangle_id,
                        (intptr_t)x, (intptr_t)y, (intptr_t)width,
                        (intptr_t)height, (intptr_t)rgba, 0, 0, 0, 0, 0, 0);
}

int dmlab_player_score(void) {
  return trap_DeepmindCallback(DEEPMIND_PLAYER_SCORE, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                               0, 0, 0);
}

void dmlab_lua_mover(int entity_id, const vec3_t entityPosition,
                     const vec3_t playerPosition, const vec3_t playerVelocity,
                     vec3_t playerPositionDelta, vec3_t playerVelocityDelta) {
  trap_DeepmindCallback(DEEPMIND_LUA_MOVER, (intptr_t)entity_id,
                        (intptr_t)entityPosition,
                        (intptr_t)playerPosition, (intptr_t)playerVelocity,
                        (intptr_t)playerPositionDelta,
                        (intptr_t)playerVelocityDelta, 0, 0, 0, 0, 0, 0);
}

void dmlab_game_event(const char* event_name, int count, const float vals[]) {
  trap_DeepmindCallback(DEEPMIND_GAME_EVENT, (intptr_t)event_name,
                        (intptr_t)count, (intptr_t)vals, 0, 0, 0, 0, 0, 0, 0, 0,
                        0);
}

void dmlab_spawn_inventory(playerState_t* player_state, int is_bot) {
  trap_DeepmindCallback(DEEPMIND_SPAWN_INVENTORY, (intptr_t)player_state,
                        is_bot, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

void dmlab_update_inventory(playerState_t* player_state, int is_bot) {
  trap_DeepmindCallback(DEEPMIND_UPDATE_INVENTORY, (intptr_t)player_state,
                        is_bot, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

char dmlab_select_team(int player_id, const char* player_name) {
  return (char)trap_DeepmindCallback(DEEPMIND_TEAM_SELECT, (intptr_t)player_id,
                                     (intptr_t)player_name, 0, 0, 0, 0, 0, 0, 0,
                                     0, 0, 0);
}

void dmlab_entities_clear(void) {
  trap_DeepmindCallback(DEEPMIND_ENTITIES_CLEAR, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                        0, 0);
}

void dmlab_entities_add(int entity_id, int user_id, int type, int flags,
                           float position[3], const char* classname) {
  trap_DeepmindCallback(DEEPMIND_ENTITIES_ADD, (intptr_t)entity_id,
                        (intptr_t)user_id, (intptr_t)type, (intptr_t)flags,
                        (intptr_t)position, (intptr_t)classname, 0, 0, 0, 0, 0,
                        0);
}

qboolean dmlab_custom_view(refdef_t* camera) {
  return trap_DeepmindCallback(DEEPMIND_CUSTOM_VIEW, (intptr_t)camera, 0, 0, 0,
                               0, 0, 0, 0, 0, 0, 0, 0);
}

qboolean dmlab_update_player_info(int clientId, char* info, int info_size) {
  return trap_DeepmindCallback(DEEPMIND_UPDATE_PLAYER_INFO, (intptr_t)clientId,
                               (intptr_t)info, (intptr_t)info_size, 0, 0, 0, 0,
                               0, 0, 0, 0, 0);
}

void dmlab_new_client_info(int player_idx, const char* player_name,
                           const char* player_model) {
  trap_DeepmindCallback(DEEPMIND_NEW_CLIENT_INFO, (intptr_t)player_idx,
                        (intptr_t)player_name, (intptr_t)player_model, 0, 0, 0,
                        0, 0, 0, 0, 0, 0);
}
