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

local TEXT_TABLE = {
    key = 'K',
    goal = 'G',
    spawn = 'P',
    vertDoor = 'I',
    horizDoor = 'H',
    wall = '*',
    emptyFloor = ' ',
}

local api = {}

local function getAscii(object)
  if object:match('Key') then
    return TEXT_TABLE.key
  elseif object:match('VertDoor') then
    return TEXT_TABLE.vertDoor
  elseif object:match('HorizDoor') then
    return TEXT_TABLE.horizDoor
  else
    return TEXT_TABLE[object]
  end
end

--[[ Builds the object list based on possible colors, and also builds the
inverted index.

Arguments:

*   possibleColors - The list of possible colors of the keys and doors.

Returns:

*   objects - The objects list.
*   objectCode - The inverted index of the objects list.
]]
function api.createObjects(possibleColors)
  local objects = {}
  for _, color in ipairs(possibleColors) do
    objects[#objects + 1] = color .. 'Key'
    objects[#objects + 1] = color .. 'VertDoor'
    objects[#objects + 1] = color .. 'HorizDoor'
  end
  objects[#objects + 1] = 'spawn'
  objects[#objects + 1] = 'goal'
  objects[#objects + 1] = 'wall'
  objects[#objects + 1] = 'emptyFloor'

  local objectCodes = {}
  for i, object in ipairs(objects) do
    objectCodes[object] = i
  end

  return objects, objectCodes
end

--[[ Transform the rooms and objects information into an ASCII map, a key list
and a door list.

Arguments:

*   roomGrid - An instance of RoomGrid which holds the rooms information.
*   objects - The objects list
*   objectCodes - The inverted index of the objects list.

Returns a table with three keys:

*   map - The ASCII map of the level.
*   keys - The colors of the keys in the map, listed from the top to bottom and
    from left to right.
*   doors - The colors of the doors in the map, listed from the top to bottom
    and from left to right.
]]
function api.makeLevel(roomGrid, objects, objectCodes)
  local mini = math.huge
  local maxi = - math.huge
  local minj = math.huge
  local maxj = - math.huge
  for i = 1, roomGrid.height do
    for j = 1, roomGrid.width do
      if roomGrid:grid(i, j) then
        mini = math.min(mini, i)
        maxi = math.max(maxi, i)
        minj = math.min(minj, j)
        maxj = math.max(maxj, j)
      end
    end
  end
  local roomRows = maxi - mini + 1
  local roomCols = maxj - minj + 1
  local map = {}
  for i = 1, roomRows * 3 + 3 do
    map[i] = {}
    for j = 1, roomCols * 4 + 3 do
      map[i][j] = objectCodes['wall']
    end
  end

  for i, room in ipairs(roomGrid.rooms) do
    local gridi, gridj = room:gridPosition()
    room:draw(map,
        (gridi - mini) * 3 + 3,
        (gridj - minj) * 4 + 3,
        objectCodes)
  end

  local asciiMap = ''
  local keys = {}
  local doors = {}

  for i = 1, roomRows * 3 + 3 do
    for j = 1, roomCols * 4 + 3 do
      local object = objects[map[i][j]]
      assert(object)
      local objectAscii = getAscii(object)
      asciiMap = asciiMap .. objectAscii
      if objectAscii == TEXT_TABLE.key then
        table.insert(keys, object:match('(.*)Key'))
      elseif objectAscii == TEXT_TABLE.vertDoor then
        table.insert(doors, object:match('(.*)VertDoor'))
      elseif objectAscii == TEXT_TABLE.horizDoor then
        table.insert(doors, object:match('(.*)HorizDoor'))
      end
    end
    asciiMap = asciiMap .. '\n'
  end

  return {
      map = asciiMap,
      keys = keys,
      doors = doors,
  }
end

return api
