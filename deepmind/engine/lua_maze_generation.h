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

#ifndef DML_DEEPMIND_ENGINE_LUA_MAZE_GENERATION_H_
#define DML_DEEPMIND_ENGINE_LUA_MAZE_GENERATION_H_

#include <utility>

#include "deepmind/level_generation/text_maze_generation/text_maze.h"
#include "deepmind/lua/class.h"
#include "deepmind/lua/lua.h"
#include "deepmind/lua/n_results_or.h"

namespace deepmind {
namespace lab {

// A Lua class that exposes an interface to maze_generation::TextMaze.
class LuaMazeGeneration : public lua::Class<LuaMazeGeneration> {
  friend class Class;
  static const char* ClassName();

 public:
  explicit LuaMazeGeneration(maze_generation::TextMaze text_maze)
      : text_maze_(std::move(text_maze)) {}

  explicit LuaMazeGeneration(const maze_generation::Size& size)
      : text_maze_(size) {}

  // Registers classes metatable with Lua.
  // [0, 0, -]
  static void Register(lua_State* L);

  // Returns table of constructors and standalone functions.
  // [0 1 -]
  static int Require(lua_State* L);

 private:
  // Constructs a LuaMazeGeneration.
  // Can be either constructed by size:
  //   Keyword Arguments:
  //     'height' (number) - Height of maze.
  //     'width' (number) - Width of maze.
  //   Returns a LuaMazeGeneration on the stack wrapping a text_maze_
  //   constructed with the given size.
  // Or constructed with an existing text maze:
  //   Keyword Arguments:
  //     'entity' (string) - Entity layer of maze.
  //     'variations' (string, optional) - Variations layer of maze.
  // [1, 1, e]
  static lua::NResultsOr Create(lua_State* L);

  // Returns the entity layer of the text_maze_ on the Lua stack.
  // [0, 1, -]
  lua::NResultsOr EntityLayer(lua_State* L);

  // Returns the entity layer of the text_maze_ on the Lua stack.
  // [0, 1, -]
  lua::NResultsOr VariationsLayer(lua_State* L);

  // Implements getEntityCell(row, col), returning the character at that
  // location within the entity layer.
  // [2, 1, -]
  lua::NResultsOr GetEntityCell(lua_State* L);

  // Implements setEntityCell(row, col, character), setting the character at
  // that location within the entity layer.
  // [3, 0, -]
  lua::NResultsOr SetEntityCell(lua_State* L);

  // Implements getVariationsCell(row, col), returning the character at that
  // location within the variations layer.
  // [2, 1, -]
  lua::NResultsOr GetVariationsCell(lua_State* L);

  // Implements setVariationsCell(row, col, character), setting the character at
  // that location within the variations layer.
  // [3, 0, -]
  lua::NResultsOr SetVariationsCell(lua_State* L);

  // Implements findRooms, returning a table of all rooms in the entity layer.
  // [1, 0, e]
  lua::NResultsOr FindRooms(lua_State* L);

  // Implements size(), returning height, width.
  // [0, 2, -]
  lua::NResultsOr Size(lua_State* L);

  // Implements visitFill(goal, f), where f will be called with the row, column,
  // distance for each cell attached to the goal.
  // [1, 0, e]
  lua::NResultsOr VisitFill(lua_State* L);

  maze_generation::TextMaze text_maze_;
};

}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_ENGINE_LUA_MAZE_GENERATION_H_
