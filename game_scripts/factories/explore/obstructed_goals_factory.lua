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

local level = {}

-- Options set in createLevelApi. Keep in global so constants can be accessed.
local opts

function level:restart(maze)
  -- Find all spawn points reachable from the goal.
  local spawnLocations = {}
  maze:visitFill{
      cell = level._goalLocations[level._goalId],
      func = function(i, j)
        local c = maze:getEntityCell(i, j)
        if c == 'P' then
          table.insert(spawnLocations, {i, j})
        end
      end
  }
  -- Select goal.
  local spawnId = random:uniformInt(1, #spawnLocations)
  local ij = spawnLocations[spawnId]
  level._spawnX, level._spawnY = maze:toWorldPos(ij[1], ij[2])
  -- Reset all doors to closed.
  for k, _ in pairs(level._doors) do
    level._doors[k] = false
  end
  -- Open the doors found in a random path from spawn point to goal.
  maze:visitRandomPath{
      seed = random:uniformInt(1, math.pow(2, 31)),
      to = spawnLocations[spawnId],
      from = level._goalLocations[level._goalId],
      func = function(i, j)
        local c = maze:getEntityCell(i, j)
        if c == 'H' or c == 'I' then
          local height = maze:size()
          local key = 'door_' .. (j - 1) .. '_' .. (height - i)
          level._doors[key] = true
        end
      end
  }
  -- Randomly open other closed doors.
  local closedDoorCount = 0
  local extraDoors = {}
  for k, v in pairs(level._doors) do
    if not v then
      table.insert(extraDoors, k)
      closedDoorCount = closedDoorCount + 1
    end
  end
  local extraDoorCount = closedDoorCount * (1.0 - opts.doorsClosed)
  random:shuffleInPlace(extraDoors)
  for i = 1, math.floor(extraDoorCount) do
    level._doors[extraDoors[i]] = true
  end
end

function level:start(maze, episode, seed)
  random:seed(seed)
  randomMap:seed(random:mapGenerationSeed())
  -- Find goals and doors.
  level._goalLocations = {}
  level._doors = {}
  local height, width = maze:size()
  for i = 1, height do
    for j = 1, width do
      local c = maze:getEntityCell(i, j)
      if c == 'G' then
        table.insert(level._goalLocations, {i, j})
      elseif c == 'H' or c == 'I' then
        local key = 'door_' .. (j - 1) .. '_' .. (height - i)
        level._doors[key] = false
      end
    end
  end
  -- Select goal.
  level._goalId = random:uniformInt(1, #level._goalLocations)
  local ij = level._goalLocations[level._goalId]
  level._goalX, level._goalY = maze:toWorldPos(ij[1], ij[2])
end

level.spawnVarsUpdater = {
    info_player_start = function(spawnVars)
      local x, y = unpack(helpers.spawnVarToNumberTable(spawnVars.origin))
      if x == level._spawnX and y == level._spawnY then
        return spawnVars
      else
        return nil
      end
    end,
    goal = function(spawnVars)
      local x, y = unpack(helpers.spawnVarToNumberTable(spawnVars.origin))
      if x == level._goalX and y == level._goalY then
        spawnVars.id = tostring(level._goalId)
        return spawnVars
      else
        return nil
      end
    end,
    func_door = function(spawnVars)
      spawnVars.wait = "-1"
      if level._doors[spawnVars.targetname] then
        spawnVars.spawnflags = '1'
      else
        spawnVars.spawnflags = '0'
      end
      return spawnVars
    end,
    trigger_multiple = function(spawnVars)
      return nil  -- doors are permanently closed
    end
}

local function createLevelApi(optsOverride)
  opts = optsOverride or {}

  -- Set defaults if not already set.
  opts.doorsClosed = opts.doorsClosed or 0.5

  -- Set base class options.
  opts.objectEntity = 'G'
  opts.hasDoors = true

  return factory.createLevelApi{opts = opts, level = level}
end

return {
    createLevelApi = createLevelApi
}
