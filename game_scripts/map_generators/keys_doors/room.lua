--[[ Copyright (C) 2018 Google Inc.

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

local random = require 'common.random'

local Room = {}
local RoomMT = {__index = Room}

function Room.new(config)
  local self = {}
  setmetatable(self, RoomMT)

  self.id = config.id
  self._grid = config.grid
  self._entryRoom = config.entryRoom
  self._entryDoorColor = config.entryDoorColor

  self._connectedRooms = {}
  self._connectedDoors = {}
  self._objects = {}
  self._objectsOffsets = random:uniformInt(0, 1) == 0 and {0, 2} or {2, 0}
  if self._entryRoom then
    self._entryRoom:connectRoom(self)
  end
  if self._entryDoorColor then
    assert(self._entryRoom, 'Entry door requires entry room.')
    self._entryRoom:addDoor(self._grid[1], self._grid[2], self._entryDoorColor)
  end

  return self
end

function Room:gridPosition()
  return unpack(self._grid)
end

function Room:entryDoorColor()
  return self._entryDoorColor
end

function Room:connectedRoom(gridi, gridj)
  return self._connectedRooms[gridi] and self._connectedRooms[gridi][gridj]
end

function Room:connectedDoor(gridi, gridj)
  return self._connectedDoors[gridi] and self._connectedDoors[gridi][gridj]
end

-- Note this is not symmetrically counted.
function Room:connectRoom(room2)
  local gridi, gridj = room2:gridPosition()
  assert(not self:connectedRoom(gridi, gridj))
  self._connectedRooms[gridi] = self._connectedRooms[gridi] or {}
  self._connectedRooms[gridi][gridj] = room2
end

function Room:addDoor(gridi, gridj, color)
  assert(self:connectedRoom(gridi, gridj))
  assert(not self:connectedDoor(gridi, gridj))
  self._connectedDoors[gridi] = self._connectedDoors[gridi] or {}
  self._connectedDoors[gridi][gridj] = color
end

function Room:addObject(key)
  if not self:hasObjectSpace() then
    return false
  end
  self._objects[#self._objects + 1] = key
  return true
end

function Room:hasObjectSpace()
  return #self._objects < 2
end

function Room:addPlayerSpawn()
  self.playerSpawn = true
end

--[[ A room looks like this:

xPPPx
.OPO.
..x..

The position [topLefti, topLeftj] refers to top left P.
Rows counted downwards; O possible objects, P possible spawn.
]]
function Room:draw(map, topLefti, topLeftj, objectCodes)
  -- This area of the map should still be untouched
  for i = topLefti, topLefti + 1 do
    for j = topLeftj, topLeftj + 2 do
      assert(map[i][j] == objectCodes['wall'])
    end
  end

  -- Init to empty.
  for i = topLefti, topLefti + 1 do
    for j = topLeftj, topLeftj + 2 do
      map[i][j] = objectCodes['emptyFloor']
    end
  end

  for i, object in ipairs(self._objects) do
    map[topLefti + 1][topLeftj + self._objectsOffsets[i]] = objectCodes[object]
  end

  local function writeMap(pos, objectName)
    assert(map[pos[1]][pos[2]] == objectCodes['emptyFloor'] or
        map[pos[1]][pos[2]] == objectCodes['wall'],
        'Cannot draw postion {' .. pos[1] .. ', ' .. pos[2] .. '} as ' ..
        objectName .. ', already drawn as object ' .. map[pos[1]][pos[2]])
    map[pos[1]][pos[2]] = objectCodes[objectName]
  end

  local function drawDoor(gridi, gridj, posi, posj, doorType)
    if self:connectedRoom(gridi, gridj) then
      local doorColor = self:connectedDoor(gridi, gridj)
      if doorColor then
        writeMap({posi, posj}, doorColor .. doorType)
      else
        writeMap({posi, posj}, 'emptyFloor')
      end
    else
      assert(not self:connectedDoor(gridi, gridj))
    end
  end

  local gridi, gridj = self:gridPosition()
  drawDoor(gridi + 1, gridj, topLefti + 2, topLeftj + 1, 'HorizDoor')
  drawDoor(gridi - 1, gridj, topLefti - 1, topLeftj + 1, 'HorizDoor')
  drawDoor(gridi, gridj - 1, topLefti, topLeftj - 1, 'VertDoor')
  drawDoor(gridi, gridj + 1, topLefti, topLeftj + 3, 'VertDoor')

  if self.playerSpawn then
    local choice = random:uniformInt(1, 4)
    local positions = {
        {topLefti, topLeftj},
        {topLefti, topLeftj + 1},
        {topLefti, topLeftj + 2},
        {topLefti + 1, topLeftj + 1}
    }
    local position = positions[choice]
    writeMap(position, 'spawn')
  end
end


return Room

