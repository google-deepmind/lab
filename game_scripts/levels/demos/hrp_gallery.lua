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

local timeout = require 'decorators.timeout'

local SCALES = {"small", "medium", "large"}
local MOVE_TYPE = {pickups.moveType.STATIC, pickups.moveType.BOB}

local api = {}

function api:init(settings)
  make_map.seedRng(1)  -- Use a fixed seed since this is a simple demo level.
  random:seed(1)
  self._shapes = hrp.shapes()
  table.sort(self._shapes)
  self._patterns = hrp.patterns()
  table.sort(self._patterns)
  self._map = self:_makeMap()
end

function api:nextMap()
  return self._map
end

function api:updateSpawnVars(spawnVars)
  if spawnVars.classname == 'info_player_start' then
    -- Spawn looking along the row of objects
    spawnVars.angle = '0'
    spawnVars.randomAngleRange = '0'
  else
    local id = self._classnameToId[spawnVars.classname] or 1
    spawnVars.id = tostring(id)
  end
  return spawnVars
end

function api:canPickup(id)
  -- Enable pickups so we can update the instruction channel.
  return true
end

function api:pickup(id)
  local shape = self._shapes[id]
  if shape then self.setInstruction(shape) end
  -- Respawn instantly.
  return 0
end

function api:_makePickup(c)
  if c == 'O' then
    -- Make new pickup and classname to reference it.
    local id = hrp.pickupCount() + 1
    local classname = hrp.create{
        shape = self._shapes[id],
        color1 = random:color(),
        color2 = random:color(),
        pattern = self._patterns[(id - 1) % #self._patterns + 1],
        scale = SCALES[(id - 1) % #SCALES + 1],
        moveType = MOVE_TYPE[(id - 1) % #MOVE_TYPE + 1],
    }
    self._classnameToId[classname] = id
    return classname
  end
end

function api:_makeMap()
  hrp.reset()
  self._classnameToId = {}

  -- Repeat 'O' enough time to show each shape once, add spawn on next row.
  local map = ' ' .. string.rep(' ', #self._shapes) .. ' \n' ..
              ' ' .. string.rep('O', #self._shapes) .. ' \n' ..
              'P' .. string.rep(' ', #self._shapes) .. ' '
  local var = ' ' .. string.rep(' ', #self._shapes) .. ' \n' ..
              ' ' .. string.rep('AB', #self._shapes / 2) .. ' \n' ..
              ' ' .. string.rep(' ', #self._shapes) .. ' '

  return make_map.makeMap{
      mapName = 'hrpgallery_map',
      mapEntityLayer = map,
      mapVariationsLayer = var,
      useSkybox = true,
      callback = function (i, j, c, maker)
        local pickup = self:_makePickup(c)
        if pickup then
          return maker:makeEntity{i = i, j = j, classname = pickup}
        end
      end,
  }
end

custom_observations.decorate(api)
pickup_decorator.decorate(api)
timeout.decorate(api, 60 * 60) -- 60 minutes.

return api
