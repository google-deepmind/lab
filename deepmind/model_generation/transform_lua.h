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

#ifndef DML_DEEPMIND_MODEL_GENERATION_TRANSFORM_LUA_H_
#define DML_DEEPMIND_MODEL_GENERATION_TRANSFORM_LUA_H_

#include "deepmind/lua/lua.h"
#include "deepmind/lua/read.h"
#include "deepmind/model_generation/transform.h"

namespace deepmind {
namespace lab {

// Construct a table representation of transform and push it onto the stack.
// [1, 0, -]
void Push(lua_State* L, const Transform& transform);

// Read a transform from the given position in the stack and load it onto xfrm.
// Returns whether the transform was successfully read.
// [0, 0, -]
lua::ReadResult Read(lua_State* L, int idx, Transform* transform);

}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_MODEL_GENERATION_TRANSFORM_LUA_H_
