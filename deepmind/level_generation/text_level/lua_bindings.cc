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
#include <string>
#include <utility>
#include <vector>

#include "absl/log/log.h"
#include "absl/container/flat_hash_map.h"
#include "deepmind/level_generation/text_level/translate_text_level.h"
#include "deepmind/lua/call.h"
#include "deepmind/lua/class.h"
#include "deepmind/lua/lua.h"
#include "deepmind/lua/n_results_or.h"
#include "deepmind/lua/push.h"
#include "deepmind/lua/read.h"
#include "deepmind/lua/table_ref.h"

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
    LOG(FATAL) << "User callback invocation failed ('" + res.error() + "')";
  } else if ((res.n_results() == 0) ||
             (res.n_results() == 1 && lua_isnil(L, -1))) {
    lua_pop(L, res.n_results());
    return false;
  } else if ((res.n_results() > 1) ||
             !(lua::Read(L, -1, &data) || lua::Read(L, -1, &data.front()))) {
    LOG(FATAL) << "User callback returned invalid results.";
  } else {
    auto is_empty = [](const std::string& s) { return s.empty(); };
    data.erase(std::remove_if(data.begin(), data.end(), is_empty), data.end());
    out->swap(data);

    lua_pop(L, res.n_results());
    return true;
  }
}

lua::NResultsOr LuaSnippetEmitter::MakeEntity(lua_State* L) {
  lua::TableRef table;
  if (!lua::Read(L, 2, &table)) {
    return "[makeEntity] - Invalid argument, it must be a table.";
  }

  double i, j, height;
  std::string classname;
  absl::flat_hash_map<std::string, std::string> attrs;

  if (!table.LookUp("i", &i) || !table.LookUp("j", &j) ||
      !table.LookUp("classname", &classname)) {
    return "[makeEntity] - Invalid arguments";
  }

  if (!table.LookUp("height", &height)) {
    height = 0;
  }

  if (table.Contains("attributes")) {
    if (!table.LookUp("attributes", &attrs)) {
      LOG(ERROR) << "[makeEntity] - Malformed attribute table in user "
                    "callback; ignoring.";
    }
  }

  lua::Push(L,
            emitter_.AddEntity(i, j, height, std::move(classname),
                               std::vector<std::pair<std::string, std::string>>(
                                   attrs.begin(), attrs.end())));
  return 1;
}

lua::NResultsOr LuaSnippetEmitter::MakeSpawnPoint(lua_State* L) {
  lua::TableRef table;
  if (!lua::Read(L, 2, &table)) {
    return "[makeSpawnPoint] - Invalid argument, it must be a table.";
  }

  double i, j, height, angle_rad;

  if (!table.LookUp("i", &i) || !table.LookUp("j", &j)) {
    return "[makeSpawnPoint] - Invalid arguments";
  }

  if (!table.LookUp("height", &height)) {
    height = 0;
  }

  if (!table.LookUp("angleRad", &angle_rad)) {
    angle_rad = 0.0;
  }

  lua::Push(L, emitter_.AddSpawn(i, j, height, angle_rad));
  return 1;
}

lua::NResultsOr LuaSnippetEmitter::MakeDoor(lua_State* L) {
  lua::TableRef table;
  if (!lua::Read(L, 2, &table)) {
    return "[makeDoor] - Invalid argument, it must be a table.";
  }

  double i, j;
  bool is_east_west;

  if (!table.LookUp("i", &i) || !table.LookUp("j", &j) ||
      !table.LookUp("isEastWest", &is_east_west)) {
    return "[makeDoor] - Invalid arguments";
  }

  lua::Push(L, emitter_.AddDoor(i, j, is_east_west ? 'I' : 'H'));
  return 1;
}

lua::NResultsOr LuaSnippetEmitter::MakeFenceDoor(lua_State* L) {
  lua::TableRef table;
  if (!lua::Read(L, 2, &table)) {
    return "[makeFenceDoor] - Invalid argument, it must be a table.";
  }

  double i, j;
  bool is_east_west;

  if (!table.LookUp("i", &i) || !table.LookUp("j", &j) ||
      !table.LookUp("isEastWest", &is_east_west)) {
    return "[makeFenceDoor] - Invalid arguments";
  }

  lua::Push(L, emitter_.AddFenceDoor(i, j, is_east_west ? 'I' : 'H'));
  return 1;
}

lua::NResultsOr LuaSnippetEmitter::AddPlatform(lua_State* L) {
  lua::TableRef table;
  if (!lua::Read(L, 2, &table)) {
    return "[addPlatform] - Invalid argument, it must be a table.";
  }

  double i, j, height;

  if (!table.LookUp("i", &i) || !table.LookUp("j", &j) ||
      !table.LookUp("height", &height)) {
    return "[addPlatform] - Invalid arguments";
  }

  lua::Push(L, emitter_.AddPlatform(i, j, height));
  return 1;
}

lua::NResultsOr LuaSnippetEmitter::AddGlassColumn(lua_State* L) {
  lua::TableRef table;
  if (!lua::Read(L, 2, &table)) {
    return "[addGlassColumn] - Invalid argument, it must be a table.";
  }

  double i, j, height;

  if (!table.LookUp("i", &i) || !table.LookUp("j", &j) ||
      !table.LookUp("height", &height)) {
    return "[addGlassColumn] - Invalid arguments";
  }

  lua::Push(L, emitter_.AddGlassColumn(i, j, height));
  return 1;
}

}  // namespace lab
}  // namespace deepmind
