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

#ifndef DML_DEEPMIND_LUA_PUSH_SCRIPT_H_
#define DML_DEEPMIND_LUA_PUSH_SCRIPT_H_

#include <cstddef>
#include <cstring>
#include <string>

#include "deepmind/lua/lua.h"
#include "deepmind/lua/n_results_or.h"

namespace deepmind {
namespace lab {
namespace lua {

// Push a script onto stack ready for calling. If script has a syntax error it
// returns that instead.
// [0, (+1|0), -]
NResultsOr PushScript(        //
    lua_State* L,             //
    const char* buffer,       //
    std::size_t buffer_size,  //
    const char* script_name);

inline NResultsOr PushScript(  //
    lua_State* L,              //
    const char* buffer,        //
    const char* script_name) {
  return PushScript(L, buffer, std::strlen(buffer), script_name);
}

inline NResultsOr PushScript(
    lua_State* L,
    const std::string& script,
    const std::string& script_name) {
  return PushScript(L, script.data(), script.size(), script_name.data());
}

// Loads a script from file and onto the stack ready for calling. If the file
// cannot be loaded or has a syntax error that error is returned instead.
// [0, (+1|0), -]
NResultsOr PushScriptFile(  //
    lua_State* L,           //
    const char* filename);

inline NResultsOr PushScriptFile(  //
    lua_State* L,                  //
    const std::string& filename) {
  return PushScriptFile(L, filename.data());
}

}  // namespace lua
}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_LUA_PUSH_SCRIPT_H_
