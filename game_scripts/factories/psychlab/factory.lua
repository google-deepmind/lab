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
local setting_overrides = require 'decorators.setting_overrides'
local debug_observations = require 'decorators.debug_observations'
local game = require 'dmlab.system.game'
local tensor = require 'dmlab.system.tensor'
local timeout = require 'decorators.timeout'
local random = require 'common.random'
local map_maker = require 'dmlab.system.map_maker'
local randomMap = random(map_maker:randomGen())

local TEXTURE_NAME = 'textures/map/slot1'

-- This is the common factory for all PsychLab levels in DM Lab.
local factory = {}

function factory.createLevelApi(kwargs)
  assert(kwargs.env, 'Must provide environment')
  kwargs.envOpts = kwargs.envOpts or {}
  kwargs.envOpts.episodeLengthSeconds = kwargs.episodeLengthSeconds or 300
  kwargs.envOpts.camera = kwargs.envOpts.camera or {0, -30, 150}
  kwargs.envOpts.cameraLook = kwargs.envOpts.cameraLook or {2, 90, 0}

  local api = {
      _mouseDown = nil,
      _pcontinue = 1
  }
  api.opts = kwargs.envOpts

  function string.starts(String, Start)
   return string.sub(String, 1, string.len(Start)) == Start
  end

  function api:init(params)
    -- Option to disable idle timeout, false for test runs.
    api.opts.timeoutIfIdle = params.allowHoldOutLevels == 'true' or
                             params.invocationMode == 'testbed'  -- Legacy
  end

  function api:loadTexture(textureName)
    if textureName == TEXTURE_NAME then
      local shape = api._screen:shape()
      api.big_screen = tensor.ByteTensor(shape[1], shape[2], 4)
      return api.big_screen
    end
  end

  function api:modifyControl(actions)
    local click = false
    local clickUp = false
    local mouseDown = actions.buttonsDown
    if self._mouseDown ~= mouseDown then
      click = mouseDown ~= 0
      clickUp = not click
      self._mouseDown = mouseDown
    end
    actions.buttonsDown = 0
     -- Prevent movement
    actions.moveBackForward = 0
    actions.strafeLeftRight = 0

    local action = {
        {(click and 1 or 0), (clickUp and 1 or 0)},
        self._pos and {self._pos[1], 1.0 - self._pos[3]} or {-1, -1},
    }
    local screen, reward, screenDirty
    screen, reward, self._pcontinue, screenDirty = self._env:step(action)
    if screenDirty or self._screenDirty then
      game:updateTexture(TEXTURE_NAME, screen)
      self._screenDirty = false
    end

    if reward ~= 0 then
      game:addScore(reward)
    end

    -- Set api:pos to nil so we can tell if api:lookAt() has been
    -- called since the last call to api:modifyControls().
    self._pos = nil
    return actions
  end

  function api:hasEpisodeFinished(timeSeconds)
    return self._pcontinue == 0
  end

  function api:lookat(ent, looked_at, pos)
    -- This function is called if and only if looking at the screen.
    if looked_at then
      self._pos = pos
    end
  end

  function api:start(episode_id, seed)
    random:seed(seed)
    randomMap:seed(random:mapGenerationSeed())
    self._env = kwargs.env(self.opts)
    local screen = self._env:reset(episode_id, seed)
    self._screenDirty = true
    self._screen = screen
  end

  function api:updateSpawnVars(spawnVars)
    if spawnVars.classname == 'info_player_start' then
      spawnVars.angle = '90'
      spawnVars.randomAngleRange = '45'
    end
    return spawnVars
  end

  function api:nextMap()
    return 'big_screen'
  end

  custom_observations.decorate(api)
  setting_overrides.decorate{
      api = api,
      apiParams = kwargs.envOpts,
      decorateWithTimeout = true
  }
  return api
end

return factory
