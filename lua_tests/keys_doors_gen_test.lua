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
local gen = require 'map_generators.keys_doors.gen'

local CONF_RED_HERRING_KEY = {
    coreSequenceLength = 1,
    numTrapRooms = 0,
    addToGoalRoom = false,
    numEasyExtraRooms = 0,
    numRedHerringRooms = 0,
    numUnopenableDoors = 0,
    numUselessKeys = 1,
    numOpenableDoors = 0,
}

local CONF_SIMPLE_TRAP = {
    coreSequenceLength = 2,
    numRedHerringRoomUselessKeys = 1,
    redHerringRespectCoreOrder = true,
    probRedHerringRoomDuringCore = 1,
    numTrapRooms = 0,
    addToGoalRoom = false,
    numEasyExtraRooms = 0,
    numRedHerringRooms = 1,
    numUnopenableDoors = 0,
    numUselessKeys = 0,
    numOpenableDoors = 0,
    coreNoEarlyKeys = true,
    -- need to switch this off otherwise the red herring room might
    -- not be attached in the right place
    numMaxEmptyRooms = 0,
    startRoomGridPositions = {{1, 2}, {2, 1}, {3, 2}, {2, 3}},
}

local CONF_TRAP = {
    coreSequenceLength = 2,
    numTrapRooms = 1,
    addToGoalRoom = false,
    numEasyExtraRooms = 0,
    numRedHerringRooms = 0,
    numUnopenableDoors = 0,
    numUselessKeys = 0,
    numOpenableDoors = 0,
    startRoomGridPositions = {{1, 2}, {2, 1}, {3, 2}, {2, 3}},
}

local SEED = 156844960 -- Set the cl number as the random seed.

local tests = {}

