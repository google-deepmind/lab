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

#include <cstring>
#include <random>
#include <string>

#include "absl/log/log.h"
#include "deepmind/engine/lua_random.h"
#include "deepmind/lua/bind.h"
#include "deepmind/lua/call.h"
#include "deepmind/lua/n_results_or_test_util.h"
#include "deepmind/lua/push_script.h"
#include "deepmind/lua/table_ref.h"
#include "deepmind/lua/vm_test_util.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace deepmind {
namespace lab {
namespace {

using ::deepmind::lab::lua::testing::IsOkAndHolds;
using ::testing::Eq;

class LuaMazeGenerationTest : public lua::testing::TestWithVm {
 protected:
  LuaMazeGenerationTest() {
    LuaMazeGeneration::Register(L);
    vm()->AddCModuleToSearchers(
        "dmlab.system.maze_generation", &lua::Bind<LuaMazeGeneration::Require>,
        {reinterpret_cast<void*>(static_cast<std::uintptr_t>(0))});
    vm()->AddCModuleToSearchers(
        "dmlab.system.maze_generation1", &lua::Bind<LuaMazeGeneration::Require>,
        {reinterpret_cast<void*>(static_cast<std::uintptr_t>(1))});
    LuaRandom::Register(L);
    vm()->AddCModuleToSearchers(
        "dmlab.system.sys_random", &lua::Bind<LuaRandom::Require>,
        {&prbg_[0], reinterpret_cast<void*>(static_cast<std::uintptr_t>(0))});
    vm()->AddCModuleToSearchers(
        "dmlab.system.sys_random1", &lua::Bind<LuaRandom::Require>,
        {&prbg_[1], reinterpret_cast<void*>(static_cast<std::uintptr_t>(1))});
  }

  std::mt19937_64 prbg_[2];
};

constexpr char kCreateMaze[] = R"(
local maze_generation = require 'dmlab.system.maze_generation'
local maze = maze_generation.mazeGeneration{width = 5, height = 3}
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
local maze_generation = require 'dmlab.system.maze_generation'
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

local maze = maze_generation.mazeGeneration{
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
local maze_generation = require 'dmlab.system.maze_generation'
local maze = maze_generation.mazeGeneration{width = 5, height = 3}

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
local maze_generation = require 'dmlab.system.maze_generation'
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

local maze = maze_generation.mazeGeneration{
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
local maze_generation = require 'dmlab.system.maze_generation'
local entityLayer = [[
*********
*   *   *
*   *   *
** ******
*    G  *
*   *   *
*********
]]
local maze = maze_generation.mazeGeneration{entity = entityLayer}
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

constexpr char kCreateRandomMaze0[] = R"(
local sys_random = require 'dmlab.system.sys_random'
local maze_generation = require 'dmlab.system.maze_generation'
sys_random:seed(0)
local maze = maze_generation.randomMazeGeneration{
    random = sys_random,
    width = 15,
    height = 13,
    maxRooms = 3,
    extraConnectionProbability = 0.0,
    simplify = false,
}

return maze:entityLayer(), maze:variationsLayer()
)";

TEST_F(LuaMazeGenerationTest, CreateRandomMaze0) {
  lua::PushScript(L, kCreateRandomMaze0, "kCreateRandomMaze0");
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(2));
  std::string ent_layer, var_layer;
  ASSERT_TRUE(lua::Read(L, 1, &ent_layer));
  ASSERT_TRUE(lua::Read(L, 2, &var_layer));
  constexpr const char kExpectedEntityLayer0[] =
      "***************\n"
      "*             *\n"
      "***** ***** * *\n"
      "*     *     * *\n"
      "***** *     * *\n"
      "*     *     * *\n"
      "*     *     * *\n"
      "*           * *\n"
      "*     * *** * *\n"
      "*       * *   *\n"
      "******* * *   *\n"
      "*             *\n"
      "***************\n";

  constexpr const char kExpectedEntityLayer1[] =
      "***************\n"
      "*   *   *     *\n"
      "*   *** * *   *\n"
      "*         *   *\n"
      "*   *** *** ***\n"
      "*         * * *\n"
      "*** *     * * *\n"
      "*   *     * * *\n"
      "* * * ***** * *\n"
      "* *   *     * *\n"
      "* ***** ***** *\n"
      "*             *\n"
      "***************\n";

  EXPECT_THAT(ent_layer, testing::AnyOf(Eq(kExpectedEntityLayer0),
                                        Eq(kExpectedEntityLayer1)));
  constexpr const char kExpectedVariationsLayer0[] =
      "...............\n"
      "...............\n"
      "...............\n"
      ".......AAAAA...\n"
      ".......AAAAA...\n"
      ".BBBBB.AAAAA...\n"
      ".BBBBB.AAAAA...\n"
      ".BBBBB.AAAAA...\n"
      ".BBBBB.........\n"
      ".BBBBB.....CCC.\n"
      "...........CCC.\n"
      "...........CCC.\n"
      "...............\n";

  constexpr const char kExpectedVariationsLayer1[] =
      "...............\n"
      ".BBB.......AAA.\n"
      ".BBB.......AAA.\n"
      ".BBB.......AAA.\n"
      ".BBB...........\n"
      ".BBB.CCCCC.....\n"
      ".....CCCCC.....\n"
      ".....CCCCC.....\n"
      "...............\n"
      "...............\n"
      "...............\n"
      "...............\n"
      "...............\n";
  EXPECT_THAT(var_layer, testing::AnyOf(Eq(kExpectedVariationsLayer0),
                                        Eq(kExpectedVariationsLayer1)));
}

constexpr char kCreateRandomMaze1[] = R"(
local sys_random = require 'dmlab.system.sys_random'
local maze_generation = require 'dmlab.system.maze_generation'
sys_random:seed(0)
local maze = maze_generation.randomMazeGeneration{
    random = sys_random,
    width = 15,
    height = 13,
    maxRooms = 3,
    extraConnectionProbability = 0.0,
    object = 'A',
    spawn = '2',
    hasDoors = true,
    roomSpawnCount = 1,
    roomObjectCount = 1,
}

return maze:entityLayer(), maze:variationsLayer()
)";

TEST_F(LuaMazeGenerationTest, CreateRandomMaze1) {
  lua::PushScript(L, kCreateRandomMaze1, "kCreateRandomMaze1");
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(2));
  std::string ent_layer;
  ASSERT_TRUE(lua::Read(L, 1, &ent_layer));
  constexpr const char kExpectedEntityLayer0[] =
      "***************\n"
      "*****         *\n"
      "***** *****H* *\n"
      "***** *   2 * *\n"
      "*****H*     * *\n"
      "*    2*   A * *\n"
      "*     *     * *\n"
      "*   A I     * *\n"
      "*     *H***H*H*\n"
      "*     I ***   *\n"
      "******* ***   *\n"
      "*******   I 2A*\n"
      "***************\n";

