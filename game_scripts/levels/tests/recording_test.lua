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
local make_map = require 'common.make_map'
local pickups = require 'common.pickups'

local api = {}

function api:start(episode, seed)
  make_map.seedRng(seed)
  api._count = 0
end

function api:createPickup(classname)
  return pickups.defaults[classname]
end

function api:nextMap()
  api._count = api._count + 1

  return make_map.makeMap{
      mapName = 'recording_test',
      mapEntityLayer = string.rep('A', api._count) .. 'P'
  }
end

function api:updateSpawnVars(spawnVars)
  if spawnVars.classname == 'info_player_start' then
    -- Spawn facing south.
    spawnVars.angle = '180'
    spawnVars.randomAngleRange = '0'
  end
  return spawnVars
end

function api:hasEpisodeFinished(time_seconds)
  -- Each map lasts one second.
  if time_seconds >= api._count then
    game:finishMap()
  end
  return false
end

return api
