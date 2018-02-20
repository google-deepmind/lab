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
local set = require 'common.set'

--[[ Object placement strategies

A placer is a function which will be called to decide on the mapping between
object placeholders in the map and actual pickup ids.

Placers are called with the following keyword arguments:

*   `map` (table) Description of the map in use.
*   `mapIdToPickupId` (list) map -> pickup relationship.  The placer should
    modify this list directly to define the pickup placement.
*   `objectDescriptions` (list) The objects present in the level.

Returns nothing, mapIdToPickupId is modified in place.
]]
local placers = {}

function placers.createStatic()
  return function(kwargs)
    table.sort(kwargs.mapIdToPickupId)
  end
end

function placers.createRandom()
  return function(kwargs)
    random:shuffleInPlace(kwargs.mapIdToPickupId)
  end
end

--[[ Room placer allows objects to specify where they should be placed.

If an object's region attribute is, e.g. 'A' then only consider map locations
whch correspond with 'A' in the variation layer.  A region of 'any' indicates
that the object can be placed anywhere.
]]
function placers.createRoom()
  return function(kwargs)
    local mapIdToPickupId = kwargs.mapIdToPickupId
    local objectDescriptions = kwargs.objectDescriptions
    local map = kwargs.map
    local unusedMapIds = set.Set(mapIdToPickupId)

    local function isLocationInRoom(map, room, location)
      return room == 'any' or map.source.rooms[room][location]
    end

    local function getRoomLocations(map, room, unusedIds)
      local locations = {}
      for mapId, _ in pairs(unusedIds) do
        if isLocationInRoom(map, room, mapId) then
          locations[#locations + 1] = mapId
        end
      end
      return locations
    end

    for _, object in ipairs(objectDescriptions) do
      if object.region ~= 'DO_NOT_PLACE' then
        local candidates = getRoomLocations(map, object.region, unusedMapIds)
        if #candidates <= 0 then
          error('No rooms found for ' .. object.region)
        end
        local mapId = random:choice(candidates)
        unusedMapIds[mapId] = nil
        mapIdToPickupId[mapId] = object._pickupId
      end
    end
    -- Ensure any remaining unused slots don't have a pickup.
    for index, _ in pairs(unusedMapIds) do mapIdToPickupId[index] = 0 end
  end
end

return placers
