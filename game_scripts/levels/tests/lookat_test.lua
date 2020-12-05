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
local tensor = require 'dmlab.system.tensor'
local random = require 'common.random'
local pickups = require 'common.pickups'
local custom_observations = require 'decorators.custom_observations'

local api = {}
local lookAt = tensor.Tensor(4)

function api:createPickup(classname)
  return pickups.defaults[classname]
end

function api:start(episode, seed)
  random:seed(seed)
end

function api:nextMap()
  return 'lookat_test'
end

function api:customObservationSpec()
  return {
      {name = 'LOOK_AT', type = 'Doubles', shape = lookAt:shape()},
  }
end

function api:customObservation(name)
  if name == 'LOOK_AT' then
    return lookAt
  end
end

function api:lookat(entity, lookedAt, position)
  lookAt(1):val(position[1])
  lookAt(2):val(position[2])
  lookAt(3):val(position[3])
  lookAt(4):val(lookedAt and 1.0 or 0.0)
end

custom_observations.decorate(api)

return api
