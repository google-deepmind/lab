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
local Room = require 'map_generators.keys_doors.room'
local RoomGrid = require 'map_generators.keys_doors.room_grid'

local tests = {}

local function roomGridConfig()
  return {
      roomGridHeight = 2,
      roomGridWidth = 3,
  }
end

function tests.testCreateRoomGrid()
  local roomGrid = RoomGrid.new(roomGridConfig())
  asserts.EQ(next(roomGrid.rooms), nil)
  asserts.EQ(roomGrid.height, 2)
  asserts.EQ(roomGrid.width, 3)
end

function tests.testCreateRoom()
  local roomGrid = RoomGrid.new(roomGridConfig())
  local room1 = roomGrid:createRoom{grid = {2, 1}}
  asserts.EQ(room1.id, 1)
  asserts.EQ(roomGrid.rooms[1], room1)
  asserts.EQ(roomGrid:grid(2, 1), room1)

  local room2 = roomGrid:createRoom{grid = {1, 3}}
  asserts.EQ(room2.id, 2)
  asserts.EQ(roomGrid.rooms[2], room2)
  asserts.EQ(roomGrid:grid(1, 3), room2)
end

--[[ The grid map looks like this in the following 2 tests:
...
.PQ
where P is room1, Q is room2. ]]
function tests.testGetRandomFreeGridPosCompact()
  local config = roomGridConfig()
  config.compact = true
  local roomGrid = RoomGrid.new(config)
  local room1 = roomGrid:createRoom{grid = {2, 2}}
  local room2 = roomGrid:createRoom{grid = {2, 3}}
  local position = roomGrid:getRandomFreeGridPos(room1, {1, 1})

  asserts.EQ(position[1], 2)
  asserts.EQ(position[2], 1)
end

function tests.testGetAllFreeGridPos()
  local roomGrid = RoomGrid.new(roomGridConfig())
  local room1 = roomGrid:createRoom{grid = {2, 2}}
  local room2 = roomGrid:createRoom{grid = {2, 3}}
  local positions = roomGrid:getAllFreeGridPos(room1)

  asserts.EQ(#positions, 2)
  asserts.EQ(positions[1][1], 2)
  asserts.EQ(positions[1][2], 1)
  asserts.EQ(positions[2][1], 1)
  asserts.EQ(positions[2][2], 2)
end

function tests.testFindClosestPositionsToStart()
  local roomGrid = RoomGrid.new{
      roomGridHeight = 3,
      roomGridWidth = 3,
  }
  local pos1, indices1 = roomGrid:findClosestPositionsToStart(
      {{1, 2}, {2, 1}, {2, 3}}, {2, 2})
  asserts.EQ(#pos1, 1)
  asserts.EQ(pos1[1][1], 1)
  asserts.EQ(pos1[1][2], 2)
  asserts.EQ(#indices1, 1)
  asserts.EQ(indices1[1], 1)

  local pos2, indices2 = roomGrid:findClosestPositionsToStart(
      {{1, 2}, {2, 1}, {2, 3}}, {3, 2})
  asserts.EQ(#pos2, 2)
  asserts.EQ(pos2[1][1], 2)
  asserts.EQ(pos2[1][2], 1)
  asserts.EQ(pos2[2][1], 2)
  asserts.EQ(pos2[2][2], 3)
  asserts.EQ(#indices2, 2)
  asserts.EQ(indices2[1], 2)
  asserts.EQ(indices2[2], 3)
end

function tests.testFindRoomWithObjectSpace()
  local roomGrid = RoomGrid.new(roomGridConfig())
  local room1 = roomGrid:createRoom{grid = {1, 1}}
  local room2 = roomGrid:createRoom{grid = {1, 2}}
  room1:addObject('key')
  room1:addObject('key')
  asserts.EQ(roomGrid:findRoomWithObjectSpace(), room2)
  room2:addObject('goal')
  asserts.EQ(roomGrid:findRoomWithObjectSpace(), room2)
  local room3 = roomGrid:createRoom{grid = {1, 3}}
  asserts.EQ(roomGrid:findRoomWithObjectSpace({}, {3}), room3)
  asserts.EQ(roomGrid:findRoomWithObjectSpace({2}), room3)
  asserts.EQ(roomGrid:findRoomWithObjectSpace({3}), room2)
end

function tests.testFindRoomWithGridSpace()
  local roomGrid = RoomGrid.new(roomGridConfig())
  local room1 = roomGrid:createRoom{grid = {1, 1}}
  local room2 = roomGrid:createRoom{grid = {1, 2}}
  local room3 = roomGrid:createRoom{grid = {1, 3}}
  local room, pos = roomGrid:findRoomWithGridSpace(nil, {2, 3})
  asserts.EQ(room, room1)
  asserts.EQ(pos[1], 2)
  asserts.EQ(pos[2], 1)
end

function tests.testFindRoomWithGridSpaceWhenCompact()
  local config = roomGridConfig()
  config.compact = true
  local roomGrid = RoomGrid.new(config)
  local room1 = roomGrid:createRoom{grid = {1, 1}}
  local room2 = roomGrid:createRoom{grid = {1, 2}}
  local room3 = roomGrid:createRoom{grid = {1, 3}}
  local room, pos = roomGrid:findRoomWithGridSpace({1, 3})
  asserts.EQ(room, room3)
  asserts.EQ(pos[1], 2)
  asserts.EQ(pos[2], 3)
end

return test_runner.run(tests)
