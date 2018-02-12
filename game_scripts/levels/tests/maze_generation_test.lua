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
local make_map = require 'common.make_map'
local map_maker = require 'dmlab.system.map_maker'
local maze_generation = require 'dmlab.system.maze_generation'
local random = require 'common.random'

local randomMap = random(map_maker:randomGen())

local api = {}

function api:nextMap()
  return api._map
end

function api:start(episode, seed)
  local mapName = 'maze'
  random:seed(seed)
  randomMap:seed(seed)
  api._maze = maze_generation.randomMazeGeneration{
      seed = seed,
      width = 17,
      height = 17,
      maxRooms = 4,
      roomMaxSize = 5,
      roomMinSize = 3,
      extraConnectionProbability = 0.0,
  }
  api._map = make_map.makeMap{
      mapName = mapName,
      mapEntityLayer = api._maze:entityLayer(),
      mapVariationsLayer = api._maze:variationsLayer(),
      useSkybox = true,
  }
  debug_observations.setMaze(api._maze)
end

custom_observations.decorate(api)

return api