  constexpr const char kExpectedEntityLayer1[] =
      "***************\n"
      "*A  ***** IA  *\n"
      "* 2 ***** *2  *\n"
      "*   I     *   *\n"
      "*   ***H***H***\n"
      "*   I 2   * ***\n"
      "***H*    A* ***\n"
      "*   *     * ***\n"
      "* * *H***** ***\n"
      "* *   *     ***\n"
      "* ***** *******\n"
      "*       *******\n"
      "***************\n";

  EXPECT_THAT(ent_layer, testing::AnyOf(Eq(kExpectedEntityLayer0),
                                        Eq(kExpectedEntityLayer1)));
}

constexpr char kCreateRandomMaze2[] = R"(
local sys_random = require 'dmlab.system.sys_random'
local maze_generation = require 'dmlab.system.maze_generation'
sys_random:seed(123)
local maze = maze_generation.randomMazeGeneration{
    random = sys_random,
    width = 9,
    height = 9,
    roomMinSize = 7,
    roomMaxSize = 7,
    maxRooms = 1,
    extraConnectionProbability = 0.0,
    roomSpawnCount = 0,
    roomObjectCount = 0,
}

return maze:entityLayer(), maze:variationsLayer()
)";

TEST_F(LuaMazeGenerationTest, CreateRandomMaze2) {
  lua::PushScript(L, kCreateRandomMaze2, sizeof(kCreateRandomMaze2) - 1,
                  "kCreateRandomMaze2");
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(2));
  std::string ent_layer, var_layer;
  ASSERT_TRUE(lua::Read(L, 1, &ent_layer));
  ASSERT_TRUE(lua::Read(L, 2, &var_layer));
  EXPECT_EQ(
      "*********\n"
      "*       *\n"
      "*       *\n"
      "*       *\n"
      "*       *\n"
      "*       *\n"
      "*       *\n"
      "*       *\n"
      "*********\n",
      ent_layer);
  EXPECT_EQ(
      ".........\n"
      ".AAAAAAA.\n"
      ".AAAAAAA.\n"
      ".AAAAAAA.\n"
      ".AAAAAAA.\n"
      ".AAAAAAA.\n"
      ".AAAAAAA.\n"
      ".AAAAAAA.\n"
      ".........\n",
      var_layer);
}

