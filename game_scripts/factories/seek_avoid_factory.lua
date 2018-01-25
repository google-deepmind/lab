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

local game = require 'dmlab.system.game'
local pickups = require 'common.pickups'
local helpers = require 'common.helpers'
local custom_observations = require 'decorators.custom_observations'
local setting_overrides = require 'decorators.setting_overrides'
local timeout = require 'decorators.timeout'
local random = require 'common.random'
local map_maker = require 'dmlab.system.map_maker'
local randomMap = random(map_maker:randomGen())

local factory = {}

--[[ Creates a Seek Avoid API.
Keyword arguments:

*   `mapName` (string) - Name of map to load.
*   `episodeLengthSeconds` (number) - Episode length in seconds.
]]
function factory.createLevelApi(kwargs)
  assert(kwargs.mapName and kwargs.episodeLengthSeconds)

  local api = {}

  function api:createPickup(classname)
    return pickups.defaults[classname]
  end

  function api:start(episode, seed, params)
    random:seed(seed)
    randomMap:seed(random:mapGenerationSeed())
    api._has_goal = false
    api._count = 0
    api._finish_count = 0
  end

  function api:pickup(spawn_id)
    api._count = api._count + 1
    if not api._has_goal and api._count == api._finish_count then
      game:finishMap()
    end
  end

  function api:updateSpawnVars(spawnVars)
    local classname = spawnVars.classname
    if spawnVars.random_items then
      local possibleClassNames = helpers.split(spawnVars.random_items, ',')
      if #possibleClassNames > 0 then
        classname = possibleClassNames[
          random:uniformInt(1, #possibleClassNames)]
      end
    end
    local pickup = pickups.defaults[spawnVars.classname]
    if pickup then
      if pickup.type == pickups.type.REWARD and pickup.quantity > 0 then
        api._finish_count = api._finish_count + 1
        spawnVars.id = tostring(api._finish_count)
      end
      if pickup.type == pickups.type.GOAL then
        api._has_goal = true
      end
    end

    return spawnVars
  end

  function api:nextMap()
    -- Fast map restarts.
    local map = kwargs.mapName
    kwargs.mapName = ''
    return map
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
