--[[ Copyright (C) 2017-2019 Google Inc.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
]]

local asserts = require 'testing.asserts'
local test_runner = require 'testing.test_runner'
local maze_generation = require 'dmlab.system.maze_generation'
local gen = require 'map_generators.platforms_v1.gen'
local random = require 'common.random'
local map_maker = require 'dmlab.system.map_maker'
local randomMap = random(map_maker:randomGen())

local LEVEL_CONFIGS = {
    easy = {
        difficulty = 0.1,
        expectedSize = 11,
    },
    medium = {
        difficulty = 0.5,
        expectedSize = 15,
    },
    hard = {
        difficulty = 0.9,
        expectedSize = 19,
    },
    random = {
        difficulty = nil,
        expectedSize = nil,
    }
}

local tests = {}

--[[ Check whether in the map, there is a path between (px, py) and (gx, gy)
that the height of platforms in the path is non-ascending.
Because the algorithm is very platforms-level-specific, I didn't find an easy
way to implement this with existing utilities. Instead, I wrote a flood fill by
myself, and added a unit test for it below. ]]
local function hasPath(maze, px, py, gx, gy)
  if maze:getEntityCell(px, py) == '.' or
      maze:getEntityCell(gx, gy) == '.' then
    return false
  end
  -- Use the variations layer of the maze to mark whether a cell has been
  -- reached through flood fill. '.' stands for unreached, and '*' stands for
  -- reached.
  local height, width = maze:size()
  for i = 1, height do
    for j = 1, width do
      maze:setVariationsCell(i, j, '.')
    end
  end
  maze:setVariationsCell(px, py, '*')

  local dirs = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}}
  local queue = {{px, py}}
  local parent = {0}
  local r = 1
  local f = 1
  while r <= f do
    local rx, ry = unpack(queue[r])
    -- The character stands for the height of cell (rx, ry)
    local rh = maze:getEntityCell(rx, ry)
    for _, dir in ipairs(dirs) do
      local fx, fy = rx + dir[1], ry + dir[2]
      -- Check whether the cell (fx, fy) is unreached and contains a platform.
      if maze:getVariationsCell(fx, fy) == '.' then
        -- The character stands for the height of cell (fx, fy)
        local fh = maze:getEntityCell(fx, fy)
        -- Check whether is step is non-ascending.
        if fh ~= '.' and string.byte(fh) >= string.byte(rh) then
          maze:setVariationsCell(fx, fy, '*')
          f = f + 1
          queue[f] = {fx, fy}
          parent[f] = r
          if fx == gx and fy == gy then
            while f ~= 0 do
              maze:setVariationsCell(queue[f][1], queue[f][2], '#')
              f = parent[f]
            end
            return true
          end
        end
      end
    end
    r = r + 1
  end
  return false
end

-- Add a test for the testing helper function.
function tests.testHasPath()
  local maze = maze_generation.mazeGeneration{
      entity = [[
.....
.aaa.
.bcd.
.cdc.
.....
]]}
  asserts.EQ(hasPath(maze, 2, 2, 2, 4), true)
  asserts.EQ(hasPath(maze, 2, 4, 2, 2), true)
  asserts.EQ(hasPath(maze, 2, 2, 3, 4), true)
  asserts.EQ(hasPath(maze, 3, 4, 2, 2), false)
  asserts.EQ(hasPath(maze, 2, 2, 4, 4), false)
  asserts.EQ(hasPath(maze, 4, 4, 2, 2), false)
end

local function testLevel(config)
  for seed = 1, 500 do
    randomMap:seed(seed)
    local level = gen.makeLevel(config.difficulty, randomMap)
    local maze = maze_generation.mazeGeneration{entity = level.map}
    local height, width = maze:size()
    if config.expectedSize then
      asserts.EQ(height, config.expectedSize)
      asserts.EQ(width, config.expectedSize)
    end
    local px, py = level.playerPosition.x + 1, level.playerPosition.y + 1
    local gx, gy = level.goalPosition.x + 1, level.goalPosition.y + 1
    asserts.EQ(maze:getEntityCell(px, py), 'a')
    asserts.EQ(hasPath(maze, px, py, gx, gy), true)
    local main_path = {}
    for i = 1, height do
      for j = 1, width do
        if maze:getVariationsCell(i, j) == '#' then
          main_path[#main_path + 1] = {i, j}
        end
      end
    end
    -- Verify there is only one path from start to goal.
    for _, cell in ipairs(main_path) do
      local x, y = unpack(cell)
      local ch = maze:getEntityCell(x, y)
      maze:setEntityCell(x, y, '.')
      asserts.EQ(hasPath(maze, px, py, gx, gy), false)
      maze:setEntityCell(x, y, ch)
    end
  end

  -- Same seed twice
  for seed = 1, 50 do
    randomMap:seed(seed)
    random:seed(seed)
    local level1 = gen.makeLevel(config.difficulty, randomMap)
    local level2 = gen.makeLevel(config.difficulty, random)
    asserts.tablesEQ(level1, level2)
  end
end

-- Separate tests for three difficulties instead of looping through them in one
-- test to get better stacktrace.

function tests.platformsEasy()
  testLevel(LEVEL_CONFIGS.easy)
end

function tests.platformsMedium()
  testLevel(LEVEL_CONFIGS.medium)
end

function tests.platformsHard()
  testLevel(LEVEL_CONFIGS.hard)
end

function tests.platformsRandom()
  testLevel(LEVEL_CONFIGS.random)
end

return test_runner.run(tests)
