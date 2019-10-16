// Copyright (C) 2016-2019 Google Inc.
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

#include "deepmind/lua/push_script.h"

#include <string>
#include <utility>

#include "absl/strings/str_cat.h"
#include "deepmind/lua/read.h"

namespace deepmind {
namespace lab {
namespace lua {

NResultsOr PushScript(lua_State* L, const char* buffer, std::size_t buffer_size,
                      const char* script_name) {
  if (luaL_loadbuffer(L, buffer, buffer_size, script_name)) {
    std::string error;
    if (!IsFound(Read(L, -1, &error)))
      error = "Failed to retrieve error!";
    return std::move(error);
  }
  return 1;
}

NResultsOr PushScriptFile(lua_State* L, const char* filename) {
  int ret = luaL_loadfile(L, filename);
  if (ret == LUA_ERRFILE) {
    return absl::StrCat("Failed to open file '", filename, "'");
  } else if (ret != 0) {
    std::string error;
    if (!IsFound(Read(L, -1, &error)))
      error = "Failed to retrieve error!";
    return std::move(error);
  }
  return 1;
}

}  // namespace lua
}  // namespace lab
}  // namespace deepmind
