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
local map_maker = require 'dmlab.system.map_maker'
local model = require 'dmlab.system.model'
local pickups = require 'common.pickups'
local random = require 'common.random'
local randomMap = random(map_maker:randomGen())
local setting_overrides = require 'decorators.setting_overrides'
local transform = require 'common.transform'

local factory = {}

local PICKUP_VERTICAL_OFFSET = 20.0
local LOAD_MD3 = 'models/mushroom_desert_01a.md3'

local PICKUPS = {
    frag_mushroom_desert_01a = {
        name = 'Mushroom',
        classname = 'frag_mushroom_desert_01a',
        model = 'mushroom_reward',
        quantity = 1,
        type = pickups.type.REWARD,
        moveType = pickups.moveType.STATIC
    }
}

--[[ Creates a Natural Labyrinth API.
Keyword arguments:

*   `maps` (array of strings) Map names to choose from.
*   `episodeLengthSeconds` (number) Episode length in seconds.
]]
function factory.createLevelApi(kwargs)
  assert(#kwargs.maps > 0, 'This factory requires at least 1 map.')
  assert(kwargs.episodeLengthSeconds > 0,
      'Episode length must be greater than 0s.')
  local api = {}

  function api:start(episode, seed)
    random:seed(seed)
    randomMap:seed(seed)
  end

  function api:createModel(modelName)
    if modelName == 'mushroom_reward' then
      local mushroom = model:loadMD3(LOAD_MD3)
      return model:hierarchy{
          transform = transform.translate{0, 0, -PICKUP_VERTICAL_OFFSET},
          model = mushroom
      }
    end
  end

  function api:createPickup(classname)
    return PICKUPS[classname] or pickups.defaults[classname]
  end

  function api:nextMap()
    return kwargs.maps[random:uniformInt(1, #kwargs.maps)]
  end

  local function updateOrigin(originString)
    local location = {}
    for v in originString:gmatch("[0-9.-]+") do
      location[#location + 1] = tonumber(v)
    end
    location[3] = location[3] + PICKUP_VERTICAL_OFFSET
    return location[1] .. ' ' .. location[2] .. ' ' .. location[3]
  end

  function api:updateSpawnVars(spawnVars)
    if spawnVars.classname == "frag_mushroom_desert_01a" then
      spawnVars.origin = updateOrigin(spawnVars.origin)
    end
    return spawnVars
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
