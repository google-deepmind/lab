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
local game_entities = require 'dmlab.system.game_entities'
local helpers = require 'common.helpers'
local make_map = require 'common.make_map'
local pickups = require 'common.pickups'
local tensor = require 'dmlab.system.tensor'
local api = {}

function api:createPickup(classname)
  return pickups.defaults[classname]
end

local MAP_ENTITIES = [[PAAALLL]]

function api:updateSpawnVars(spawnVars)
  if spawnVars.classname == "info_player_start" then
    -- Spawn facing East.
    spawnVars.angle = "0"
    spawnVars.randomAngleRange = "0"
  end
  return spawnVars
end

function api:nextMap()
  make_map.seedRng(1)
  api._map = make_map.makeMap{
      mapName = "empty_room",
      mapEntityLayer = MAP_ENTITIES,
      useSkybox = true,
      pickups = {L = 'lemon_reward', A = 'apple_reward'},
  }
  return api._map
end

local PICKUP_CLASSES = {'lemon_reward', 'apple_reward'}

local function observePickups()
  local entities = game_entities:entities(PICKUP_CLASSES)
  local result = tensor.DoubleTensor(#entities, 5)
  for i, entity in pairs(entities) do
    local x, y, z = unpack(entity.position)
    local v = entity.visible and 1 or 0
    local row = result(i)
    row(1):val(x)
    row(2):val(y)
    row(3):val(z)
    row(4):val(v)
    row(5):val(pickups.defaults[entity.classname].quantity)
  end
  return result
end

custom_observations.decorate(api)
custom_observations.addSpec('DEBUG.PICKUPS', 'Doubles', {0, 5}, observePickups)

return api
