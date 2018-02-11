// Copyright (C) 2016-2018 Google Inc.
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
  // [0, 1, -]
  static lua::NResultsOr Require(lua_State* L);

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

  // Constructs a LuaMazeGeneration with a random layout of rooms and corridors.
  //   Keyword Arguments:
  //     'seed' (number, required) - Seed value for the random number generator.
  //     'height' (number, required) - Height of maze.
  //     'width' (number, required) - Width of maze.
  //     'maxRooms' (number, default=0) - Maximum number of rooms.
  //     'maxVariations' (number, default=26) - Maximum number of variations.
  //     'roomMinSize' (number, default=3) - Minimum room width or height.
  //     'roomMaxSize' (number, default=7) - Maximum room width or height.
  //     'retryCount' (number, default=1000) - Maximum number of attempts to lay
  //         out rooms.
  //     'simplify' (bool, default=true) - Remove dead ends and horseshoe bends.
  //     'extraConnectionProbability' (number, default=0.05) - Probability of
  //         additional connections between adjacent rooms (1 is always added).
  //     'roomSpawnCount' (number, default=0) - The number of spawn locations
  //         per room.
  //     'spawn' (character, default='P') - The character that specifies spawn
  //         points.
  //     'roomObjectCount' (number, default=0) - The number of objects per room.
  //     'object' (character, default='G') - The character that specifies object
  //         positions.
  //     'hasDoors' (bool, default=false) - Whether to add doors to corridors
  //         between rooms.
  // [1, 1, e]
  static lua::NResultsOr CreateRandom(lua_State* L);

  // Implements rotate(clockwiseRotations), which creates and returns a rotated
  // copy of the maze.
  // Rotations are 90 degrees clockwise * 'clockwiseRotations' times.
  // If clockwiseRotations is a multiple of 4, does nothing.
  // If clockwiseRotations is negative, rotates counter-clockwise.
  // [1, 1, e]
  // Returns new maze object.
  lua::NResultsOr Rotate(lua_State* L);

  // Implements paste(row, col, maze), to copy the entity layer values
  // of `maze` into this object's entity layer starting at (row,col).
  // Parts of `maze` that lie outside the current bounds are silently ignored.
  // [3, 1, e]
  lua::NResultsOr Paste(lua_State* L);

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

  // Sets a rectangular area of the maze to the specified character.
  // The rectangle is clamped to the maze's bounds, so its legal to specify
  // a rectangle that is partially (or entirely) outside the maze.
  //   Keyword Arguments:
  //     'row' (number, required) - start row of the rectangle to set.
  //         1 is the first row, but it's valid to specify a row <=0, if your
  //         rectangle starts above the maze.
  //     'col' (number, required) - start column of the rectangle to set.
  //         1 is the first column, but it's valid to specify a column <=0, if
  //          your rectangle starts left of the maze.
  //     'height' (number, required) - height of the rectangle to set.
  //         A height of 0 is an empty rectangle, so nothing will be set.
  //     'width' (number, required) - width of the rectangle to set.
  //         A width of 0 is an empty rectangle, so nothing will be set.
  //     'character' (character, required) - all values within the rectangle
  //         are set this character.
  // [1, 1, e]
  lua::NResultsOr FillEntityRect(lua_State* L);

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

  // Implements toWorldPos(row, column), returning X, Y.
  // [2, 2, e]
  lua::NResultsOr ToWorldPos(lua_State* L);

  // Implements fromWorldPos(x, y), returning row, column.
  // [2, 2, e]
  lua::NResultsOr FromWorldPos(lua_State* L);

  // Implements visitFill(goal, f), where f will be called with the row, column,
  // distance for each cell attached to the goal.
  // [1, 0, e]
  lua::NResultsOr VisitFill(lua_State* L);

  // Implements visitRandomPath(start, goal, f), where a random path will be
  // computed from start to goal and f will be called for each cell traversed.
  // Returns whether a path was found.
  // [1, 1, e]
  lua::NResultsOr VisitRandomPath(lua_State* L);

  // Implementation of CountEntities and CountVariations.
  // [1, 1, e]
  lua::NResultsOr CountCharacters(lua_State* L,
                                  maze_generation::TextMaze::Layer layer);

  // Implements countEntities(entities), where entities is a string of entities
  // to be counted.
  // Returns the count.
  // [1, 1, e]
  lua::NResultsOr CountEntities(lua_State* L);

  // Implements countVariations(varations), where varations is a string of
  // varations to be counted.
  // Returns the count.
  // [1, 1, e]
  lua::NResultsOr CountVariations(lua_State* L);

  maze_generation::TextMaze text_maze_;

  static std::uint64_t mixer_seq_;
};

}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_ENGINE_LUA_MAZE_GENERATION_H_
