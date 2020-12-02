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

local lang_fastmapping = require 'factories.language.fastmapping_factory'
local object_generator = require 'language.object_generator'
local factory = require 'levels.contributed.fast_mapping.factories.fast_mapping_factory'
local game = require 'dmlab.system.game'
local lcm = require 'language.make'
local maze_generation = require 'dmlab.system.maze_generation'
local object_generator = require 'language.object_generator'
local placers = require 'language.placers'
local random = require 'common.random'
local selectors = require 'language.selectors'
local texter = require 'language.texter'

local ALL_ATTRIBUTES = {
    shape = factory.EVAL_SHAPES,
    pattern = {'solid'},
    color = {"red"},
    size = {"medium"},
}

local CONFOUNDER_SHAPES = {}
for i = 1, #factory.EVAL_SHAPES do
  CONFOUNDER_SHAPES[i] = 'runcible' .. i
end
local OBJECT_COUNT = 2
local DISTRACTOR_OBJECT_COUNT = 1
local GOAL_REWARD = 10
local LEARNING_REWARD = 0
local EPISODE_LENGTH_SECONDS = 60

local fastMap = lang_fastmapping.buildMap(OBJECT_COUNT, DISTRACTOR_OBJECT_COUNT)

local fastTask = factory.createTask{
    confoundNames = true,
    confounderShapes = CONFOUNDER_SHAPES,
    episodeLengthSeconds = EPISODE_LENGTH_SECONDS,
    distractorObjectCount = DISTRACTOR_OBJECT_COUNT,
    goalShapes = factory.EVAL_SHAPES,
    objectCount = OBJECT_COUNT,
    goalReward = GOAL_REWARD,
    learningReward = LEARNING_REWARD,
}

return factory.createLevelApi{
    episodeLengthSeconds = 60,
    instructor = factory.defaultInstructor,
    levelMapSelector = selectors.createIdentity(fastMap),
    objectContext = object_generator.createContext{attributes = ALL_ATTRIBUTES},
    taskSelector = selectors.createDiscreteDistribution({{10, fastTask}}),
}
