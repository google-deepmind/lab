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

local events = require 'dmlab.system.events'
local game = require 'dmlab.system.game'
local tensor = require 'dmlab.system.tensor'
local inventory = require 'common.inventory'

local api = {}

local map = 'seekavoid_arena_01'

function api:nextMap()
  local result = map
  map = ''
  return result
end

function api:spawnInventory(loadout)
  local view = inventory.View(loadout)
  view:setGadgets{
      inventory.GADGETS.IMPULSE,
      inventory.GADGETS.RAPID,
      inventory.GADGETS.BEAM,
      inventory.GADGETS.DISC,
  }
  view:setGadgetAmount(inventory.GADGETS.IMPULSE, inventory.UNLIMITED)
  view:setGadgetAmount(inventory.GADGETS.RAPID, inventory.UNLIMITED)
  view:setGadgetAmount(inventory.GADGETS.BEAM, inventory.UNLIMITED)
  view:setGadgetAmount(inventory.GADGETS.DISC, inventory.UNLIMITED)
  return view:loadOut()
end

function api:updateInventory(loadout)
  local view = inventory.View(loadout)
  local gadget = view:gadget()
  for k, i in pairs(inventory.GADGETS) do
    if i == gadget then
      events:add('GADGET', k)
      return
    end
  end
  events:add('GADGET', tostring(gadget))
end

function api:customDiscreteActionSpec()
  return {{name = 'NEXT_GADGET', min = 0, max = 1}}
end

function api:customDiscreteActions(actions)
  if actions[1] ~= 0 then
    game:console('weapnext')
  end
end

function api:hasEpisodeFinished(time)
  return false
end

return api
