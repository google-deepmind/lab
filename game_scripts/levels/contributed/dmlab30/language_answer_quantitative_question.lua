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

--[[ Ask yes/no questions based on object colors / counts:

Are all hats blue?
Is any hat blue?
Is anything red?
Are most hats blue?

The general setup is that a pair of objects representing 'answer yes', 'answer
no' when picked up are placed in region A and B on the map.  We use groups 1
and 2 for these for consistency.

Objects are then generated in the remaining groups to represent the problem
being asked, these cannot be picked up so the agent can examine them.

For some problems (e.g. is anything red?) we need to create an object which
is not placed in the world to have some instantiated attributes to reference.
Group 3 is used for this where required.
]]

local factory = require 'factories.language.factory'
local object_generator = require 'language.object_generator'
local item_count = require 'language.item_count'
local lcm = require 'language.make'
local placers = require 'language.placers'
local selectors = require 'language.selectors'
local reward_controllers = require 'language.reward_controllers'
local texture_sets = require 'themes.texture_sets'


-- Set up the map in zones: 'A' will have the 'yes' object, 'B' the 'no' object.
local quantificationSmallMap = {
    name = 'quantificationSmallMap',
    textureSet = texture_sets.CUSTOMIZABLE_FLOORS,
    entityLayer = [[
.....O
P..OO
P..OO
.....O
]],
    variationsLayer = [[
CCCCCA
CCCDD
CCCDD
CCCCCB
]],
    -- TODO(simongreen): Add methods to compute room information automatically.
    -- The data below indicates which object location ids within the map are
    -- present in which room.  The indexes are ordered top to bottom, left to
    -- right within the entityLayer.
    rooms = {
        A = {[1] = true},
        B = {[6] = true},
        C = {},
        D = {[2] = true,
             [3] = true,
             [4] = true,
             [5] = true
        },
    }
}

--[[ For these quantification tasks:

*   Use all shapes.
*   Use 6 colors for objects: RGBCMY.
*   No patterns - use solid.
*   No pickup unless explicit.
*   Unless explicitly specified, don't use region A, B, C as these are reserved.
]]
local COLORS = {'red', 'green', 'blue', 'cyan', 'magenta', 'yellow'}
local objectContext = object_generator.createContext{
    attributes = {
        color = COLORS,
    },
    attributeDefaults = {
        size = 'medium',
        pattern = 'solid',
        canPickup = false,
        region = {lcm.isNot({'A', 'B', 'C'}), lcm.random()},
    }
}

--[[ A common pattern for object counts with these problems is that we want a
few single items for e.g. yes, no, goal and then some variable counts to give an
even distribution of problems for which the answer is yes/no.  This utility
helps us make this pattern less wordy in the task descriptions.
]]
local function concatWithSelector(initial, selector)
  return item_count.concat(initial, selectors.createCalling(selector))
end

-- All the tasks below define group 1 and 2 the same:
-- 1 Magic yes ball - collect this to answer 'yes'
-- 2 Magic no ball - collect this to answer 'no'.
local YES_GROUP = {shape = 'ball', color = 'white', region = 'A', static = true,
                   canPickup = true}
local NO_GROUP = {shape = 'ball', color = 'black', region = 'B', static = true,
                  canPickup = true}


-- TASK: Are all cars red?
-- Yes: Distractor count is 0 => itemsPerGroup[4] == 0
local evenChanceOfAllSelector = selectors.createRandom{
    -- All goals objects and no distractors.
    selectors.createIdentity({4, 0}),
    -- At least one distractor.
    item_count.createGroupCounts{groupMin = {1, 1}, totalObjects = 4}}

