// Copyright (C) 2017 Google Inc.
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

#ifndef DML_DEEPMIND_MODEL_GENERATION_LUA_TRANSFORM_H_
#define DML_DEEPMIND_MODEL_GENERATION_LUA_TRANSFORM_H_

#include "deepmind/lua/lua.h"
#include "deepmind/lua/n_results_or.h"

namespace deepmind {
namespace lab {

class LuaTransform {
 public:
  // Returns table of constructors and standalone functions.
  // [0 1 -]
  static int Require(lua_State* L);

  static lua::NResultsOr CreateTranslation(lua_State* L);
  static lua::NResultsOr CreateRotation(lua_State* L);
  static lua::NResultsOr CreateScaling(lua_State* L);
};

}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_MODEL_GENERATION_LUA_TRANSFORM_H_
