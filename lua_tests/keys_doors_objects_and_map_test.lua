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
local api = require 'map_generators.keys_doors.objects_and_map'
local RoomGrid = require 'map_generators.keys_doors.room_grid'

local tests = {}

function tests.testCreateObjects()
  local objects, objectCodes = api.createObjects({'red', 'green'})
  asserts.EQ(#objects, 10)
  asserts.EQ(objects[1], 'redKey')
  asserts.EQ(objects[2], 'redVertDoor')
  asserts.EQ(objects[3], 'redHorizDoor')
  asserts.EQ(objects[4], 'greenKey')
  asserts.EQ(objects[5], 'greenVertDoor')
  asserts.EQ(objects[6], 'greenHorizDoor')
  asserts.EQ(objects[7], 'spawn')
  asserts.EQ(objects[8], 'goal')
  asserts.EQ(objects[9], 'wall')
  asserts.EQ(objects[10], 'emptyFloor')

  for i, object in ipairs(objects) do
    asserts.EQ(objectCodes[object], i)
  end
end

function tests.testMakeMap()
  local expectedMap = [[
***************
***************
******   ******
******K K******
*******H*******
**   I   I   **
**   *K K*   **
***************
***************
]]
  local expectedKeys = {'blue', 'blue', 'red', 'red'}
  local expectedDoors = {'green', 'red', 'blue'}

  local objects, objectCodes = api.createObjects({'red', 'green', 'blue'})
  local roomGrid = RoomGrid.new{
      roomGridHeight = 3,
      roomGridWidth = 3,
  }
  local room1 = roomGrid:createRoom{grid = {2, 2}}
  local room2 = roomGrid:createRoom{
      grid = {2, 1},
      entryRoom = room1,
      entryDoorColor = 'red',
  }
  local room3 = roomGrid:createRoom{
      grid = {1, 2},
      entryRoom = room1,
      entryDoorColor = 'green',
  }
  local room4 = roomGrid:createRoom{
      grid = {2, 3},
      entryRoom = room1,
      entryDoorColor = 'blue',
  }
  room1:addObject('redKey')
  room1:addObject('redKey')
  room3:addObject('blueKey')
  room3:addObject('blueKey')

  local level = api.makeLevel(roomGrid, objects, objectCodes)
  asserts.EQ(level.map, expectedMap)
  asserts.EQ(#level.keys, #expectedKeys)
  for i = 1, #level.keys do
    asserts.EQ(level.keys[i], expectedKeys[i])
  end
  asserts.EQ(#level.doors, #expectedDoors)
  for i = 1, #level.doors do
    asserts.EQ(level.doors[i], expectedDoors[i])
  end
end

return test_runner.run(tests)