constexpr char kCreateRandomMazeEvenSize[] = R"(
local sys_random = require 'dmlab.system.sys_random'
local maze_generation = require 'dmlab.system.maze_generation'
sys_random:seed(0)
local maze = maze_generation.randomMazeGeneration{
    random = sys_random,
    roomMinSize = 4
}

return maze:entityLayer(), maze:variationsLayer()
)";

TEST_F(LuaMazeGenerationTest, CreateRandomMazeEvenSize) {
  lua::PushScript(L, kCreateRandomMazeEvenSize,
                  sizeof(kCreateRandomMazeEvenSize) - 1,
                  "kCreateRandomMazeEvenSize");
  ASSERT_FALSE(lua::Call(L, 0).ok());
}

constexpr char kCreateRandomMazeMaxVariations[] = R"(
local sys_random = require 'dmlab.system.sys_random'
local maze_generation = require 'dmlab.system.maze_generation'
sys_random:seed(0)
local maze = maze_generation.randomMazeGeneration{
    random = sys_random,
    roomMinSize = 27
}

return maze:entityLayer(), maze:variationsLayer()
)";

TEST_F(LuaMazeGenerationTest, CreateRandomMazeMaxVariations) {
  lua::PushScript(L, kCreateRandomMazeMaxVariations,
                  sizeof(kCreateRandomMazeMaxVariations) - 1,
                  "kCreateRandomMazeMaxVariations");
  ASSERT_FALSE(lua::Call(L, 0).ok());
}

constexpr char kCreateRandomMazeNoRandom[] = R"(
local maze_generation = require 'dmlab.system.maze_generation'
local maze = maze_generation.randomMazeGeneration{
    width = 15,
    height = 13,
    maxRooms = 3,
    extraConnectionProbability = 0.0,
    simplify = false,
}

return maze:entityLayer(), maze:variationsLayer()
)";

TEST_F(LuaMazeGenerationTest, CreateRandomMazeNoRandom) {
  lua::PushScript(L, kCreateRandomMazeNoRandom,
                  sizeof(kCreateRandomMazeNoRandom) - 1,
                  "kCreateRandomMazeNoRandom");
  ASSERT_FALSE(lua::Call(L, 0).ok());
}

constexpr int kRandomMazeSeedCount = 10000;
constexpr char kCreateRandomMazes[] = R"(
local seed, maps, vers = ...
local sys_random = require('dmlab.system.sys_random' .. vers)
local maze_generation = require('dmlab.system.maze_generation' .. vers)
sys_random:seed(seed)
local maze = maze_generation.randomMazeGeneration{
      random = sys_random,
      width = 17,
      height = 17,
      extraConnectionProbability = 0.0,
      hasDoors = false,
      roomCount = 4,
      maxRooms = 4,
      roomMaxSize = 5,
      roomMinSize = 3
}
local key = maze:entityLayer()
local count = maps[key] or 0
maps[key] = count + 1
)";

TEST_F(LuaMazeGenerationTest, CreateRandomMazes) {
  auto maps = lua::TableRef::Create(L);
  lua::PushScript(L, kCreateRandomMazes, sizeof(kCreateRandomMazes) - 1,
                  "kCreateRandomMazes");
  for (int i = 0; i < kRandomMazeSeedCount; ++i) {
    LOG(INFO) << "Phase 1: step " << i + 1 << " out of "
              << kRandomMazeSeedCount;
    lua_pushvalue(L, -1);
    lua::Push(L, i);
    lua::Push(L, maps);
    lua::Push(L, "");
    ASSERT_THAT(lua::Call(L, 3), IsOkAndHolds(0));
  }
  const auto mixer_seed_0_maps = maps.KeyCount();
  const auto mixer_seed_0_repeat_ratio =
      (kRandomMazeSeedCount - mixer_seed_0_maps) /
      static_cast<float>(kRandomMazeSeedCount);
  ASSERT_GE(mixer_seed_0_repeat_ratio, 0.0);
  ASSERT_LE(mixer_seed_0_repeat_ratio, 0.01);
  for (int i = 0; i < kRandomMazeSeedCount; ++i) {
    LOG(INFO) << "Phase 2: step " << i + 1 << " out of "
              << kRandomMazeSeedCount;
    lua_pushvalue(L, -1);
    lua::Push(L, i);
    lua::Push(L, maps);
    lua::Push(L, "1");
    ASSERT_THAT(lua::Call(L, 3), IsOkAndHolds(0));
  }
  const auto mixer_seed_1_maps = maps.KeyCount() - mixer_seed_0_maps;
  const auto mixer_seed_1_repeat_ratio =
      (kRandomMazeSeedCount - mixer_seed_1_maps) /
      static_cast<float>(kRandomMazeSeedCount);
  ASSERT_GE(mixer_seed_1_repeat_ratio, 0.0);
  ASSERT_LE(mixer_seed_1_repeat_ratio, 0.01);
  lua_pop(L, 1);
}

