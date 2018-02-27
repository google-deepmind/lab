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

local custom_observations = require 'decorators.custom_observations'
local game = require 'dmlab.system.game'
local geometric_pickups = require 'common.geometric_pickups'
local helpers = require 'common.helpers'
local map_maker = require 'dmlab.system.map_maker'
local pickups = require 'common.pickups'
local random = require 'common.random'
local setting_overrides = require 'decorators.setting_overrides'
local randomMap = random(map_maker:randomGen())


local factory = {}

--[[ Returns a table with two named entries, each of which is a random object.
The reward values of the 'match' and 'nonMatch' named objects is according to
the supplied parameters.
]]
local function createNonMatchRandomObjectPair(matchReward, nonMatchReward)
  local objects = {}
  local pickupKeys = geometric_pickups.randomUniquePickupNames(2)

  local matchObjectName = pickupKeys[1]
  objects.match = {
      name = matchObjectName,
      classname = matchObjectName,
      model = 'models/' .. matchObjectName .. '.md3',
      quantity = matchReward,
      type = pickups.type.GOAL
  }

  local nonMatchObjectName = pickupKeys[2]
  objects.nonMatch = {
      name = nonMatchObjectName,
      classname = nonMatchObjectName,
      model = 'models/' .. nonMatchObjectName .. '.md3',
      quantity = nonMatchReward,
      type = pickups.type.GOAL
  }

  return objects
end

--[[ Creates a Non-Match level API.

Keyword arguments:

*   `mapName` (string) Name of map file to load with level script.
*   `teleportReward` (number) Amount of reward given when teleporting.
*   `matchReward` (number) Amount of reward given for picking up the
    object that matches the example object (typically negative).
*   `nonMatchReward` (number) Amount of reward given for picking up the
    object that does not match the example object (typically positive).
*   `randomizePositions` (boolean) Determines whether positions of the example
    object and choice objects be randomised each time.
*   `randomizeNonMatchObject` (boolean) Determines whether the object that does
    not match the example object be randomised each time.
*   `randomizeEachTime` (boolean) Whether all objects and positions will be
    randomised each time.

Returns a DM Lab level API.
]]

function factory.createLevelApi(kwargs)
  assert(kwargs.mapName)
  assert(kwargs.episodeLengthSeconds)
  assert(kwargs.teleportReward)
  assert(kwargs.matchReward)
  assert(kwargs.nonMatchReward)
  assert(kwargs.randomizePositions ~= nil)
  assert(kwargs.randomizeNonMatchObject ~= nil)
  assert(kwargs.randomizeEachTime ~= nil)

  local api = {}

  function api:createPickup(classname)
    return api._pickups[classname] or pickups.defaults[classname]
  end

  function api:start(episode, seed, params)
    random:seed(seed)
    randomMap:seed(seed)
    api._time_remaining = kwargs.episodeLengthSeconds
    api._objects = nil
  end

  function api:nextMap()
    api._pickups = {}

    if api._objects == nil or kwargs.randomizeEachTime then
      api._objects = createNonMatchRandomObjectPair(kwargs.matchReward,
                                                    kwargs.nonMatchReward)
    end

    if not kwargs.randomizeEachTime and kwargs.randomizeNonMatchObject then
      local objects = createNonMatchRandomObjectPair(kwargs.nonMatchReward,
                                                     kwargs.nonMatchReward)
      -- Find an object that is not the same as the current match object
      api._objects.nonMatch =
          objects.nonMatch.classname == api._objects.match.classname and
          objects.match or objects.nonMatch
    end

    local exampleSpawnLocations = {'pickup_example_1', 'pickup_example_2'}
    local choiceSpawnLocations = {'pickup_choice_1', 'pickup_choice_2'}

    if kwargs.randomizeEachTime or kwargs.randomizePositions then
      random:shuffleInPlace(exampleSpawnLocations)
      random:shuffleInPlace(choiceSpawnLocations)
    end

    api._pickups[exampleSpawnLocations[1]] = api._objects.match
    api._pickups[choiceSpawnLocations[1]] = api._objects.nonMatch
    api._pickups[choiceSpawnLocations[2]] = api._objects.match

    return kwargs.mapName
  end

  function api:trigger(spawnId, targetName)
    game:addScore(kwargs.teleportReward)
    return false
  end

  custom_observations.decorate(api)
  setting_overrides.decorate{
      api = api,
      apiParams = kwargs,
      decorateWithTimeout = true
  }
  return api
end

return factory
