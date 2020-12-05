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
local debug_observations = require 'decorators.debug_observations'
local gen = require 'map_generators.platforms_v1.gen'
local log = require 'common.log'
local helpers = require 'common.helpers'
local map_maker = require 'dmlab.system.map_maker'
local maze_generation = require 'dmlab.system.maze_generation'
local pickups = require 'common.pickups'
local random = require 'common.random'
local setting_overrides = require 'decorators.setting_overrides'
local tensor = require 'dmlab.system.tensor'
local texture_sets = require 'themes.texture_sets'
local themes = require 'themes.themes'

local randomMap = random(map_maker:randomGen())

local MAX_HEIGHT = 40
local PICKUP_HEIGHT = 4

local MAP_NAME = 'platforms_map'
local DEFAULT_SKYBOX = 'map/lab_games/sky/lg_sky_03'

local GOAL = {
    name = 'Goal',
    classname = 'goal',
    model = 'models/goal_object_03.md3', -- use large model
    quantity = 100,
    type = pickups.type.REWARD
}
local GOAL_ID = -1

local factory = {}

function factory.createLevelApi(kwargs)
  -- Use `false` to represent values that are unspecified.
  kwargs.cellSize = kwargs.cellSize or false
  kwargs.episodeLengthSeconds = kwargs.episodeLengthSeconds or 60
  kwargs.goalHeight = kwargs.goalHeight or 1
  kwargs.difficulty = kwargs.difficulty or false
  kwargs.levelMap = kwargs.levelMap or false
  kwargs.skyboxTextureName = kwargs.skyboxTextureName or DEFAULT_SKYBOX
  if kwargs.useInvisibleColumns == nil then
    kwargs.useInvisibleColumns = true
  end

  local api = {}

  function api:start(episode, seed)
    random:seed(seed)
    randomMap:seed(random:mapGenerationSeed())
    if not kwargs.levelMap then
      log.info('Generating levelMap with from seed ' .. seed)
    end
    local levelMap = kwargs.levelMap or
                     gen.makeLevel(kwargs.difficulty, randomMap)
    api._asciiMap = levelMap.map
    log.info('\n' .. api._asciiMap)

    local maze = maze_generation:mazeGeneration{entity = api._asciiMap}
    debug_observations.setMaze(maze)

    api._player = levelMap.playerPosition
    api._goal = levelMap.goalPosition
    api._timeOut = nil
    local a, z, dot = string.byte('a'), string.byte('z'), string.byte('.')
    map_maker:mapFromTextLevel{
        entityLayer = api._asciiMap,
        mapName = MAP_NAME,
        skyboxTextureName = kwargs.skyboxTextureName,
        ceilingHeight = 10.0,
        drawDefaultLayout = false,
        theme = themes.fromTextureSet{textureSet = texture_sets.MISHMASH},
        cellSize = kwargs.cellSize,
        callback = function (i, j, c_string, maker)
          local c = c_string:byte()
          if i == api._player.x and j == api._player.y then
            assert(c >= a and c <= z, "Player Spawn point is invalid!")
          end
          if c == dot then
            if kwargs.useInvisibleColumns then
              maker:addGlassColumn{
                  i = i,
                  j = j,
                  height = MAX_HEIGHT + 2
              }
            end
          end
          if c >= a and c <= z then
            local height = MAX_HEIGHT - (c - a)
            maker:addPlatform{
                i = i,
                j = j,
                height = height
            }
            if kwargs.useInvisibleColumns then
              maker:addGlassColumn{
                  i = i,
                  j = j,
                  height = height - 1
              }
            end
            if i == api._player.x and j == api._player.y then
              return maker:makeSpawnPoint{
                  i = i,
                  j = j,
                  height = height
              }
            end
            if i == api._goal.x and j == api._goal.y then
              c = ''
              for n = 1, kwargs.goalHeight do
                 c = c .. '\n\n' .. maker:makeEntity{
                     i = i,
                     j = j,
                     height = height + (n - 1) * PICKUP_HEIGHT,
                     classname = 'goal',
                 }
              end
              return c
            end
          end
        end
    }
    return MAP_NAME
  end

  function api:nextMap()
    return MAP_NAME
  end

  function api:createPickup(classname)
    if classname == 'goal' then
      return GOAL
    end
  end

  function api:pickup(entity_id)
    if entity_id == GOAL_ID then
      api._timeOut = api._time + 0.2
    end
  end

  function api:updateSpawnVars(spawnVars)
    local classname = spawnVars.classname
    if classname == 'goal' then
      spawnVars.id = tostring(GOAL_ID)
      spawnVars.wait = "-1"
      spawnVars.spawnflags = "1"
    end
    return spawnVars
  end

  function api:modifyControl(actions)
    actions.crouchJump = 0
    actions.buttonsDown = 0
    return actions
  end

  function api:hasEpisodeFinished(timeSeconds)
    api._time = timeSeconds
    return api._timeOut ~= nil and timeSeconds > api._timeOut
  end

  custom_observations.decorate(api)
  setting_overrides.decorate{
      api = api,
      apiParams = kwargs,
      decorateWithTimeout = true
  }

  local function goalPosition()
    return tensor.DoubleTensor({api._goal.y, api._goal.x})
  end

  custom_observations.addSpec(
    'DEBUG.SKYMAZE.GOAL_POSITION', 'Doubles', {2}, goalPosition)

  return api
end

return factory
