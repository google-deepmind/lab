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

local tensor = require 'dmlab.system.tensor'
local game = require 'dmlab.system.game'
local custom_observations = {}
local obs = {}
local obsSpec = {}

function custom_observations.add_spec(name, type, shape, callback)
  obsSpec[#obsSpec + 1] = {name = name, type = type, shape = shape}
  obs[name] = callback
end

local function velocity()
  local info = game:playerInfo()
  local a = info.angles[2] / 180.0 * math.pi
  local s = math.sin(a)
  local c = math.cos(a)
  local velx = info.vel[1] * c + info.vel[2] * s
  local vely = -info.vel[1] * s + info.vel[2] * c
  return tensor.DoubleTensor{velx, vely, info.vel[3]}
end

local function angularVelocity()
  return tensor.DoubleTensor(game:playerInfo().anglesVel)
end

-- Decorate the api with a player translation velocity and angular velocity
-- observation. These observations are relative to the player.
function custom_observations.decorate(api)
  local init = api.init
  function api:init(params)
    custom_observations.add_spec('VEL.TRANS', 'Doubles', {3}, velocity)
    custom_observations.add_spec('VEL.ROT', 'Doubles', {3}, angularVelocity)
    return init and init(params)
  end

  local customObservationSpec = api.customObservationSpec
  function api:customObservationSpec()
    local specs = customObservationSpec and customObservationSpec(api) or {}
    for i, spec in ipairs(obsSpec) do
      specs[#specs + 1] = spec
    end
    return specs
  end

  local customObservation = api.customObservation
  function api:customObservation(name)
    return obs[name] and obs[name]() or customObservation(api, name)
  end
end

return custom_observations
