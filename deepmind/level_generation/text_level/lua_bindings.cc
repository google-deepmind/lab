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

#include "deepmind/level_generation/text_level/lua_bindings.h"

#include <algorithm>
#include <functional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "deepmind/support/logging.h"
#include "deepmind/lua/call.h"
#include "deepmind/lua/class.h"
#include "deepmind/lua/lua.h"
#include "deepmind/lua/n_results_or.h"
#include "deepmind/lua/push.h"
#include "deepmind/lua/read.h"

namespace deepmind {
namespace lab {

// An entity callback that calls a Lua function.
//
// The Lua function f is called as f(i, j, ent) with the semantics documented in
// the text-level specification. The function must be located at position idx on
// the Lua stack. The return value is interpreted as follows:
//
//   * No results or a single result "nil" means that the function did not
//     handle the entity. (That is, default handlers may be invoked.)
//   * Otherwise, there must be a single result that is either a string or an
//     array of strings. All strings are appended  to the output collection, and
//     in this case the entity is considered handled.
//
// To handle an entity without emitting any output (e.g. to suppress a default
// handler, the Lua function may return an empty string.
//
// Any error in the Lua function (including a return value other than what is
// described above) results in the entity not having been handled, and no data
// is appended to the .map output.
bool LuaCustomEntityCallback(
    lua_State* L,
    int idx,
    std::size_t i,
    std::size_t j,
    char ent,
    const MapSnippetEmitter& emitter,
    std::vector<std::string>* out) {
  lua_pushvalue(L, idx);
  lua::Push(L, static_cast<lua_Integer>(i));
  lua::Push(L, static_cast<lua_Integer>(j));
  lua::Push(L, std::string(1, ent));
  LuaSnippetEmitter::CreateObject(L, &emitter);

  auto res = lua::Call(L, 4);
  std::vector<std::string> data(1);

  if (!res.ok()) {
    LOG(ERROR) << "User callback invocation failed ('" + res.error() + "')";
    return false;
  } else if ((res.n_results() == 0) ||
             (res.n_results() == 1 && lua_isnil(L, -1))) {
    VLOG(1) << "User callback(" << i << ", " << j << ", '" << ent
            << "') returned nil or nothing";
    lua_pop(L, res.n_results());
    return false;
  } else if ((res.n_results() > 1) ||
             !(lua::Read(L, -1, &data) || lua::Read(L, -1, &data.front()))) {
    LOG(ERROR) << "User callback returned invalid results.";
    lua_pop(L, res.n_results());
    return false;
  } else {
    auto is_empty = std::mem_fn(&std::string::empty);
    data.erase(std::remove_if(data.begin(), data.end(), is_empty), data.end());

    VLOG(1) << "User callback(" << i << ", " << j << ", '" << ent
            << "') returned " << data.size() << " non-empty string(s)";
    out->swap(data);

    lua_pop(L, res.n_results());
    return true;
  }
}

lua::NResultsOr LuaSnippetEmitter::MakeEntity(lua_State* L) {
  int i, j;
  std::string class_name;
  std::unordered_map<std::string, std::string> attrs;

  if (!lua::Read(L, 2, &i) ||
      !lua::Read(L, 3, &j) ||
      !lua::Read(L, 4, &class_name)) {
    return "Bad arguments";
  }

  if (lua_gettop(L) == 5 && !lua::Read(L, 5, &attrs)) {
    LOG(ERROR) << "Malformed attribute table in user callback; ignoring.";
  }

  lua::Push(L, emitter_.AddEntity(
      i, j, std::move(class_name),
      std::vector<std::pair<std::string, std::string>>(
          attrs.begin(), attrs.end())));
  return 1;
}

lua::NResultsOr LuaSnippetEmitter::MakeSpawnPoint(lua_State* L) {
  int i, j;
  double angle_rad;

  if (!lua::Read(L, 2, &i) || !lua::Read(L, 3, &j)) {
    return "Bad arguments";
  }

  if (!lua::Read(L, 4, &angle_rad)) {
    angle_rad = 0.0;
  }

  lua::Push(L, emitter_.AddSpawn(i, j, angle_rad));
  return 1;
}

lua::NResultsOr LuaSnippetEmitter::MakeDoor(lua_State* L) {
  int i, j;
  bool is_east_west;

  if (!lua::Read(L, 2, &i) ||
      !lua::Read(L, 3, &j) ||
      !lua::Read(L, 4, &is_east_west)) {
    return "Bad arguments";
  }

  lua::Push(L, emitter_.AddDoor(i, j, is_east_west ? 'I' : 'H'));
  return 1;
}

}  // namespace lab
}  // namespace deepmind
