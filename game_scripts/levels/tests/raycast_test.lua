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

local make_map = require 'common.make_map'
local pickups = require 'common.pickups'
local custom_observations = require 'decorators.custom_observations'
local game = require 'dmlab.system.game'
local tensor = require 'dmlab.system.tensor'
local helpers = require 'common.helpers'
local api = {}

local MAP_ENTITIES = [[
*****
*A A*
* * *
*P*A*
*****
]]


function api:init(params)
  make_map.seedRng(1)
  self._map = make_map.makeMap{
      mapName = 'empty_room',
      mapEntityLayer = MAP_ENTITIES,
      useSkybox = true,
  }
  self.rewards = {}
end

function api:nextMap()
  return self._map
end

function api:createPickup(classname)
  return pickups.defaults[classname]
end

function api:updateSpawnVars(spawnVars)
  if spawnVars.classname == 'info_player_start' then
    -- Spawn facing north.
    spawnVars.angle = '90'
    spawnVars.randomAngleRange = '0'
  else
    self.rewards[#self.rewards + 1] =
      helpers.spawnVarToNumberTable(spawnVars.origin)
  end
  return spawnVars
end

function api:customObservationSpec()
  return {{name = 'RAYCASTS', type = 'Doubles', shape = {0}}}
end

function api:customObservation(name)
  assert(name == 'RAYCASTS', 'Bad observation name')
  local playerInfo = game:playerInfo()
  local result = tensor.DoubleTensor(#self.rewards)
  for i, pos in ipairs(self.rewards) do
    result(i):val(game:raycast(playerInfo.pos, pos))
  end
  return result
end

custom_observations.decorate(api)

return api
