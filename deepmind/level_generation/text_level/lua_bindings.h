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
// Lua bindings for the text level generation.

#ifndef DML_DEEPMIND_LEVEL_GENERATION_TEXT_LEVEL_LUA_BINDINGS_H_
#define DML_DEEPMIND_LEVEL_GENERATION_TEXT_LEVEL_LUA_BINDINGS_H_

#include <cstddef>
#include <string>
#include <vector>

#include "deepmind/level_generation/text_level/translate_text_level.h"
#include "deepmind/lua/class.h"
#include "deepmind/lua/n_results_or.h"

namespace deepmind {
namespace lab {

// An entity callback that calls a Lua function.
//
// The Lua function f is called as f(i, j, ent, emitter) with the semantics
// documented in the text-level specification. The function must be located
// at position idx on the Lua stack. The emitter is a Lua class shown below.
//
// Make sure to register LuaSnippetEmitter with the VM that is used by this
// callback:
//
//    LuaSnippetEmitter::Register(L);
//    // put callback function on stack
//    auto cb = std::bind(LuaCustomEntityCallback, L, -1, _1, _2, _3, _4, _5);
//    string map = TranslateTextLevel(ents, vars, &rng, cb);
//
// The return value is interpreted as follows:
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
    std::vector<std::string>* out);

// LuaSnippetEmitter wraps the MapSnippetEmitter for use from Lua.
class LuaSnippetEmitter : public lua::Class<LuaSnippetEmitter> {
  friend class Class;
  static const char* ClassName() {
    return "deepmind.lab.TextLevelEmitter";
  }

 public:
  // Lua member functions. See below for documentation. All functions return
  // strings that are suitable for returning from the custom entity callback.
  static void Register(lua_State* L) {
    const Class::Reg methods[] = {
        {"makeEntity", Member<&LuaSnippetEmitter::MakeEntity>},
        {"makeSpawnPoint", Member<&LuaSnippetEmitter::MakeSpawnPoint>},
        {"makeDoor", Member<&LuaSnippetEmitter::MakeDoor>},
        {"makeFenceDoor", Member<&LuaSnippetEmitter::MakeFenceDoor>},
        {"addPlatform", Member<&LuaSnippetEmitter::AddPlatform>},
        {"addGlassColumn", Member<&LuaSnippetEmitter::AddGlassColumn>},
    };
    Class::Register(L, methods);
  }

  explicit LuaSnippetEmitter(const MapSnippetEmitter* emitter)
      : emitter_(*emitter) {}

  // makeEntity({i, j[, height], classname[, attributes]}),
  // use table call conventions.
  //
  // Creates a custom entity at the centre of cell (i, j) with the given class
  // name and list of attributes (which must be a string-to-string map).
  // If the height is given, the entity will be created at
  lua::NResultsOr MakeEntity(lua_State* L);

  // makeSpawnPoint({i, j[, height, angleRad]}), use table call conventions.
  //
  // Creates a spawn point at cell (i, j) facing in direction of angle_rad,
  // which is the angle (in radians) counter-clockwise from West.
  lua::NResultsOr MakeSpawnPoint(lua_State* L);

  // makeDoor({i, j, isEastWest}), use table call conventions.
  //
  // Creates a door in cell (i, j). If isEastWest is true, the door is of type
  // 'I', otherwise it is of type 'H'.
  lua::NResultsOr MakeDoor(lua_State* L);

  // makeFenceDoor({i, j, isEastWest}), use table call conventions.
  //
  // Creates a fence door in cell (i, j). If isEastWest is true, the door is of
  // type 'I', otherwise it is of type 'H'.
  lua::NResultsOr MakeFenceDoor(lua_State* L);

  // makePlatform({i, j, height}), use table call conventions.
  //
  // Adds a platform cell in cell (i, j) and place the platform at the given
  // height.
  lua::NResultsOr AddPlatform(lua_State* L);

  // addGlassColumn({i, j, height}), use table call conventions.
  //
  // Adds an invisible column all the way from the ground to the given height in
  // cell (i, j).
  lua::NResultsOr AddGlassColumn(lua_State* L);

 private:
  const MapSnippetEmitter& emitter_;
};

}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_LEVEL_GENERATION_TEXT_LEVEL_LUA_BINDINGS_H_
