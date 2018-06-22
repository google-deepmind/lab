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
local make_map = require 'common.make_map'
local pickups = require 'common.pickups'
local custom_observations = require 'decorators.custom_observations'
local game = require 'dmlab.system.game'
local timeout = require 'decorators.timeout'
local api = {}

local MAP_ENTITIES = [[
*********
*       *
*       *
*       *
*   P   *
*       *
*       *
*       *
*********
]]

function api:init(params)
  make_map.seedRng(1)
  api._map = make_map.makeMap{
      mapName = 'empty_room',
      mapEntityLayer = MAP_ENTITIES,
      useSkybox = true,
  }
end

function api:nextMap()
  return self._map
end

function api:updateSpawnVars(spawnVars)
  if spawnVars.classname == 'info_player_start' then
    -- Spawn facing East.
    spawnVars.angle = '0'
    spawnVars.randomAngleRange = '0'
  end
  -- This will also print the origins of the extra entities.
  log.info(spawnVars.origin)
  return spawnVars
end

function api:extraEntities()
  -- List of entities to create.
  local vars = {
      {
          classname = 'apple_reward',
          origin = '550 450 30',
      },
      {
          classname = 'apple_reward',
          origin = '600 450 30',
      },
      {
          classname = 'apple_reward',
          origin = '650 450 30',
      },
      {
          classname = 'apple_reward',
          origin = '700 450 30',
      }
  }
  -- `updateSpawnVars` is not called by default; so call it anyway.
  local varsResult = {}
  for i, v in ipairs(vars) do
    varsResult[#varsResult + 1] = self:updateSpawnVars(v)
  end
  return varsResult
end

function api:createPickup(classname)
  if classname == 'apple_reward' then
    return {
        name = 'Apple',
        classname = 'apple_reward',
        model = 'models/apple.md3',
        quantity = 1,
        type = pickups.type.REWARD,
    }
  end
end

timeout.decorate(api, 60 * 60)
custom_observations.decorate(api)
return api
