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

#include "deepmind/model_generation/lua_transform.h"

#include "deepmind/lua/bind.h"
#include "deepmind/lua/push.h"
#include "deepmind/lua/read.h"
#include "deepmind/lua/table_ref.h"
#include "deepmind/model_generation/geometry_util.h"
#include "deepmind/model_generation/transform_lua.h"

namespace deepmind {
namespace lab {

int LuaTransform::Require(lua_State* L) {
  auto table = lua::TableRef::Create(L);
  table.Insert("translate", &lua::Bind<LuaTransform::CreateTranslation>);
  table.Insert("rotate", &lua::Bind<LuaTransform::CreateRotation>);
  table.Insert("scale", &lua::Bind<LuaTransform::CreateScaling>);
  lua::Push(L, table);
  return 1;
}

lua::NResultsOr LuaTransform::CreateTranslation(lua_State* L) {
  std::array<float, 3> t;
  if (lua::Read(L, -1, &t)) {
    Transform xfrm;
    xfrm = Eigen::Translation3f(t[0], t[1], t[2]);
    Push(L, xfrm);
    return 1;
  }
  return "[transform.translate] Must call with offset vector, "
         "recieved: " +
         lua::ToString(L, -1);
}

using geometry::kPi;

lua::NResultsOr LuaTransform::CreateRotation(lua_State* L) {
  float angle;
  std::array<float, 3> a;
  if (lua::Read(L, -2, &angle) && lua::Read(L, -1, &a)) {
    Transform xfrm;
    xfrm = Eigen::AngleAxis<float>(angle * kPi / 180.0f,  // Convert to radians.
                                   Eigen::Vector3f(a[0], a[1], a[2]));
    Push(L, xfrm);
    return 1;
  }
  return "[transform.translate] Must call with angle (in degrees) and rotation "
         "axis, "
         "recieved: " +
         lua::ToString(L, -2) + ", " + lua::ToString(L, -1);
}

lua::NResultsOr LuaTransform::CreateScaling(lua_State* L) {
  std::array<float, 3> s;
  if (lua::Read(L, -1, &s)) {
    Transform xfrm;
    xfrm = Eigen::Scaling(s[0], s[1], s[2]);
    Push(L, xfrm);
    return 1;
  }
  return "[transform.rotate] Must call with scaling factor vector, "
         "recieved: " +
         lua::ToString(L, -1);
}

}  // namespace lab
}  // namespace deepmind
