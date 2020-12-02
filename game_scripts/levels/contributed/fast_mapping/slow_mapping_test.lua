--[[ Copyright (C) 2020 Google Inc.

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

local twoObjectMap = {
    name = 'twoPositions',
    entityLayer = [[
..O
P..
..O
]],
}

local shapeTask = {
    {
        reward = 10,
        size = "medium",
        color = "red",
        pattern = {'solid'},
        shape = lcm.oneOf(object_generator.SHAPES),
    },
    {
        reward = -10,
        size = "medium",
        color = "red",
        pattern = {'solid'},
        shape = {
            lcm.choices(object_generator.SHAPES),
            lcm.notSameAs(1),
            lcm.random(),
        },
     },
    itemsPerGroup = {1, 1},
    goalGroup = 1,
    key = "[goal.shape]",
    rewardController = reward_controllers.createSimple(),
}

return factory.createLevelApi{
    taskSelector = selectors.createDiscreteDistribution({{1, shapeTask}}),
    playerSpawnAngleRange = 20,
    levelMapSelector = selectors.createOrdered{twoObjectMap},
    episodeLengthSeconds = 60,
    objectPlacer = placers.createRandom(),
}
