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
local make_map = require 'common.make_map'
local inventory = require 'common.inventory'
local custom_observations = require 'decorators.custom_observations'
local timeout = require 'decorators.timeout'
local tensor = require 'dmlab.system.tensor'
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
  self._playerView = {}
  make_map.seedRng(1)
  api._map = make_map.makeMap{
      mapName = "empty_room",
      mapEntityLayer = MAP_ENTITIES,
      useSkybox = true,
  }
end

function api:nextMap()
  return self._map
end

function api:customObservationSpec()
  return {
      {name = 'DEBUG.AMOUNT', type = 'Doubles', shape = {1}},
      {name = 'DEBUG.GADGET', type = 'Doubles', shape = {1}},
  }
end

function api:customObservation(name)
  local view = self._playerView[1]
  if name == 'DEBUG.AMOUNT' then
    return tensor.Tensor{view:gadgetAmount(view:gadget())}
  elseif name == 'DEBUG.GADGET' then
    return tensor.Tensor{view:gadget()}
  end
end

function api:updateSpawnVars(spawnVars)
  if spawnVars.classname == "info_player_start" then
    -- Spawn facing East.
    spawnVars.angle = "0"
    spawnVars.randomAngleRange = "0"
  end
  return spawnVars
end

function api:spawnInventory(loadOut)
  local view = inventory.View(loadOut)
  view:setGadgets{inventory.GADGETS.ORB, inventory.GADGETS.RAPID}
  view:setGadgetAmount(inventory.GADGETS.ORB, 2)
  view:setGadgetAmount(inventory.GADGETS.RAPID, 10)
  self._playerView[view:playerId()] = view
  return view:loadOut()
end

function api:updateInventory(loadOut)
  local view = inventory.View(loadOut)
  self._playerView[view:playerId()] = view
end

timeout.decorate(api, 60 * 60)
custom_observations.decorate(api)

return api
