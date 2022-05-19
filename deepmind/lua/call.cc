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

#include "deepmind/lua/call.h"

#include <string>
#include <utility>

#include "absl/log/check.h"
#include "deepmind/lua/n_results_or.h"
#include "deepmind/lua/push.h"
#include "deepmind/lua/read.h"

namespace deepmind {
namespace lab {
namespace lua {

extern "C" {
static int traceback(lua_State* L) {
  if (!lua_isstring(L, 1)) return 1;
  lua_getglobal(L, "debug");
  if (!lua_istable(L, -1)) {
    lua_pop(L, 1);
    return 1;
  }
  lua_getfield(L, -1, "traceback");
  if (!lua_isfunction(L, -1)) {
    lua_pop(L, 2);
    return 1;
  }
  lua_pushvalue(L, 1);
  lua_pushinteger(L, 2);
  lua_call(L, 2, 1);
  return 1;
}
}  // extern "C"

NResultsOr Call(lua_State* L, int nargs, bool with_traceback) {
  CHECK_GE(nargs, 0) << "Invalid number of arguments: " << nargs;
  int err_stackpos = 0;
  if (with_traceback) {
    err_stackpos = lua_gettop(L) - nargs;
    Push(L, traceback);
    lua_insert(L, err_stackpos);
  }
  if (lua_pcall(L, nargs, LUA_MULTRET, err_stackpos) != 0) {
    std::string error;
    if (!IsFound(Read(L, -1, &error))) {
      error = "Failed to retrieve error!";
    }
    if (with_traceback) {
      lua_remove(L, err_stackpos);
    }
    lua_pop(L, 1);
    return std::move(error);
  } else {
    if (with_traceback) {
      lua_remove(L, err_stackpos);
    }
    return lua_gettop(L) - err_stackpos + 1;
  }
}

}  // namespace lua
}  // namespace lab
}  // namespace deepmind
