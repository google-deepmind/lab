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

local debug_observations = require 'decorators.debug_observations'
local game = require 'dmlab.system.game'
local inventory = require 'common.inventory'
local timeout = require 'decorators.timeout'
local tensor = require 'dmlab.system.tensor'

local obs = {}
local obsSpec = {}
local instructionObservation = ''

local custom_observations = {}

custom_observations.playerNames = {''}
custom_observations.playerInventory = {}
custom_observations.playerTeams = {}

function custom_observations.addSpec(name, type, shape, callback)
  -- Only add spec if not already present.
  if obs[name] == nil then
    obsSpec[#obsSpec + 1] = {name = name, type = type, shape = shape}
    obs[name] = callback
  end
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

local function languageChannel()
  return instructionObservation or ''
end

local function teamScore()
  local info = game:playerInfo()
  return tensor.DoubleTensor{info.teamScore, info.otherTeamScore}
end

local framesRemainingTensor = tensor.DoubleTensor(1)
local function framesRemainingAt60()
  framesRemainingTensor:val(timeout.timeRemainingSeconds() * 60)
  return framesRemainingTensor
end

--[[ Decorate the api to support custom observations:

1.  Player translational velocity (VEL.TRANS).
2.  Player angular velocity (VEL.ROT).
3.  Language channel for, e.g. giving instructions to the agent (INSTR).
4.  See debug_observations.lua for those.
]]
function custom_observations.decorate(api)
  local init = api.init
  function api:init(params)
    custom_observations.addSpec('VEL.TRANS', 'Doubles', {3}, velocity)
    custom_observations.addSpec('VEL.ROT', 'Doubles', {3}, angularVelocity)
    custom_observations.addSpec('INSTR', 'String', {0}, languageChannel)
    custom_observations.addSpec('TEAM.SCORE', 'Doubles', {2}, teamScore)
    custom_observations.addSpec('FRAMES_REMAINING_AT_60', 'Doubles', {1},
                                framesRemainingAt60)
    api.setInstruction('')
    debug_observations.extend(custom_observations)
    if params.enableCameraMovement == 'true' then
      debug_observations.enableCameraMovement(api)
    end
    return init and init(api, params)
  end

  local customObservationSpec = api.customObservationSpec
  function api:customObservationSpec()
    local specs = customObservationSpec and customObservationSpec(api) or {}
    for i, spec in ipairs(obsSpec) do
      specs[#specs + 1] = spec
    end
    return specs
  end

  local team = api.team
  function api:team(playerId, playerName)
    custom_observations.playerNames[playerId] = playerName
    local result = team and team(self, playerId, playerName) or 'p'
    custom_observations.playerTeams[playerId] = result
    return result
  end

  local spawnInventory = api.spawnInventory
  function api:spawnInventory(loadOut)
    local view = inventory.View(loadOut)
    custom_observations.playerInventory[view:playerId()] = view
    return spawnInventory and spawnInventory(self, loadOut)
  end

  local updateInventory = api.updateInventory
  function api:updateInventory(loadOut)
    local view = inventory.View(loadOut)
    custom_observations.playerInventory[view:playerId()] = view
    return updateInventory and updateInventory(self, loadOut)
  end

  local customObservation = api.customObservation
  function api:customObservation(name)
    return obs[name] and obs[name]() or customObservation(api, name)
  end

  -- Levels can call this to define the language channel observation string
  -- returned to the agent.
  function api.setInstruction(text)
    instructionObservation = text
  end
end

return custom_observations
