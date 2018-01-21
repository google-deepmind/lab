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

#ifndef DML_DEEPMIND_MODEL_GENERATION_LUA_MODEL_H_
#define DML_DEEPMIND_MODEL_GENERATION_LUA_MODEL_H_

#include "deepmind/include/deepmind_calls.h"
#include "deepmind/lua/class.h"
#include "deepmind/lua/lua.h"

namespace deepmind {
namespace lab {

class LuaModel : public lua::Class<LuaModel> {
  friend class Class;
  static const char* ClassName();

 public:
  // '*calls' owned by the caller and should out-live this object.
  explicit LuaModel(const DeepmindCalls* calls) : calls_(calls) {}

  // Registers the class as well as member functions.
  static void Register(lua_State* L);

  lua::NResultsOr CreateCone(lua_State* L);
  lua::NResultsOr CreateCube(lua_State* L);
  lua::NResultsOr CreateCylinder(lua_State* L);
  lua::NResultsOr CreateSphere(lua_State* L);
  lua::NResultsOr CreateHierarchy(lua_State* L);
  lua::NResultsOr CreateCircularLayout(lua_State* L);
  lua::NResultsOr CreateLinearLayout(lua_State* L);
  lua::NResultsOr LoadMD3(lua_State* L);
  lua::NResultsOr SaveMD3(lua_State* L);

 private:
  const DeepmindCalls* calls_;
};

}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_MODEL_GENERATION_LUA_MODEL_H_
