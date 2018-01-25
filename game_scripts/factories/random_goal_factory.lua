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
local map_maker = require 'dmlab.system.map_maker'
local maze_generation = require 'dmlab.system.maze_generation'
local helpers = require 'common.helpers'
local pickups = require 'common.pickups'
local custom_observations = require 'decorators.custom_observations'
local setting_overrides = require 'decorators.setting_overrides'
local random = require 'common.random'
local map_maker = require 'dmlab.system.map_maker'
local randomMap = random(map_maker:randomGen())

local factory = {}

--[[ Creates a Nav Maze Random Goal.
Keyword arguments:

*   `mapName` (string) - Name of map to load.
*   `entityLayer` (string) - Text representation of the maze.
*   `episodeLengthSeconds` (number, default 600) - Episode length in seconds.
*   `scatteredRewardDensity` (number, default 0.1) - Density of rewards.
]]

function factory.createLevelApi(kwargs)
  kwargs.scatteredRewardDensity = kwargs.scatteredRewardDensity or 0.1
  kwargs.episodeLengthSeconds = kwargs.episodeLengthSeconds or 600
  local maze = maze_generation.mazeGeneration{entity = kwargs.entityLayer}
  local api = {}

  function api:createPickup(class)
    return pickups.defaults[class]
  end

  function api:start(episode, seed, params)
    random:seed(seed)
    randomMap:seed(seed)
    api._timeRemaining = kwargs.episodeLengthSeconds
    local height, width = maze:size()
    height = (height - 1) / 2
    width = (width - 1) / 2

    api._goal = {random:uniformInt(1, height) * 2,
                 random:uniformInt(1, width) * 2}

    local goalLocation
    local allSpawnLocations = {}
    local fruitLocations = {}
    maze:visitFill{cell = api._goal, func = function(row, col, distance)
      if row % 2 == 1 or col % 2 == 1 then
        return
      end
      row = row / 2 - 1
      col = col / 2 - 1
      -- Axis is flipped in DeepMind Lab.
      row = height - row - 1
      local key = '' .. (col * 100 + 50) .. ' ' .. (row * 100 + 50) .. ' '

      if distance == 0 then
        goalLocation = key .. '20'
      end
      if distance > 0 then
        fruitLocations[#fruitLocations + 1] = key .. '20'
      end
      if distance > 8 then
        allSpawnLocations[#allSpawnLocations + 1] = key .. '30'
      end
    end}
    random:shuffleInPlace(fruitLocations)
    api._goalLocation = goalLocation
    api._fruitLocations = fruitLocations
    api._allSpawnLocations = allSpawnLocations
  end

  function api:updateSpawnVars(spawnVars)
    local classname = spawnVars.classname
    if classname == 'apple_reward' then
      return api._newSpawnVars[spawnVars.origin]
    elseif classname == 'info_player_start' then
      return api._newSpawnVarsPlayerStart
    end
    return spawnVars
  end

  function api:nextMap()
    api._newSpawnVars = {}

    local maxFruit = math.floor(kwargs.scatteredRewardDensity *
                                #api._fruitLocations + 0.5)
    for i, fruitLocation in ipairs(api._fruitLocations) do
      if i > maxFruit then
        break
      end
      api._newSpawnVars[fruitLocation] = {
          classname = 'apple_reward',
          origin = fruitLocation
      }
    end

    local spawnLocation = api._allSpawnLocations[
                                 random:uniformInt(1, #api._allSpawnLocations)]
    api._newSpawnVarsPlayerStart = {
        classname = 'info_player_start',
        origin = spawnLocation
    }

    api._newSpawnVars[api._goalLocation] = {
        classname = 'goal',
        origin = api._goalLocation
    }
    -- Fast map restarts.
    local map = kwargs.mapName
    kwargs.mapName = ''
    return map
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
