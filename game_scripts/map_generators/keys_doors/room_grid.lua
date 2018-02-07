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

local Room = require 'map_generators.keys_doors.room'
local random = require 'common.random'

local RoomGrid = {}
local RoomGridMT = {__index = RoomGrid}

function RoomGrid.new(config)
  assert(config)
  assert(config.roomGridHeight)
  assert(config.roomGridWidth)

  local self = {}
  setmetatable(self, RoomGridMT)

  self.rooms = {}
  self._grid = {}
  self.height = config.roomGridHeight
  self.width = config.roomGridWidth
  self._compact = config.compact

  return self
end

function RoomGrid:grid(gridi, gridj)
  return self._grid[gridi] and self._grid[gridi][gridj]
end

function RoomGrid:createRoom(roomConfig)
  roomConfig.id = #self.rooms + 1
  local room = Room.new(roomConfig)
  local gridi, gridj = room:gridPosition()

  self.rooms[room.id] = room
  self._grid[gridi] = self._grid[gridi] or {}
  assert(not self._grid[gridi][gridj])
  self._grid[gridi][gridj] = room

  return room
end

function RoomGrid:getRandomFreeGridPos(room, startRoomPos)
  local freePositions = self:getAllFreeGridPos(room)
  if not freePositions then
    return nil
  end
  if self._compact then
    freePositions =
        self:findClosestPositionsToStart(freePositions, startRoomPos)
  end
  local freeGridPos = freePositions[random:uniformInt(1, #freePositions)]
  return freeGridPos
end

function RoomGrid:getAllFreeGridPos(room)
  local gridi, gridj = room:gridPosition()
  local possiblePositions = {
      {gridi, gridj + 1}, {gridi + 1, gridj},
      {gridi, gridj - 1}, {gridi - 1, gridj}
  }
  local freePositions = {}
  for i, pos in ipairs(possiblePositions) do
    if not self:grid(pos[1], pos[2]) and
        pos[1] > 0 and pos[1] <= self.height and
        pos[2] > 0 and pos[2] <= self.width then
      freePositions[#freePositions + 1] = pos
    end
  end
  if #freePositions > 0 then
    return freePositions
  else
    return nil
  end
end

function RoomGrid:findClosestPositionsToStart(positions, startRoomPos)
  local closestPositions = {}
  local closestIndices = {}
  local closest = nil

  -- Take into account that rooms are wider in y than x.
  local function roomDistance(pos)
    local xdiff = (pos[1] - startRoomPos[1]) * 3
    local ydiff = (pos[2] - startRoomPos[2]) * 4
    return xdiff * xdiff + ydiff * ydiff
  end

  for i, pos in ipairs(positions) do
    assert(not self:grid(pos[1], pos[2]))
    closest = closest or pos
    if roomDistance(pos) < roomDistance(closest) then
      closest = pos
      closestPositions = {pos}
      closestIndices = {i}
    elseif roomDistance(pos) == roomDistance(closest) then
      closestPositions[#closestPositions + 1] = pos
      closestIndices[#closestIndices + 1] = i
    end
  end
  assert(#closestPositions > 0)
  return closestPositions, closestIndices
end

function RoomGrid:_makeIncludedMap(excludeIds, includeIds)
  local excludeIds = excludeIds or {}
  if not includeIds then
    includeIds = {}
    for id = 1, #self.rooms do
      includeIds[#includeIds + 1] = id
    end
  end
  assert(type(excludeIds) == 'table')
  assert(type(includeIds) == 'table')
  local isIncluded = {}
  for _, id in ipairs(includeIds) do
    isIncluded[id] = true
  end
  for _, id in ipairs(excludeIds) do
    isIncluded[id] = false
  end
  return isIncluded
end

function RoomGrid:findRoomWithObjectSpace(excludeIds, includeIds)
  local isIncluded = self:_makeIncludedMap(excludeIds, includeIds)
  local possibleRooms = {}
  for i, room in ipairs(self.rooms) do
    if isIncluded[i] and room:hasObjectSpace() then
      possibleRooms[#possibleRooms + 1] = room
    end
  end
  if #possibleRooms > 0 then
    return possibleRooms[random:uniformInt(1, #possibleRooms)]
  end
  return nil
end

-- Find a free grid position beside a room in includeIds but not in excludeIds.
function RoomGrid:findRoomWithGridSpace(startRoomPos, excludeIds, includeIds)
  local isIncluded = self:_makeIncludedMap(excludeIds, includeIds)
  local possibleRooms = {}
  local freeGridPositions = {}
  for i, room in ipairs(self.rooms) do
    if isIncluded[i] then
      local freeGridPos = self:getRandomFreeGridPos(room, startRoomPos)
      if freeGridPos then
        possibleRooms[#possibleRooms + 1] = room
        freeGridPositions[#freeGridPositions + 1] = freeGridPos
      end
    end
  end
  if #possibleRooms == 0 then
    return nil
  end
  local index
  if self._compact then
    local _, indices =
        self:findClosestPositionsToStart(freeGridPositions, startRoomPos)
    index = indices[random:uniformInt(1, #indices)]
  else
    index = random:uniformInt(1, #possibleRooms)
  end
  return possibleRooms[index], freeGridPositions[index]
end

return RoomGrid