constexpr char kVisitRandomPath[] = R"(
local sys_random = require 'dmlab.system.sys_random'
local maze_generation = require 'dmlab.system.maze_generation'
local seed = ...
local entityLayer = [[
***************
*P            *
***** ***** * *
*     *     * *
***** *     * *
*     *     * *
*     *     * *
*           * *
*     * *** * *
*       *G*   *
******* * *   *
*             *
***************
]]

local maze = maze_generation.mazeGeneration{
  entity = entityLayer
}

local oi = 2
local oj = 2
local startVisited = false
local goalVisited = false

assert(maze:getEntityCell(oi, oj) == 'P', 'Misplaced start')
assert(maze:getEntityCell(10, 10) == 'G', 'Misplaced goal')
sys_random:seed(seed)
maze:visitRandomPath{
  random = sys_random,
  from = {2, 2},
  to = {10, 10},
  func = function(i, j)
    local c = maze:getEntityCell(i, j)
    if c == 'P' then
      assert(not startVisited, 'Start already visited')
      startVisited = true
    elseif c == 'G' then
      assert(not goalVisited, 'Goal already visited')
      goalVisited = true
    elseif c == '*' then
      error('Hit wall!')
    elseif c == ' ' then
      assert(startVisited, 'Start not visited yet')
      assert(not goalVisited, 'Goal already visited')
      assert(oi ~= i or oj ~= j, 'Movement not continuous')
    else
      error('Unknown error!')
    end
    assert(oi == i or oj == j, 'Movement not continuous')
    assert(oi + 1 == i or oi == i or oi == i + 1, 'Movement not continuous')
    assert(oj + 1 == j or oj == j or oj == j + 1, 'Movement not continuous')
    oi, oj = i, j
  end
}
assert(startVisited, 'Start not visited')
assert(goalVisited, 'Goal not visited')
)";


TEST_F(LuaMazeGenerationTest, kVisitRandomPath) {
  lua::PushScript(L, kVisitRandomPath, std::strlen(kVisitRandomPath),
                  "kVisitRandomPath");
  for (int seed = 1; seed < 100; ++seed) {
    lua_pushvalue(L, -1);
    lua::Push(L, seed);
    EXPECT_THAT(lua::Call(L, 1), IsOkAndHolds(0)) << " with seed " << seed;
  }
  lua_pop(L, 1);
}

constexpr char kCountEntities[] = R"(
local maze_generation = require 'dmlab.system.maze_generation'
local entityLayer = [[
*****
ABCDE
ABCD*
ABC**
AB***
A****
*****
]]
local variationsLayer = [[
.....
ABCDE
ABCD.
ABC..
AB...
A....
.....
]]

local maze = maze_generation.mazeGeneration{
  entity = entityLayer,
  variations = variationsLayer,
}

assert(maze:countEntities('A') == 5, 'Incorrect A count')
assert(maze:countEntities('B') == 4, 'Incorrect B count')
assert(maze:countEntities('C') == 3, 'Incorrect C count')
assert(maze:countEntities('D') == 2, 'Incorrect D count')
assert(maze:countEntities('E') == 1, 'Incorrect E count')
assert(maze:countEntities('ABCDE') == 15, 'Incorrect letter count')
assert(maze:countEntities('*') == 20, 'Incorrect wall count')

assert(maze:countVariations('A') == 5, 'Incorrect A count')
assert(maze:countVariations('B') == 4, 'Incorrect B count')
assert(maze:countVariations('C') == 3, 'Incorrect C count')
assert(maze:countVariations('D') == 2, 'Incorrect D count')
assert(maze:countVariations('E') == 1, 'Incorrect E count')
assert(maze:countVariations('ABCDE') == 15, 'Incorrect letter count')
assert(maze:countVariations('.') == 20, 'Incorrect dot count')
local status, msg = pcall(maze.countEntities, maze, 10)
assert(status == false, 'Failed to raise error with invalid input.')
assert(msg:match('string'), 'Message does not mention type: ' .. msg)
)";

TEST_F(LuaMazeGenerationTest, kCountEntities) {
  lua::PushScript(L, kCountEntities, std::strlen(kCountEntities),
                  "kCountEntities");
  EXPECT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

}  // namespace

}  // namespace lab
}  // namespace deepmind
