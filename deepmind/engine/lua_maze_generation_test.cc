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

#include "deepmind/engine/lua_maze_generation.h"

#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "deepmind/lua/call.h"
#include "deepmind/lua/n_results_or_test_util.h"
#include "deepmind/lua/push_script.h"
#include "deepmind/lua/vm_test_util.h"

namespace deepmind {
namespace lab {

namespace {

using ::deepmind::lab::lua::testing::IsOkAndHolds;

class LuaMazeGenerationTest : public lua::testing::TestWithVm {
 protected:
  LuaMazeGenerationTest() {
    LuaMazeGeneration::Register(L);
    vm()->AddCModuleToSearchers("dmlab.system.maze_generation",
                                LuaMazeGeneration::Require);
  }
};

constexpr char kCreateMaze[] = R"(
local maze_gen = require 'dmlab.system.maze_generation'
local maze = maze_gen.MazeGeneration{width = 5, height = 3}
return maze:entityLayer(), maze:variationsLayer()
)";

TEST_F(LuaMazeGenerationTest, ConstructDefaultMaze) {
  lua::PushScript(L, kCreateMaze, sizeof(kCreateMaze) - 1, "kCreateMaze");
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(2));
  std::string ent_layer, var_layer;
  EXPECT_TRUE(lua::Read(L, 1, &ent_layer));
  EXPECT_TRUE(lua::Read(L, 2, &var_layer));
  EXPECT_EQ(
      "*****\n"
      "*****\n"
      "*****\n",
      ent_layer);
  EXPECT_EQ(
      ".....\n"
      ".....\n"
      ".....\n",
      var_layer);
}

constexpr char kCreateMazeFromString[] = R"(
local maze_gen = require 'dmlab.system.maze_generation'
local entityLayer = [[
*******
*   *G*
* * * *
*P*   *
*******
]]

local variationsLayer = [[
.......
.ABC.E.
.A.C.E.
.A.CDE.
.......
]]

local maze = maze_gen.MazeGeneration{
  entity = entityLayer,
  variations = variationsLayer
}

return maze:entityLayer(), maze:variationsLayer()
)";

TEST_F(LuaMazeGenerationTest, ConstructMazeFromString) {
  lua::PushScript(L, kCreateMazeFromString, sizeof(kCreateMazeFromString) - 1,
                  "kCreateMazeFromString");
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(2));
  std::string ent_layer, var_layer;
  EXPECT_TRUE(lua::Read(L, 1, &ent_layer));
  EXPECT_TRUE(lua::Read(L, 2, &var_layer));
  EXPECT_EQ(
      "*******\n"
      "*   *G*\n"
      "* * * *\n"
      "*P*   *\n"
      "*******\n",
      ent_layer);
  EXPECT_EQ(
      ".......\n"
      ".ABC.E.\n"
      ".A.C.E.\n"
      ".A.CDE.\n"
      ".......\n",
      var_layer);
}

constexpr char kReadWriteMaze[] = R"(
local maze_gen = require 'dmlab.system.maze_generation'
local maze = maze_gen.MazeGeneration{width = 5, height = 3}

assert('*' == maze:getEntityCell(3, 3))
assert('.' == maze:getVariationsCell(3, 3))

-- Test out of bounds.
assert('' == maze:getEntityCell(6, 4))
assert('' == maze:getVariationsCell(0, 0))

-- Call with too many arguments.
assert(pcall(maze.getVariationsCell, maze, 10, 10, 10) ~= true)

-- Return modified layers.
maze:setEntityCell(1, 1, 'P')
maze:setEntityCell(2, 3, ' ')
maze:setVariationsCell(1, 1, 'A')
maze:setVariationsCell(2, 3, 'A')
return maze:entityLayer(), maze:variationsLayer()
)";

TEST_F(LuaMazeGenerationTest, ConstructReadWrite) {
  lua::PushScript(L, kReadWriteMaze, sizeof(kReadWriteMaze) - 1,
                  "kReadWriteMaze");
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(2));
  std::string ent_layer, var_layer;
  ASSERT_TRUE(lua::Read(L, 1, &ent_layer));
  ASSERT_TRUE(lua::Read(L, 2, &var_layer));
  EXPECT_EQ(
      "P****\n"
      "** **\n"
      "*****\n",
      ent_layer);
  EXPECT_EQ(
      "A....\n"
      "..A..\n"
      ".....\n",
      var_layer);
}

constexpr char kFindRooms[] = R"(
local maze_gen = require 'dmlab.system.maze_generation'
local entityLayer = [[
*********
*   *   *
*   *   *
** *** **
**     **
  ** ****
  ** ****
* ** ****
*       *
*       *
*********
]]

local maze = maze_gen.MazeGeneration{
  entity = entityLayer,
  variations = "",
}

local rooms = maze:findRooms("*")
local a = string.byte('A')
for i, room in ipairs(rooms) do
    room:visit(function(row, col)
      maze:setVariationsCell(row, col, string.char(a + i - 1))
    end)
end
return maze:variationsLayer()
)";

TEST_F(LuaMazeGenerationTest, FindRooms) {
  lua::PushScript(L, kFindRooms, sizeof(kFindRooms) - 1, "kFindRooms");
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(1));
  std::string ent_layer, var_layer;
  ASSERT_TRUE(lua::Read(L, 1, &var_layer));
  EXPECT_EQ(
      ".........\n"
      ".AAA.BBB.\n"
      ".AAA.BBB.\n"
      ".........\n"
      ".........\n"
      "CC.......\n"
      "CC.......\n"
      ".........\n"
      ".DDDDDDD.\n"
      ".DDDDDDD.\n"
      ".........\n",
      var_layer);
}

constexpr char kVisitFill[] = R"(
local maze_gen = require 'dmlab.system.maze_generation'
local entityLayer = [[
*********
*   *   *
*   *   *
** ******
*    G  *
*   *   *
*********
]]
local maze = maze_gen.MazeGeneration{entity = entityLayer}
local height, width = maze:size()
maze:visitFill{cell={5, 7}, wall="*", func=function(row, col, distance)
    -- Start must be zero
    assert(row ~= 5 or col ~= 7 or distance == 0)

    -- Must not visit top right corner.
    assert(row ~= 7 or col ~= 3)

    -- Must be within size
    assert(row <= height)
    assert(col <= width)
end}

)";

TEST_F(LuaMazeGenerationTest, kVisitFill) {
  lua::PushScript(L, kVisitFill, sizeof(kVisitFill) - 1, "kVisitFill");
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

}  // namespace

}  // namespace lab
}  // namespace deepmind