local GOAL = 3
local areAllShapesThisColorTask = {
    YES_GROUP,
    NO_GROUP,
    -- 3 1-4 Random GOAL objects (all same shape & color).
    {shape = lcm.commonRandom(), color = lcm.commonRandom()},
    -- 4 0-3 Distractors with same shape as goal but different colors.
    {shape = lcm.sameAs(GOAL), color = lcm.differentTo(GOAL)},
    goalGroup = GOAL,
    key = 'Are all [goal.shape] objects [goal.color]?',
    -- Need 1 YES, 1 NO object plus selector to vary groups 2 and 3
    itemsPerGroup = concatWithSelector({1, 1}, evenChanceOfAllSelector),
    rewardController = reward_controllers.createAnswer{
        truth = function (counts) return counts[GOAL + 1] == 0 end,
        groupToAnswer = {true, false}},
}


-- TASK: Is any car blue?
-- Yes = items in visible goal group > 0
-- Select item counts so the question has equal probability of true/false.
local evenChanceOfAnySelector = selectors.createRandom{
    -- Minimum one visible item.
    item_count.createGroupCounts{totalObjects = 4,
                                 groupMin = {1, 0, 0}},
    -- Set max to 0 so no visible items.
    item_count.createGroupCounts{totalObjects = 4,
                                 groupMin = {0, 0, 0},
                                 groupMax = {0, -1, -1}}}

local VISIBLE_GOALS = 4
local isAnyShapeThisColorTask = {
    YES_GROUP,
    NO_GROUP,
    -- 3 A random GOAL object. Not visible in world. Used to seed the question.
    {region = 'DO_NOT_PLACE'},
    -- 4 0-4 Objects with same shape and color as GOAL.
    {shape = lcm.sameAs(GOAL), color = lcm.sameAs(GOAL)},
    -- 5 0-4 Distractors with same shape as goal but different colors.
    {shape = lcm.sameAs(GOAL), color = lcm.differentTo(GOAL)},
    -- 6 0-4 Distractors with different shape to goal.
    {shape = lcm.differentTo(GOAL)},
    goalGroup = GOAL,
    key = 'Is any [goal.shape] [goal.color]?',
    -- Need 1 YES, 1 NO, 1 non-visible GOAL, plus selector to vary the
    -- visible goals and distractor groups 4, 5 and 6 as described above.
    itemsPerGroup = concatWithSelector({1, 1, 1}, evenChanceOfAnySelector),
    rewardController = reward_controllers.createAnswer{
        truth = function (counts) return counts[VISIBLE_GOALS] > 0 end,
        groupToAnswer = {true, false}},
}


-- TASK: Is anything yellow?
-- Yes = itemsPerGroup[4] > 0
local isAnythingThisColorTask = {
    YES_GROUP,
    NO_GROUP,
    -- 3 A random GOAL object. Not visible in world. Used to seed the question.
    {region = 'DO_NOT_PLACE'},
    -- 4 0-4 VISIBLE_GOAL Objects with same color as GOAL.
    {color = lcm.sameAs(GOAL)},
    -- 5 0-4 Distractors with different colors.
    {color = lcm.differentTo(GOAL)},
    -- 6 0-4 Distractors with different colors (to reuse itemsPerGroup)
    {color = lcm.differentTo(GOAL)},
    goalGroup = GOAL,
    key = 'Is anything [goal.color]?',
    -- Need 1 YES, 1 NO, 1 non-visible GOAL, plus selector to vary the
    -- visible goals and distractor groups 4, 5 and 6 as described above.
    itemsPerGroup = concatWithSelector({1, 1, 1}, evenChanceOfAnySelector),
    rewardController = reward_controllers.createAnswer{
        truth = function (counts) return counts[VISIBLE_GOALS] > 0 end,
        groupToAnswer = {true, false}},
}


-- TASK: Are most hats red?
-- Yes = itemsPerGroup[3] > itemsPerGroup[4]

--[[ Getting even yes/no problems seems a bit complicated for this task.

Ways to guarantee more red hats:

*   set min red hats 1, max other hats 0
*   set min red hats 2, max other hats 1
*   set min red hats 3, max other anything
*   set min red hats 4, max other anything

Ways to guarantee not more red hats:

*   set max red 0, min other 0
*   set max red 1, min other 1
*   set max red 2, min other 2
]]

