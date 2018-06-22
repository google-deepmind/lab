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

local log = require 'common.log'
local random = require 'common.random'
local configs = require 'map_generators.keys_doors.configs'
local RoomGrid = require 'map_generators.keys_doors.room_grid'
local objectsAndMap = require 'map_generators.keys_doors.objects_and_map'

local config
local roomGrid
local objects
local objectCodes

local coreSequenceColors
local coreRooms
local startRoomPos

local freeColors
local connectingRooms
local coreColorIdsByRoomId
local trapRooms
local redHerringRooms
local roomsToExclude
local noCoreKeyRoomIds
local iCoreColor
local emptyRooms
local unopenableColors

-- Prints the value if config.verbose is true. For tables, it will print the
-- key/value pairs.
local function verbosePrint(val)
  if config.verbose or log.getLogLevel() > log.INFO then
    if type(val) == 'table' then
      print('{')
      for k, v in pairs(val) do
        print(k .. ' = ' .. tostring(v) .. ',')
      end
      print('}')
    else
      print(val)
    end
  end
end

-- Santinizes the config and generates the objects list and its inverted index.
local function initialize(overrideConfig)
  -- Try generating a config and santinize it.
  config = configs.getConfig(overrideConfig)
  if config.startRoomGridPositions then
    for _, possiblePos in pairs(config.startRoomGridPositions) do
      assert(
          possiblePos[1] > 0 and
          possiblePos[1] <= config.roomGridHeight and
          possiblePos[2] > 0 and
          possiblePos[2] <= config.roomGridWidth)
    end
  end

  objects, objectCodes = objectsAndMap.createObjects(config.possibleColors)
  verbosePrint(objects)
end

