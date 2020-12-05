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
local factory = require 'factories.lasertag.factory'
local make_map = require 'common.make_map'
local maze_generation = require 'dmlab.system.maze_generation'
local tensor = require 'dmlab.system.tensor'

local MAP = [[
********
*  *P  *
*P     *
********
]]

local api = factory.createLevelApi{
    episodeLengthSeconds = 60 * 5,
    botCount = 1
}

local spawnCount = 1
function api:updateSpawnVars(spawnVars)
  if spawnVars.classname == 'info_player_start' then
    -- Spawn facing East.
    spawnVars.angle = '0'
    spawnVars.randomAngleRange = '0'
    -- Make Bot spawn on first 'P' and player on second 'P'.
    spawnVars.nohumans = spawnCount == 1 and '1' or '0'
    spawnVars.nobots = spawnCount == 2 and '1' or '0'
    spawnCount = spawnCount + 1
  end
  return spawnVars
end

function api:nextMap()
  local maze = maze_generation:mazeGeneration{entity = MAP}
  debug_observations.setMaze(maze)
  -- Override default camera position.
  debug_observations.setCameraPos{395, 220, 400}
  return make_map.makeMap{
      mapName = 'empty_room',
      mapEntityLayer = MAP,
      allowBots = true,
  }
end

return api