-- Select item counts so the question has equal probabily of true/false.
local evenChanceOfMoreSelector = selectors.createRandom{
    -- Minimum one visible item.
    item_count.createGroupCounts{totalObjects = 4,
                                 groupMin = {1, 0, 0}},
    -- Set max to 0 so no visible items.
    item_count.createGroupCounts{totalObjects = 4,
                                 groupMin = {0, 0, 0},
                                 groupMax = {0, -1, -1}}}

local evenChanceOfMoreSelector = selectors.createDiscreteDistribution{
    -- Yes setups
    {1, item_count.createGroupCounts{totalObjects = 4,
                                     groupMin = {1, 0, 0},
                                     groupMax = {-1, 0, -1}}},
    {1, item_count.createGroupCounts{totalObjects = 4,
                                     groupMin = {2, 0, 0},
                                     groupMax = {-1, 1, -1}}},
    {1, item_count.createGroupCounts{totalObjects = 4,
                                     groupMin = {3, 0, 0},
                                     groupMax = {-1, 1, -1}}},
    {1, item_count.createGroupCounts{totalObjects = 4,
                                     groupMin = {4, 0, 0},
                                     groupMax = {-1, 0, -1}}},
    -- No setups
    {1, item_count.createGroupCounts{totalObjects = 4,
                                     groupMin = {0, 1, 0},
                                     groupMax = {0, -1, -1}}},
    {2, item_count.createGroupCounts{totalObjects = 4,
                                     groupMin = {0, 1, 0},
                                     groupMax = {1, -1, -1}}},
    {1, item_count.createGroupCounts{totalObjects = 4,
                                     groupMin = {0, 2, 0},
                                     groupMax = {2, -1, -1}}},
}


local areMostShapesThisColorTask = {
    YES_GROUP,
    NO_GROUP,
    -- 3 A random GOAL object. Not visible in world. Used to seed the question.
    {region = 'DO_NOT_PLACE'},
    -- 4 0-4 Visible random goal objects (all same shape & color).
    {shape = lcm.sameAs(GOAL), color = lcm.sameAs(GOAL)},
    -- 5 0-4 Distractors with same shape as goal but different colors.
    {shape = lcm.sameAs(GOAL), color = lcm.differentTo(GOAL)},
    -- 6 0-4 Distractors with different shape to goal.
    {shape = lcm.differentTo(GOAL)},
    goalGroup = GOAL,
    key = 'Are most [goal.shape] objects [goal.color]?',
    -- Need 1 YES, 1 NO, 1 non-visible GOAL, plus selector to vary the
    -- visible goals and distractor groups 4, 5 and 6 as described above.
    itemsPerGroup = concatWithSelector({1, 1, 1}, evenChanceOfMoreSelector),
    rewardController = reward_controllers.createAnswer{
        truth = function (counts)
          return counts[VISIBLE_GOALS] > counts[VISIBLE_GOALS + 1] end,
        groupToAnswer = {true, false}},
}


-- Put yes and no objects on green and red floors respectively.
local function regionColorSelector(ignored_kwargs)
  return {
      A = {floor = 'green'},
      B = {floor = 'red'},
      C = {floor = 'orange'},
      D = {floor = 'black'},
  }
end

return factory.createLevelApi{
    objectContext = objectContext,
    episodeLengthSeconds = 60,
    playerSpawnAngleRange = 20,
    objectPlacer = placers.createRoom(),
    levelRegionColorSelector = regionColorSelector,
    levelMapSelector = selectors.createIdentity(quantificationSmallMap),
    taskSelector = selectors.createDiscreteDistribution{
        {1, areAllShapesThisColorTask},
        {1, isAnyShapeThisColorTask},
        {1, isAnythingThisColorTask},
        {1, areMostShapesThisColorTask},
    },
}
