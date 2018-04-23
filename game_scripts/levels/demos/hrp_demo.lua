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
local random = require 'common.random'
local hrp = require 'common.human_recognisable_pickups'

local custom_observations = require 'decorators.custom_observations'
local pickup_decorator = require 'decorators.human_recognisable_pickups'

local api = {}

function api:updateSpawnVars(spawnVars)
  if spawnVars.classname == 'info_player_start' then
    spawnVars.angle = '0'
    spawnVars.randomAngleRange = '0'
  elseif spawnVars.classname == self._cherries then
    -- If there's no id set, canPickup won't be called.
    spawnVars.id = '1'
  end
  return spawnVars
end

function api:canPickup(id)
  -- Turn off pickups so you can get up close and personal.
  return false
end

-- Sets all objects in the map to have the class name 'recognisable_object',
-- which we'll customise in api:createPickup().
function api:nextMap()
  hrp.reset()

  local map = 'PO'
  self._cherries = hrp.create{
      shape = "cherries",
      color1 = {255, 0, 0},
      color2 = {0, 255, 0},
      pattern = "chequered",
  }
  return make_map.makeMap{
      mapName = 'hrpdemo_map',
      mapEntityLayer = map,
      useSkybox = true,
      pickups = {O = self._cherries}
  }
end

custom_observations.decorate(api)
pickup_decorator.decorate(api)

return api
