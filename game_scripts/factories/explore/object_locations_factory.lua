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

local factory = require 'factories.explore.factory'
local game = require 'dmlab.system.game'
local helpers = require 'common.helpers'
local pickups = require 'common.pickups'
local random = require 'common.random'
local map_maker = require 'dmlab.system.map_maker'
local randomMap = random(map_maker:randomGen())

local level = {
    _pickupCount = 0,
}

-- Options set in createLevelApi. Keep in global so constants can be accessed.
local opts

function level:pickup(spawnId)
  level._pickupCount = level._pickupCount + 1
  if level._pickupCount == level._rewardCount then
    level._pickupCount = 0
    game:finishMap()
  end
end

function level:restart(maze)
  -- Select spawn point.
  local spawnId = random:uniformInt(1, #level._spawnLocations)
  local ij = level._spawnLocations[spawnId]
  level._spawnX, level._spawnY = maze:toWorldPos(ij[1], ij[2])
end

function level:start(maze, episode, seed)
  random:seed(seed)
  randomMap:seed(seed)
  -- Find spawn points.
  level._spawnLocations = {}
  local rewardLocations = {}
  local height, width = maze:size()
  for i = 1, height do
    for j = 1, width do
      local c = maze:getEntityCell(i, j)
      if c == 'P' then
        table.insert(level._spawnLocations, {i, j})
      elseif c == 'A' then
        local x, y = maze:toWorldPos(i, j)
        local key = y .. '_' .. x
        table.insert(rewardLocations, key)
      end
    end
  end
  -- Select rewards.
  random:shuffleInPlace(rewardLocations)
  level._pickupCount = 0
  level._rewardCount = opts.rewardDensity * #rewardLocations
  level._rewards = {}
  for i = 1, level._rewardCount do
    level._rewards[rewardLocations[i]] = i
  end
end

level.spawnVarsUpdater = {
    ['info_player_start'] = function(spawnVars)
      local x, y = unpack(helpers.spawnVarToNumberTable(spawnVars.origin))
      if x == level._spawnX and y == level._spawnY then
        return spawnVars
      else
        return nil
      end
    end,
    ['apple_reward'] = function(spawnVars)
      local j, i = unpack(helpers.spawnVarToNumberTable(spawnVars.origin))
      local key = i .. '_' .. j
      local rewardId = level._rewards[key]
      if rewardId then
        spawnVars.id = tostring(rewardId)
        spawnVars.wait = '-1'
        return spawnVars
      else
        return nil
      end
    end
}

local function createLevelApi(optsOverride)
  opts = optsOverride or {}

  opts.rewardDensity = opts.rewardDensity or 0.5

  -- Set base class options.
  opts.objectEntity = opts.objectEntity or 'A'
  return factory.createLevelApi{opts = opts, level = level}
end

return {
    createLevelApi = createLevelApi
}
