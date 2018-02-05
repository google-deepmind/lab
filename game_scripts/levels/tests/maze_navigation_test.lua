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
local maze_generation = require 'dmlab.system.maze_generation'
local tensor = require 'dmlab.system.tensor'

local MAP = [[
********
*  * OO*
*  *   *
* P*O  *
* * *  *
*  *   *
********
]]

local api = {}

function api:nextMap()
  local maze = maze_generation:mazeGeneration{entity = MAP}
  debug_observations.setMaze(maze)
  return make_map.makeMap{
      mapName = 'maze',
      mapEntityLayer = MAP,
  }
end

custom_observations.decorate(api)

return api