--[[ Trap room is a kind room whose door shares the same color of the door of
the competingRoom, and it contains a key to open a future door. However, if you
use the existing key to enter it, you will not be able to enter the competing
room, which is in the core path, thus makes it impossible to reach the goal.

Returns:

*   Whether successfully added a trap room.
]]
local function addTrapRoom(competingRoom)
  assert(#trapRooms <= config.numTrapRooms)
  if config.numTrapRooms == #trapRooms or
      coreColorIdsByRoomId[competingRoom.id] == #coreSequenceColors then
    return false
  end
  -- Exclude later rooms
  local laterIds = {}
  for index = competingRoom.id, #roomGrid.rooms do
    laterIds[#laterIds + 1] = index
  end
  local entryRoom, trapPos = roomGrid:findRoomWithGridSpace(
      startRoomPos, laterIds)
  if entryRoom then
    -- The door to the trap room can be opened with a key needed for the
    -- puzzle, wasting the key.
    local trapRoom = roomGrid:createRoom{
        grid = trapPos,
        entryRoom = entryRoom,
        entryDoorColor = competingRoom:entryDoorColor()
    }
    -- Add a key from later in the sequence to the trap room as lure
    -- Problem to be fixed: it's not just enough that the key comes
    -- from later in the sequence; the door the key belongs to also
    -- has to be after the connecting room, rather than before, on
    -- path to the goal (the latter can happen when backtracking).
    -- Leaving this unfixed for now.
    local lureColor = random:choice(
        coreSequenceColors,
        coreColorIdsByRoomId[competingRoom.id] + 1)
    trapRoom:addObject(lureColor .. 'Key')
    trapRooms[#trapRooms + 1] = trapRoom.id
    noCoreKeyRoomIds[#noCoreKeyRoomIds + 1] = trapRoom.id
    return true
  end
  return false
end

--[[ Red herring rooms are rooms whose doors can be opened by keys in the core
path, so that agents may waste their keys to enter them.

Returns:

*   Whether successfully added a red herring room.
]]
local function addRedHerringRoom()
  if #coreSequenceColors == 0 or
      #redHerringRooms >= config.numRedHerringRooms then
    return false
  end
  local maxColorId
  if config.redHerringRespectCoreOrder then
    maxColorId = iCoreColor
  else
    maxColorId = #coreSequenceColors
  end
  local colorToAdd = random:choice(coreSequenceColors, 1, maxColorId)
  local entryRoom, gridPos = roomGrid:findRoomWithGridSpace(
      startRoomPos, roomsToExclude)
  if entryRoom then
    local room = roomGrid:createRoom{
        grid = gridPos,
        entryRoom = entryRoom,
        entryDoorColor = colorToAdd
    }
    redHerringRooms[#redHerringRooms + 1] = room.id
    noCoreKeyRoomIds[#noCoreKeyRoomIds + 1] = room.id
    return true
  end
  return false
end

-- Add extra doors between existing adjacent rooms. The doors can be either
-- openable or unopenable.
local function addExtraDoors(openable)
  local maxDoors, colors
  if openable then
    maxDoors = config.numOpenableDoors
    colors = coreSequenceColors
  else
    maxDoors = config.numUnopenableDoors
    colors = freeColors
    if #colors == 0 then
      verbosePrint('No free colors for extra unopenable doors.')
      return
    end
  end

  local numDoors = 0
  local possibleDirections = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}}

  -- Permute the sequence of the rooms to add randomness.
  local permutedRooms = {}
  for _, room in ipairs(roomGrid.rooms) do
    permutedRooms[#permutedRooms + 1] = room
  end
  for i = 1, #permutedRooms do
    local j = random:uniformInt(i, #permutedRooms)
    permutedRooms[i], permutedRooms[j] = permutedRooms[j], permutedRooms[i]
  end

  for _, room in ipairs(permutedRooms) do
    if numDoors == maxDoors then
      break
    end
    for _, dir in ipairs(possibleDirections) do
      if numDoors == maxDoors then
        break
      end
      -- Pick up a room and one of its adjacent rooms.
      local gridi, gridj = room:gridPosition()
      local i = gridi + dir[1]
      local j = gridj + dir[2]
      local targetRoom = roomGrid:grid(i, j)
      if targetRoom then
        -- Check if the two rooms are connected before.
        if not room:connectedDoor(i, j) and
            not targetRoom:connectedDoor(gridi, gridj) and
            not room:connectedRoom(i, j) and
            not targetRoom:connectedRoom(gridi, gridj) then
          room:connectRoom(targetRoom)
          local colorToAdd = random:choice(colors)
          if not openable then
            unopenableColors[colorToAdd] = true
          end
          room:addDoor(i, j, colorToAdd)
          numDoors = numDoors + 1
        end
      end
    end
  end
  if openable then
    verbosePrint('Extra (potentially) openable doors added: ' .. numDoors)
  else
    verbosePrint('Extra unopenable doors added: ' .. numDoors)
    verbosePrint(unopenableColors)
  end
end

--[[ Try making a set of rooms according to the sampled config.

Returns:

*   Whether the process is successful or not.
]]
local function tryMakingRooms(overrideConfig)
  config = configs.getConfig(overrideConfig)
  roomGrid = RoomGrid.new(config)

  freeColors = {}
  for _, color in ipairs(config.possibleColors) do
    freeColors[#freeColors + 1] = color
  end
  coreSequenceColors = {}
  for _ = 1, config.coreSequenceLength do
    local colorId = random:uniformInt(1, #freeColors)
    coreSequenceColors[#coreSequenceColors + 1] = freeColors[colorId]
    table.remove(freeColors, colorId)
  end
  verbosePrint('Core sequence colors:')
  verbosePrint(coreSequenceColors)

  coreRooms = {}
  if config.startRoomGridPositions then
    startRoomPos = random:choice(config.startRoomGridPositions)
  else
    startRoomPos = {
        random:uniformInt(1, config.roomGridHeight),
        random:uniformInt(1, config.roomGridWidth)}
  end
  verbosePrint('Start room pos:')
  verbosePrint(startRoomPos)

  local lastRoom = roomGrid:createRoom{grid = startRoomPos}
  lastRoom:addPlayerSpawn()
  coreRooms[#coreRooms + 1] = lastRoom

  connectingRooms = {}
  coreColorIdsByRoomId = {}
  trapRooms = {}
  redHerringRooms = {}
  roomsToExclude = {}
  noCoreKeyRoomIds = {}
  iCoreColor = 0
  emptyRooms = {}

  -- Add rooms.
  for iRoom = 1, config.coreSequenceLength do
    local lastWasEmpty = false
    if #emptyRooms < config.numMaxEmptyRooms and
        random:uniformReal(0, 1) <= config.probEmptyRoomDuringCore then
      local lastRoomFreePos =
          roomGrid:getRandomFreeGridPos(lastRoom, startRoomPos)
      if lastRoomFreePos then
        verbosePrint('Adding empty room in core sequence')
        lastRoom = roomGrid:createRoom{
            grid = lastRoomFreePos,
            entryRoom = lastRoom
        }
        -- Otherwise would need to change how addTrapRoom works with the
        -- coreSequenceColors list.
        lastWasEmpty = true
        emptyRooms[#emptyRooms + 1] = lastRoom.id
        table.insert(emptyRooms, lastRoom.id)
      end
    end

    iCoreColor = iCoreColor + 1

    -- Compactness can lead to patterns if we don't randomize the order.
    local placeRedHerring
    if random:uniformReal(0, 1) <= config.probRedHerringRoomDuringCore then
      local ordering = {'before', 'after'}
      placeRedHerring = random:choice(ordering)
    end

    if placeRedHerring == 'before' then
      addRedHerringRoom()
    end

    -- Add at least one to the last room: the new room and door, or the new key.
    local case = random:uniformInt(1, 2)
    if config.coreNoEarlyKeys then
      case = 2
    end
    local lastRoomFreePos =
        roomGrid:getRandomFreeGridPos(lastRoom, startRoomPos)
    local connectingRoom, roomToAddKey, gridPos
    if case == 1 and lastRoomFreePos then
      connectingRoom = lastRoom
      roomToAddKey = roomGrid:findRoomWithObjectSpace(noCoreKeyRoomIds)
      assert(roomToAddKey)
      gridPos = lastRoomFreePos
    else
      roomToAddKey = lastRoom
      -- Enforce connecting to the last room if there is not enough space for
      -- the required number of trap rooms.
      if config.numTrapRooms - #trapRooms >=
          config.coreSequenceLength - iRoom + 1 then
        connectingRoom = lastRoom
        gridPos = lastRoomFreePos
      else
        connectingRoom, gridPos = roomGrid:findRoomWithGridSpace(
            startRoomPos, noCoreKeyRoomIds)
      end
      if not connectingRoom then
        verbosePrint('Restarting generation: ran out of grid space when ' ..
            'placing core rooms.')
        return false
      end
    end

    local nextCoreColor = coreSequenceColors[iCoreColor]
    roomToAddKey:addObject(nextCoreColor .. 'Key')
    local newRoom = roomGrid:createRoom{
        grid = gridPos,
        entryRoom = connectingRoom,
        entryDoorColor = nextCoreColor}
    coreColorIdsByRoomId[newRoom.id] = iCoreColor

    -- Don't include the start room for trap room purposes.
    if #coreRooms > 1 and connectingRoom == lastRoom then
      connectingRooms[#connectingRooms + 1] = coreRooms[#coreRooms]
      -- Add some randomness to make room placement more varied
      if random:uniformReal(0, 1) <= config.probTrapRoomDuringCore then
        local trapRoom = addTrapRoom(coreRooms[#coreRooms])
        if trapRoom then
          verbosePrint('Adding trap room while placing core rooms.')
        end
      end
    end

    lastRoom = newRoom
    coreRooms[#coreRooms + 1] = lastRoom

    if iRoom == config.coreSequenceLength then
      -- Add goal
      lastRoom:addObject('goal')
      if not config.addToGoalRoom then
        roomsToExclude[#roomsToExclude + 1] = lastRoom.id
      end
    end
    if placeRedHerring == 'after' then
      addRedHerringRoom()
    end
  end

  -- Add left over trap rooms if possible
  for _, connectingRoom in ipairs(connectingRooms) do
    if #trapRooms >= config.numTrapRooms then
      break
    end
    addTrapRoom(connectingRoom)
  end
  verbosePrint('Total number of trap rooms: ' .. #trapRooms)

  for _ = 1, config.numEasyExtraRooms do
    local colorToAdd = random:choice(freeColors)
    local roomToAddRoom, gridPos = roomGrid:findRoomWithGridSpace(
        startRoomPos, roomsToExclude)
    local extraRoom = roomGrid:createRoom{
        grid = gridPos,
        entryRoom = roomToAddRoom,
        entryDoorColor = colorToAdd
    }
    verbosePrint('Adding easy extra room.')
  end

  -- Add left over red herring rooms if possible
  while #redHerringRooms < config.numRedHerringRooms do
    if not addRedHerringRoom() then
      break
    end
  end

  unopenableColors = {}
  -- unopenable doors
  addExtraDoors(false)
  -- openable doors
  addExtraDoors(true)

  local validUselessColors = {}
  for _, color in ipairs(freeColors) do
    if not unopenableColors[color] then
      validUselessColors[#validUselessColors + 1] = color
    end
  end

  for _ = 1, config.numRedHerringRoomUselessKeys do
    local roomToAddKey = roomGrid:findRoomWithObjectSpace(nil, redHerringRooms)
    if #validUselessColors == 0 or not roomToAddKey then
      break
    end
    local colorToAdd = random:choice(validUselessColors)
    assert(roomToAddKey:addObject(colorToAdd .. 'Key'))
    verbosePrint('Adding useless key to herring room.')
  end

  for _ = 1, config.numUselessKeys do
    local roomToAddKey = roomGrid:findRoomWithObjectSpace(roomsToExclude)
    if #validUselessColors == 0 or not roomToAddKey then
      break
    end
    local colorToAdd = random:choice(validUselessColors)
    assert(roomToAddKey:addObject(colorToAdd .. 'Key'))
    verbosePrint('Adding useless key.')
  end

  if config.fillWithEmptyRooms then
    local room, emptySpace = roomGrid:findRoomWithGridSpace(startRoomPos)
    local numEmptyRooms = 0
    while emptySpace do
      local emptyRoom = roomGrid:createRoom{
          grid = emptySpace,
          entryRoom = room,
      }
      numEmptyRooms = numEmptyRooms + 1
      room, emptySpace = roomGrid:findRoomWithGridSpace(startRoomPos)
    end
    verbosePrint('Added ' .. numEmptyRooms .. ' empty rooms.')
  end

  for _ = 1, config.extraUsefulKeys do
    local roomToAddKey = roomGrid:findRoomWithObjectSpace(roomsToExclude)
    if not roomToAddKey then
      break
    end
    local colorToAdd = random:choice(coreSequenceColors)
    roomToAddKey:addObject(colorToAdd .. 'Key')
    verbosePrint('Adding extra useful key.')
  end

  return true
end

--[[ Makes keys_doors level.

Arguments:

*   overrideConfig - Used to override the values in default config.
*   seed - Seed for randomization.

Returns a table with three keys:

*   map - The ASCII map of the level.
*   keys - The colors of the keys in the map, listed from the top to bottom and
    from left to right.
*   doors - The colors of the doors in the map, listed from the top to bottom
    and from left to right.
]]
local function makeLevel(overrideConfig, seed)
  if seed then
    random:seed(seed)
  end
  initialize(overrideConfig)
  repeat
  until tryMakingRooms(overrideConfig)
  return objectsAndMap.makeLevel(roomGrid, objects, objectCodes)
end

return {makeLevel = makeLevel}
