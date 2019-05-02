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

local factory = require 'factories.explore.factory'
local game = require 'dmlab.system.game'
local helpers = require 'common.helpers'
local hrp = require 'common.human_recognisable_pickups'
local random = require 'common.random'

local CATEGORY_NAME_PREFIX = 'rat_object'

local level = {}

-- Options set in createLevelApi. Keep in global so constants can be accessed.
local opts

local function categoryNameToId(name)
  return tonumber(name:match('^' .. CATEGORY_NAME_PREFIX .. '(%d+)$'))
end

local function categoryIdToName(categoryId)
  return CATEGORY_NAME_PREFIX .. categoryId
end

-- Key of the `_pickupLocations` table.
local function worldPosToKey(x, y)
  return y .. '_' .. x
end

--[[ Return the subset of pickup locations in `maze` that are accessible from
`spawnLocation`.

Key is world-location, value is pickupId, length is `density` * number of
possible locations.
--]]
local function selectPickupLocations(maze, spawnLocation, density)
  -- Find all rewards reachable from the spawn point.
  local locations = {}
  local pickupId = 0
  maze:visitFill{
      cell = spawnLocation,
      func = function(i, j, d)
        local c = maze:getEntityCell(i, j)
        if c == opts.objectEntity then
          local x, y = maze:toWorldPos(i, j)
          local key = worldPosToKey(x, y)
          table.insert(locations, key)
        end
      end
  }

  -- Keep only the first `density` percent of all possible locations.
  -- Reverse key <==> value, since we want the key to be location.
  local pickupCount = density * #locations
  local locationIdxGenerator = random:shuffledIndexGenerator(#locations)
  local pickupLocations = {}
  for pickupId = 1, pickupCount do
    local locationIdx = locationIdxGenerator()
    pickupLocations[locations[locationIdx]] = pickupId
  end
  return pickupLocations, pickupCount
end

-- Assign a random category to every pickupId. Ensure there is at least one
-- pickup of each category, if possible.
-- Keys are pickupId, values are categoryId, length is `pickupCount`.
local function assignCategories(pickupCount, categoryCount)
  local pickupCategories = {}
  for pickupId = 1, pickupCount do
    -- Ensure we have at least one of every category by assigning the first
    -- categoryCount pickups a unique category.
    local categoryId = (pickupId <= categoryCount) and
                          pickupId or random:uniformInt(1, categoryCount)
    table.insert(pickupCategories, categoryId)
  end
  return pickupCategories
end

-- Return array of {i, j} locations in `maze` that equal `type`.
local function mazeLocationsOfType(maze, type)
  local locations = {}
  local height, width = maze:size()
  for i = 1, height do
    for j = 1, width do
      local c = maze:getEntityCell(i, j)
      if c == type then
        table.insert(locations, {i, j})
      end
    end
  end
  return locations
end

-- Return true if all the pickups of positive reward have been picked-up.
-- Constant function.
function level:_allPickedUp()
  for pickupId, categoryId in ipairs(level._pickupCategories) do
    local reward = level._categoryQuantity[categoryId]
    if reward > 0 and not level._pickedUp[pickupId] then
      return false
    end
  end
  return true
end

-- Finish the map when we've picked up all the positive rewards.
function level:pickup(pickupId)
  level._pickedUp[pickupId] = true
  if level._allPickedUp() then
    game:finishMap()
  end
end

function level:restart(maze)
  -- Select player spawn location.
  local spawnIdx = random:uniformInt(1, #level._spawnLocations)
  local spawnLocation = level._spawnLocations[spawnIdx]
  level._spawnX, level._spawnY = maze:toWorldPos(spawnLocation[1],
                                                 spawnLocation[2])

  -- Select a subset of the pickup locations in the maze.
  -- Keys are world-locations, values are pickupIds.
  local pickupCount
  level._pickupLocations, pickupCount = selectPickupLocations(
      maze, spawnLocation, opts.pickupDensity)

  -- For each pickup, assign a category. The category determines what gets
  -- picked-up.
  -- Keys are pickupIds, values are categoryIds.
  level._pickupCategories = assignCategories(pickupCount,
                                             #level._categoryClassName)

  -- Reset the pickup tracker to "nothing picked up".
  -- Keys are pickupIds, values are `true` (i.e. it's a set).
  level._pickedUp = {}
end

function level:start(maze, episode, seed)
  random:seed(seed)
  level._spawnLocations = mazeLocationsOfType(maze, 'P')

  -- Categories 1,goodCategory give goodCategoryReward (positive),
  -- and goodCategory+1,categoryCount give badCategoryReward (negative).
  local goodCategoryCount = random:uniformInt(
      opts.minGoodCategory, opts.categoryCount - opts.minBadCategory)

  -- For each category, create the human-recognizable object that will be
  -- returned from createPickup(). The first `goodCategoryCount` objects will
  -- have positive reward, the rest negative.
  -- Keys are categoryIds, values are objects that can be instantiated.
  level._categoryClassName = {}
  level._categoryQuantity = {}
  local hrpPickups = hrp.uniquePickups(opts.categoryCount)
  for categoryId = 1, opts.categoryCount do
    hrpPickups[categoryId].quantity =
        (categoryId <= goodCategoryCount) and opts.goodCategoryReward or
                                              opts.badCategoryReward
    level._categoryQuantity[categoryId] = hrpPickups[categoryId].quantity
    level._categoryClassName[categoryId] = hrp.create(hrpPickups[categoryId])
  end
end

level.spawnVarsUpdater = {
    -- Only one of the player-start positions has been selected.
    -- See the setting of _spawnX,_spawnY.
    ['info_player_start'] = function(spawnVars)
      local x, y = unpack(helpers.spawnVarToNumberTable(spawnVars.origin))
      if x == level._spawnX and y == level._spawnY then
        return spawnVars
      else
        return nil
      end
    end,

    -- Apples are converted here into a human-recognizable class.
    -- The exact class is given by the categoryId assigned to the location.
    ['apple_reward'] = function(spawnVars)
      local x, y = unpack(helpers.spawnVarToNumberTable(spawnVars.origin))
      local key = worldPosToKey(x, y)
      local pickupId = level._pickupLocations[key]
      -- Only a subset of the level's apples have been selected for
      -- instantiation. The other apples are ignored.
      if pickupId then
        local categoryId = level._pickupCategories[pickupId]
        spawnVars.classname = level._categoryClassName[categoryId]
        spawnVars.id = tostring(pickupId)
        spawnVars.wait = "-1"
        return spawnVars
      else
        return nil
      end
    end
}

local function createLevelApi(optsOverride)
  opts = optsOverride or {}

  -- Set defaults if not already specified.
  opts.badCategoryReward = opts.badCategoryReward or -1
  opts.categoryCount = opts.categoryCount or 4
  opts.goodCategoryReward = opts.goodCategoryReward or 2
  opts.minBadCategory = opts.minBadCategory or 2
  opts.minGoodCategory = opts.minGoodCategory or 2
  opts.pickupDensity = opts.pickupDensity or 0.5
  opts.roomCount = opts.roomCount or 1
  opts.mazeWidth = opts.mazeWidth or 13
  opts.mazeHeight = opts.mazeHeight or 13
  opts.roomMaxSize = opts.roomMaxSize or 11
  opts.roomMinSize = opts.roomMinSize or 11

  -- Fill the maze with apples that we later change into human-recognizable
  -- pickups in `spawnVarsUpdater`.
  opts.objectEntity = 'A'

  -- Request apple_reward pickups. These will be transformed into one of several
  -- human-recognizable pickups when the spawnVars are updated above.
  return factory.createLevelApi{opts = opts, level = level}
end

return {
    createLevelApi = createLevelApi
}
