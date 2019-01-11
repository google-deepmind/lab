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

function level:restart(maze)
  local height, _ = maze:size()
  -- Select spawn point.
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
  local spawnId = random:uniformInt(1, #spawnLocations)
  local ij = spawnLocations[spawnId]
  level._spawnX, level._spawnY = maze:toWorldPos(ij[1], ij[2])
end

function level:start(maze, episode, seed)
  random:seed(seed)
  randomMap:seed(seed)
  -- Find goals.
  level._goalLocations = {}
  local height, width = maze:size()
  local first = level.opts.overrideGoalStartRow > 0 and
    level.opts.overrideGoalStartRow or 1
  local last = level.opts.overrideGoalEndRow > 0 and
    level.opts.overrideGoalEndRow or height
  assert(first <= last and first + 2 <= height and 2 <= last, "No rows used")
  for i = first, last do
    for j = 1, width do
      local c = maze:getEntityCell(i, j)
      if c == 'G' then
        table.insert(level._goalLocations, {i, j})
      end
    end
  end
  -- Select goals.
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
    end
}

local function createLevelApi(opts)
  opts = opts or {}
  opts.overrideGoalStartRow = opts.overrideGoalStartRow or 0
  opts.overrideGoalEndRow = opts.overrideGoalEndRow or 0
  -- Set base class options.
  opts.objectEntity = 'G'
  level.opts = opts
  return factory.createLevelApi{opts = opts, level = level}
end

return {
    createLevelApi = createLevelApi
}
