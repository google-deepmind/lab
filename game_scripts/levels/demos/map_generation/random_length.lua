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

-- Demonstration of using text to create a different level each time the player
-- reaches a goal object.

local random = require 'common.random'
local make_map = require 'common.make_map'
local pickups = require 'common.pickups'
local api = {}

-- `make_map` has default pickup types A = apple_reward and G = goal.
-- This callback is used to create pickups with those names.
function api:createPickup(className)
  return pickups.defaults[className]
end

-- Called at the begining of each episode.
function api:start(episode, seed)
  random:seed(seed)
  -- When converting from text to map the theme variations, wall decorations and
  -- light positions are chosen randomly.
  make_map.random():seed(random:mapGenerationSeed())
end

-- Called each time a map is needed to be generated.
-- The first map is generated after start is called and the following each time
-- the goal object is picked up.
function api:nextMap()

  -- All maps must have at least one spawn point.
  local map = 'P'

  -- Add between [0, 5] apples.
  for i = 0, make_map.random():uniformInt(0, 5) do
    map = map .. ' A'
  end

  -- Add a door and a goal object.
  -- When reaching the goal object the map is finished and a new map is
  -- requested.
  map = map .. ' I G'
  return make_map.makeMap{
      mapName = 'random_length',
      mapEntityLayer = map
  }
end

return api
