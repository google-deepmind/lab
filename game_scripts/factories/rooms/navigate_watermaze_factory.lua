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
local game = require 'dmlab.system.game'
local helpers = require 'common.helpers'
local map_maker = require 'dmlab.system.map_maker'
local random = require 'common.random'
local setting_overrides = require 'decorators.setting_overrides'

local randomMap = random(map_maker:randomGen())

local factory = {}

--[[ Creates a WaterMaze level API.

Keyword arguments:

*   `mapName` (string) Name of map file to load with level script.
*   `spawnRadius` (number) Maximum distance from the origin (level centre)
    within which the randomly-placed reward platform will spawn.

Returns a DM Lab level API.
]]
function factory.createLevelApi(kwargs)
  assert(kwargs.mapName)
  assert(kwargs.spawnRadius)
  assert(kwargs.episodeLengthSeconds)

  local api = {}

  function api:start(episode, seed, params)
    random:seed(seed)
    randomMap:seed(seed)

    local alpha = random:uniformReal(0, 2 * math.pi)
    local r = kwargs.spawnRadius * math.sqrt(random:uniformReal(0, 1))
    local x = r * math.cos(alpha)
    local y = r * math.sin(alpha)
    local origin = x .. ' ' .. y .. ' 0'
    api._platformOrigin = origin
  end

  function api:nextMap()
    return kwargs.mapName
  end

  function api:updateSpawnVars(spawnVars)
    api._addedReward = 0
    if spawnVars.random_platform ~= nil then
      spawnVars.origin = api._platformOrigin
    elseif spawnVars.classname == 'info_player_start' then
      spawnVars.spawn_orientation_segment = tostring(20)
      local x, y = unpack(helpers.spawnVarToNumberTable(spawnVars.origin))
      local length = math.sqrt(x * x + y * y)
      spawnVars.angle = tostring(math.deg(math.atan2(-y / length, -x / length)))
    end

    return spawnVars
  end

  function api:canTrigger(spawnId, targetName)
    return true
  end

  function api:trigger(spawnId, targetName)
    if spawnId == 0 and targetName == 'reward' then
      local _platformReward = 1
      local _rewardLimit = 5

      if api._addedReward >= _rewardLimit then
        game:finishMap()
      else
        api._addedReward = api._addedReward + _platformReward
        game:addScore(_platformReward)
      end
    end

    return false
  end

  --[[ Supply player position and velocity deltas.
  When activated, player's position will be set to that of the platform and
  the player's velocity cancelled out, holding them on the platform.
  ]]
  function api:playerMover(kwargs)
    assert(kwargs.moverId == 1)
    return {
        (kwargs.moverPos[1] - kwargs.playerPos[1]),
        (kwargs.moverPos[2] - kwargs.playerPos[2]),
        0.0,
    },
    {
        -kwargs.playerVel[1],
        -kwargs.playerVel[2],
        0.0,
    }
  end

  custom_observations.decorate(api)
  setting_overrides.decorate{
      api = api,
      apiParams = kwargs,
      decorateWithTimeout = true
  }
  return api
end

return factory
