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

local factory = require 'factories.language.factory'
local object_generator = require 'language.object_generator'
local lcm = require 'language.make'
local placers = require 'language.placers'
local selectors = require 'language.selectors'
local reward_controllers = require 'language.reward_controllers'

local fourObjectMap = {
    name = 'fourPositions',
    entityLayer = [[
..O.O
P....
..O.O
]],
}

-- For the easy level, only select from 5 different shapes.
-- Limit to medium sized objects only. By default, it chooses randomly between
-- small, medium, and large.
local objectContext = object_generator.createContext{
    attributes = {
        shape = {'cake', 'car', 'cassette', 'hat', 'tv'},
        size = {'medium'},
  },
}

-- Random goal object, one distractor with a different shape.
local shapeTask = {
    {reward = 10},
    {reward = -10, shape = lcm.differentTo(1)},
    itemsPerGroup = {1, 1},
    goalGroup = 1,
    key = "[goal.shape]",
    rewardController = reward_controllers.createSimple()
}

return factory.createLevelApi{
    objectContext = objectContext,
    taskSelector = selectors.createIdentity(shapeTask),
    playerSpawnAngleRange = 20,
    levelMapSelector = selectors.createOrdered{fourObjectMap},
    episodeLengthSeconds = 60,
    objectPlacer = placers.createRandom(),
}
