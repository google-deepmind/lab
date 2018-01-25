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

local api = {}

function api:start(episode, seed)
  make_map.seedRng(0)  -- Use a fixed seed since this is a simple demo level.
  api._count = 0
end

function api:createPickup(className)
  return pickups.defaults[className]
end

function api:updateSpawnVars(spawnVars)
  if spawnVars.classname == 'info_player_start' then
    -- Spawn facing the apples.
    spawnVars.angle = '0'
    spawnVars.randomAngleRange = '0'
  end
  return spawnVars
end

function api:nextMap()
  api._count = api._count + 1
  local map = 'P'
  for i = 1, api._count do
    map = map .. 'A'
  end
  map = map .. 'G'
  local str = api._count == 1 and ' apple' or ' apples'
  api.setInstruction(api._count .. str)
  return make_map.makeMap{
      mapName = 'instruction_map',
      mapEntityLayer = map
  }
end

custom_observations.decorate(api)

return api
