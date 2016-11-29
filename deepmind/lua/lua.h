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
//
// C++ header for Lua

#ifndef DML_DEEPMIND_LUA_LUA_H_
#define DML_DEEPMIND_LUA_LUA_H_

extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}

#include <cstddef>

namespace deepmind {
namespace lab {
namespace lua {

// This is equivalent to calling #table in Lua.
// The length is only well defined if all integer keys of the table are
// contiguous and start at 1. Non integer keys do not affect this value.
inline std::size_t ArrayLength(lua_State* L, int idx) {
#if LUA_VERSION_NUM == 501
  return lua_objlen(L, idx);
#elif LUA_VERSION_NUM == 502
  return lua_rawlen(L, idx);
#else
#error Only Luajit, Lua 5.1 and 5.2 are supported.
#endif
}

}  // namespace lua
}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_LUA_LUA_H_
