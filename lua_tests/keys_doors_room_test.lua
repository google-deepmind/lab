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

local tests = {}

function tests.testCreateAndConnectTwoRooms()
  local room1 = Room.new{
      id = 1,
      grid = {1, 1},
  }
  asserts.EQ(room1.id, 1)
  local gridi, gridj = room1:gridPosition()
  asserts.EQ(gridi, 1)
  asserts.EQ(gridj, 1)
  local room2 = Room.new{
      id = 2,
      grid = {1, 2},
      entryRoom = room1,
      entryDoorColor = 'red',
  }
  asserts.EQ(room2.id, 2)
  gridi, gridj = room2:gridPosition()
  asserts.EQ(gridi, 1)
  asserts.EQ(gridj, 2)
  asserts.EQ(room1:connectedRoom(1, 2), room2)
  asserts.EQ(room1:connectedDoor(1, 2), 'red')
end

function tests.testAddObject()
  local room = Room.new{
      id = 1,
      grid = {1, 1},
  }
  asserts.EQ(room:addObject('key'), true)
  asserts.EQ(room:addObject('key'), true)
  asserts.EQ(room:addObject('key'), false)
end

--[[ The final map after drawing 5 rooms should be like:

```
*************
*****   *****
*****   *****
******H******
*   Ip  I   *
*   *  k*   *
****** ******
*****   *****
*****   *****
*************
```

The position of 'p' and 'k' may change within the center room because of the
randomness. ]]
function tests.testDrawFiveRooms()
  local rooms = {}
  rooms[1] = Room.new{
      id = 1,
      grid = {2, 2},
  }
  rooms[1]:addObject('key')
  rooms[1]:addPlayerSpawn()
  rooms[2] = Room.new{
      id = 2,
      grid = {1, 2},
      entryRoom = rooms[1],
      entryDoorColor = 'red',
  }
  rooms[3] = Room.new{
      id = 3,
      grid = {2, 1},
      entryRoom = rooms[1],
      entryDoorColor = 'green',
  }
  rooms[4] = Room.new{
      id = 4,
      grid = {2, 3},
      entryRoom = rooms[1],
      entryDoorColor = 'blue',
  }
  rooms[5] = Room.new{
      id = 2,
      grid = {3, 2},
      entryRoom = rooms[1],
  }
  local code = {
    wall = '*',
    emptyFloor = ' ',
    spawn = 'p',
    key = 'k',
    redHorizDoor = 'H',
    greenHorizDoor = 'H',
    blueHorizDoor = 'H',
    redVertDoor = 'I',
    greenVertDoor = 'I',
    blueVertDoor = 'I',
  }
  local map = {}
  for i = 1, 10 do
    map[i] = {}
    for j = 1, 13 do
      map[i][j] = code.wall
    end
  end
  for i = 1, 5 do
    local gridi, gridj = rooms[i]:gridPosition()
    rooms[i]:draw(map, gridi * 3 - 1, gridj * 4 - 2, code)
  end

  -- Check room 1 object
  assert(map[6][6] == code.key or map[6][8] == code.key)

  -- Check room 1 player
  local positions = {{5, 6}, {5, 7}, {5, 8}, {6, 7}}
  local playerCount = 0
  for _, position in ipairs(positions) do
    if map[position[1]][position[2]] == code.spawn then
      playerCount = playerCount + 1
    end
  end
  asserts.EQ(playerCount, 1)

  -- Check doors
  asserts.EQ(map[4][7], 'H')
  asserts.EQ(map[5][5], 'I')
  asserts.EQ(map[5][9], 'I')
  -- This door has no color so it will be drawn as empty space.
  asserts.EQ(map[7][7], ' ')

  -- Check empty spaces and walls
  local count = {}
  count[code.wall] = 0
  count[code.emptyFloor] = 0
  for i = 1, 10 do
    for j = 1, 13 do
      if map[i][j] == code.wall or map[i][j] == code.emptyFloor then
        count[map[i][j]] = count[map[i][j]] + 1
      end
    end
  end
  asserts.EQ(count[code.wall], 96)
  asserts.EQ(count[code.emptyFloor], 29)
end

return test_runner.run(tests)