-- Returns the list of the positions of each character in the maze.
local function charPositions(maze)
  local height, width = maze:size()
  local pos = {}
  for i = 1, height do
    for j = 1, width do
      local char = maze:getEntityCell(i, j)
      if pos[char] then
        pos[char][#pos[char] + 1] = {i, j}
      else
        pos[char] = {{i, j}}
      end
    end
  end
  return pos
end

-- Returns whether there is a path between two points in a maze.
local function hasPath(maze, from, to)
  return maze:visitRandomPath{
      seed = 0,
      from = from,
      to = to,
      wall = '*H',
      func = function() end,
  }
end

function tests.confRedHerringKey()
  local level = gen.makeLevel(CONF_RED_HERRING_KEY, SEED)
  -- When complied with libstdc++ GCC 4.9.
  local map0 = [[
***********
***********
**   I   **
**K  *G  **
*** *******
**   ******
**KP ******
***********
***********
]]
  -- When complied with libc++.
  local map1 = [[
***********
***********
**P  ******
**K  ******
*** *******
**   I   **
**  K*  G**
***********
***********
]]
  assert(level.map == map0 or level.map == map1, level.map)

  asserts.EQ(#level.keys, 2)
  asserts.EQ(#level.doors, 1)
  assert(level.doors[1] == level.keys[1] or level.doors[1] == level.keys[2],
         'Actual ' .. level.doors[1])
end

function tests.batchConfRedHerringKey()
  local num_dup = 0
  local maps = {}
  for seed = 1, 100 do
    local level = gen.makeLevel(CONF_RED_HERRING_KEY, seed)
    if maps[level.map] then
      num_dup = num_dup + 1
    else
      maps[level.map] = 1
    end
    -- Replace all 'I' with 'H' for easier counting.
    level.map = level.map:gsub('I', 'H')
    local maze = maze_generation.mazeGeneration{entity = level.map}
    local pos = charPositions(maze)
    asserts.EQ(#pos['H'], 1)
    asserts.EQ(#pos['K'], 2)
    asserts.EQ(#pos['P'], 1)
    asserts.EQ(#pos['G'], 1)
    asserts.EQ(#level.keys, 2)
    asserts.EQ(#level.doors, 1)
    asserts.NE(level.keys[1], level.keys[2])
    assert(level.keys[1] == level.doors[1] or level.keys[2] == level.doors[1])
    asserts.EQ(hasPath(maze, pos['P'][1], pos['G'][1]), false)
    asserts.EQ(hasPath(maze, pos['P'][1], pos['K'][1]), true)
    asserts.EQ(hasPath(maze, pos['P'][1], pos['K'][2]), true)
    maze:setEntityCell(pos['H'][1][1], pos['H'][1][2], ' ')
    asserts.EQ(hasPath(maze, pos['P'][1], pos['G'][1]), true)
  end
  asserts.GT(20, num_dup)
end


function tests.confSimpleTrap()
  local level = gen.makeLevel(CONF_SIMPLE_TRAP, SEED)
  -- When complied with libstdc++ GCC 4.9.
  local map0 = [[
***************
***************
**   I   I   **
**G  *KP *K  **
*******H*******
******   ******
******K  ******
***************
***************
]]
  -- When complied with libc++.
  local map1 = [[
***************
***************
**   I   I   **
**  G*KP *K  **
*******H*******
******   ******
******  K******
***************
***************
]]
  assert(level.map == map0 or level.map == map1, level.map)
  asserts.EQ(#level.keys, 3)
  asserts.EQ(#level.doors, 3)
end

function tests.batchConfSimpleTrap()
  local num_dup = 0
  local maps = {}
  for seed = 1, 100 do
    local level = gen.makeLevel(CONF_SIMPLE_TRAP, seed)
    if maps[level.map] then
      num_dup = num_dup + 1
    else
      maps[level.map] = 1
    end
    -- Replace all 'I' with 'H' for easier counting.
    level.map = level.map:gsub('I', 'H')
    local maze = maze_generation.mazeGeneration{entity = level.map}
    local pos = charPositions(maze)
    asserts.EQ(#pos['H'], 3)
    asserts.EQ(#pos['K'], 3)
    asserts.EQ(#pos['P'], 1)
    asserts.EQ(#pos['G'], 1)

    -- Find out the color of the first key.
    local first_key_color = nil
    maze:visitFill{
        cell = pos['P'][1],
        wall = '*H',
        func = function(row, col, distance)
          if maze:getEntityCell(row, col) == 'K' then
            asserts.EQ(first_key_color, nil)
            for i = 1, 3 do
              if pos['K'][i][1] == row and pos['K'][i][2] == col then
                first_key_color = level.keys[i]
              end
            end
            maze:setEntityCell(row, col, ' ')
          end
        end
    }

    -- Open all doors with the same color of the first key.
    local num_opened_doors = 0
    for i = 1, 3 do
      if level.doors[i] == first_key_color then
        maze:setEntityCell(pos['H'][i][1], pos['H'][i][2], ' ')
        num_opened_doors = num_opened_doors + 1
      end
    end
    asserts.EQ(num_opened_doors, 2)

    -- Make sure the goal is not reachable at this time.
    asserts.EQ(hasPath(maze, pos['P'][1], pos['G'][1]), false)

    -- Get the colors of the keys we can get now.
    local second_keys = {}
    maze:visitFill{
        cell = pos['P'][1],
        wall = '*H',
        func = function(row, col, distance)
          if maze:getEntityCell(row, col) == 'K' then
            for i = 1, 3 do
              if pos['K'][i][1] == row and pos['K'][i][2] == col then
                second_keys[#second_keys + 1] = level.keys[i]
              end
            end
            maze:setEntityCell(row, col, ' ')
          end
        end
    }

    asserts.EQ(#second_keys, 2)
    asserts.NE(second_keys[1], second_keys[2])

    -- Open the final door.
    for i = 1, 3 do
      if level.doors[i] == second_keys[1] or
          level.doors[i] == second_keys[2] then
        maze:setEntityCell(pos['H'][i][1], pos['H'][i][2], ' ')
        num_opened_doors = num_opened_doors + 1
      end
    end
    asserts.EQ(num_opened_doors, 3)

    -- The goal is reachable now.
    asserts.EQ(hasPath(maze, pos['P'][1], pos['G'][1]), true)
  end
  asserts.GT(20, num_dup)
end

function tests.confTrap()
  local level = gen.makeLevel(CONF_TRAP, SEED)
  -- When complied with libstdc++ GCC 4.9.
  local map0 = [[
***************
***************
**   I P *   **
**  K*   *G  **
******* ***H***
******   I   **
******K  *K  **
***************
***************
]]
  -- When complied with libc++.
  local map1 = [[
***************
***************
**   I  P*   **
**  K*K  *  G**
******* ***H***
******   I   **
******   *  K**
***************
***************
]]
  assert(level.map == map0 or level.map == map1, level.map)
  asserts.EQ(#level.keys, 3)
  asserts.EQ(#level.doors, 3)
end

function tests.batchConfTrap()
  local num_dup = 0
  local maps = {}
  for seed = 1, 100 do
    local level = gen.makeLevel(CONF_TRAP, seed)
    if maps[level.map] then
      num_dup = num_dup + 1
    else
      maps[level.map] = 1
    end
    -- Replace all 'I' with 'H' for easier counting.
    level.map = level.map:gsub('I', 'H')
    local maze = maze_generation.mazeGeneration{entity = level.map}
    local pos = charPositions(maze)
    asserts.EQ(#pos['H'], 3)
    asserts.EQ(#pos['K'], 3)
    asserts.EQ(#pos['P'], 1)
    asserts.EQ(#pos['G'], 1)


    local key_colors = {}
    for i = 1, 3 do
      if key_colors[level.keys[i]] then
        key_colors[level.keys[i]] = key_colors[level.keys[i]] + 1
      else
        key_colors[level.keys[i]] = 1
      end
    end

    local door_colors = {}
    for i = 1, 3 do
      if door_colors[level.doors[i]] then
        door_colors[level.doors[i]] = door_colors[level.doors[i]] + 1
      else
        door_colors[level.doors[i]] = 1
      end
    end

    -- There should be two keys with the same color and one with another color.
    local first_key_color = nil
    local second_key_color = nil
    for color, count in pairs(door_colors) do
      if count == 2 then
        first_key_color = color
      elseif count == 1 then
        second_key_color = color
      end
    end
    assert(first_key_color)
    assert(second_key_color)

    local first_key_id = nil
    for i = 1, 3 do
      if level.keys[i] == first_key_color then
        assert(first_key_id == nil)
        first_key_id = i
      else
        asserts.EQ(level.keys[i], second_key_color)
      end
    end

    -- There should be exactly one door with first_key_color.
    local last_door_id = nil
    for i = 1, 3 do
      if level.doors[i] == second_key_color then
        assert(last_door_id == nil)
        last_door_id = i
      else
        asserts.EQ(level.doors[i], first_key_color)
      end
    end

    -- Should be able to reach the first key.
    asserts.EQ(hasPath(maze, pos['P'][1], pos['K'][first_key_id]), true)

    local num_solutions = 0

    for i = 1, 3 do
      -- There are 2 doors which can be opened with the first key, among which
      -- only one is the correct door.
      if level.doors[i] == first_key_color then
        -- Open the first door.
        maze:setEntityCell(pos['H'][i][1], pos['H'][i][2], ' ')
        -- Should be able to reach the door.
        asserts.EQ(hasPath(maze, pos['P'][1], pos['H'][i]), true)
        -- Should not be able to reach the goal at this time.
        asserts.EQ(hasPath(maze, pos['P'][1], pos['G'][1]), false)
        local succeed_to_get_second_key = false
        for j = 1, 3 do
          -- Should be able to reach a new key by opening this door.
          if level.keys[j] == second_key_color then
            if hasPath(maze, pos['H'][i], pos['K'][j]) then
              succeed_to_get_second_key = true
              -- Open the second door.
              maze:setEntityCell(pos['H'][last_door_id][1],
                  pos['H'][last_door_id][2], ' ')
              -- Test if we can reach the goal.
              if hasPath(maze, pos['K'][j], pos['G'][1]) then
                num_solutions = num_solutions + 1
              end
              -- Reset the second door.
              maze:setEntityCell(pos['H'][last_door_id][1],
                  pos['H'][last_door_id][2], 'H')
            end
          end
        end
        asserts.EQ(succeed_to_get_second_key, true)
        -- Reset the first door.
        maze:setEntityCell(pos['H'][i][1], pos['H'][i][2], 'H')
      end
    end
    asserts.EQ(num_solutions, 1)
  end
  asserts.GT(5, num_dup)
end

return test_runner.run(tests)
