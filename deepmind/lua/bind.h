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

#ifndef DML_DEEPMIND_LUA_BIND_H_
#define DML_DEEPMIND_LUA_BIND_H_

#include <string>

#include "deepmind/lua/lua.h"
#include "deepmind/lua/n_results_or.h"

namespace deepmind {
namespace lab {
namespace lua {

// Binds a function that returns a NResultsOr to properly propagate errors.
// This is does not have C-language linkage but seems to work.
template <NResultsOr (&F)(lua_State*)>
int Bind(lua_State* L) {
  {
    NResultsOr result_or = F(L);
    if (result_or.ok()) {
      return result_or.n_results();
    } else {
      lua_pushlstring(L, result_or.error().data(), result_or.error().size());
    }
  }
  // "lua_error" performs a longjmp, which is not allowed in C++ except in
  // specific situations. We take care that no objects with non-trivial
  // destructors exist when lua_error is called.
  return lua_error(L);
}

}  // namespace lua
}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_LUA_BIND_H_
