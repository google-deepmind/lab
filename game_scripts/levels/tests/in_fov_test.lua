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

local rewards = {}

function api:init(params)
  make_map.seedRng(1)
  self._map = make_map.makeMap{
      mapName = 'empty_room',
      mapEntityLayer = MAP_ENTITIES,
      useSkybox = true
  }
end

function api:nextMap()
  rewards = {}
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
    rewards[#rewards + 1] = helpers.spawnVarToNumberTable(spawnVars.origin)
  end
  return spawnVars
end

local function fovTest()
  local playerInfo = game:playerInfo()
  local result = tensor.ByteTensor(#rewards)
  for i, pos in ipairs(rewards) do
    result(i):val(game:inFov(playerInfo.pos, pos, playerInfo.angles, 120) and 1
      or 0)
  end
  return result
end

local function entityVis()
  local playerInfo = game:playerInfo()
  local result = tensor.DoubleTensor(#rewards)
  for i, pos in ipairs(rewards) do
    result(i):val(game:inFov(playerInfo.pos, pos, playerInfo.angles, 120) and
      game:raycast(playerInfo.pos, pos) or -1)
  end
  return result
end

custom_observations.decorate(api)
custom_observations.addSpec('FOVTESTS', 'Bytes', {0}, fovTest)
custom_observations.addSpec('ENTITYVIS', 'Doubles', {0}, entityVis)

return api
