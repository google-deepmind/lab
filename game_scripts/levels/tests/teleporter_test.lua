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

local events = require 'dmlab.system.events'
local game = require 'dmlab.system.game'
local maze_generation = require 'dmlab.system.maze_generation'
local custom_entities = require 'common.custom_entities'
local make_map = require 'common.make_map'
local custom_observations = require 'decorators.custom_observations'
local timeout = require 'decorators.timeout'

local api = {}
local MAP_ENTITIES = '*********\n' ..
                     '* P * t *\n' ..
                     '*   *   *\n' ..
                     '* T *   *\n' ..
                     '*********\n'

function api:init(params)
  make_map.seedRng(1)
  local maze = maze_generation.mazeGeneration{entity = MAP_ENTITIES}
  api._map = make_map.makeMap{
      mapName = "teleport_test",
      mapEntityLayer = MAP_ENTITIES,
      useSkybox = true,
      callback = function (i, j, c, maker)
        if c == 'T' then
          return custom_entities.makeTeleporter(
              {maze:toWorldPos(i + 1, j + 1)},
              'teleporter_dest_second_room')
        end
        if c == 't' then
          return custom_entities.makeTeleporterTarget(
              {maze:toWorldPos(i + 1, j + 1)},
              'teleporter_dest_second_room')
        end
      end
  }
end

function api:hasEpisodeFinished(time)
  if game:playerInfo().teleported then
    events:add("PLAYER_TELEPORTED")
  end
  return false
end

function api:nextMap()
  return self._map
end

function api:updateSpawnVars(spawnVars)
  if spawnVars.classname == "info_player_start" then
    -- Spawn facing South.
    spawnVars.angle = "-90"
    spawnVars.randomAngleRange = "0"
  end
  return spawnVars
end

timeout.decorate(api, 60 * 60)
custom_observations.decorate(api)

return api
