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

#ifndef DML_ENGINE_CODE_DEEPMIND_DM_PUBLIC_H_
#define DML_ENGINE_CODE_DEEPMIND_DM_PUBLIC_H_

typedef enum deepmindImport_enum {
  DEEPMIND_UPDATE_SPAWN_VARS,
  DEEPMIND_FIND_ITEM,
  DEEPMIND_ITEM_COUNT,
  DEEPMIND_ITEM,
  DEEPMIND_CLEAR_ITEMS,
  DEEPMIND_FINISH_MAP,
  DEEPMIND_CAN_PICKUP,
  DEEPMIND_OVERRIDE_PICKUP,
  DEEPMIND_CAN_TRIGGER,
  DEEPMIND_OVERRIDE_TRIGGER,
  DEEPMIND_OVERRIDE_LOOKAT,
  DEEPMIND_REWARD_OVERRIDE,
  DEEPMIND_SET_PLAYER_STATE,
  DEEPMIND_MAKE_SCREEN_MESSAGES,
  DEEPMIND_GET_SCREEN_MESSAGE,
  DEEPMIND_MAKE_FILLED_RECTANGLES,
  DEEPMIND_GET_FILLED_RECTANGLE,
  DEEPMIND_PLAYER_SCORE,
  DEEPMIND_LUA_MOVER,
  DEEPMIND_TEAM_SELECT,
  DEEPMIND_ENTITIES_CLEAR,
  DEEPMIND_ENTITIES_ADD,
  DEEPMIND_CUSTOM_VIEW,
} deepmindImport_t;

#endif  // DML_ENGINE_CODE_DEEPMIND_DM_PUBLIC_H_
