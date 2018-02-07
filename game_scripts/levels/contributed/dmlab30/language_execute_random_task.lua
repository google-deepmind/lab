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
local helpers = require 'common.helpers'
local item_count = require 'language.item_count'
local lcm = require 'language.make'
local placers = require 'language.placers'
local selectors = require 'language.selectors'
local reward_controllers = require 'language.reward_controllers'

local twoAreaMap = {
    name = 'twoAreaMap',
    entityLayer = [[
O....O
..PP..
O....O
]],
    variationsLayer = [[
AAABBB
AAABBB
AAABBB
]],
    rooms = {
        A = {
            [1] = true,
            [3] = true,
        },
        B = {
            [2] = true,
            [4] = true,
        },
    },
}

local twoRoomMap = {
    name = 'twoRoomMap',
    entityLayer = [[
O..***..O
..P...P..
O..***..O
]],
    variationsLayer = [[
AAA...BBB
AAA...BBB
AAA...BBB
]],
    rooms = {
        A = {
            [1] = true,
            [3] = true,
        },
        B = {
            [2] = true,
            [4] = true,
        },
    },
}

local fourRoomMap = {
    name = 'fourRoomMap',
    entityLayer = [[
***O.O***
***...***
***.P.***
O..*.*..O
..P...P..
O..*.*..O
***.P.***
***...***
***O.O***
]],
    variationsLayer = [[
...AAA...
...AAA...
...AAA...
BBB...CCC
BBB...CCC
BBB...CCC
...DDD...
...DDD...
...DDD...
]],
    rooms = {
        A = {
            [1] = true,
            [2] = true,
        },
        B = {
            [3] = true,
            [5] = true,
        },
        C = {
            [4] = true,
            [6] = true,
        },
        D = {
            [7] = true,
            [8] = true,
        },
    }
}

local sixRoomMap = {
    name = 'sixRoomMap',
    entityLayer = [[
O..***..O
..P...P..
O..*.*..O
****.****
O..*.*..O
..P...P..
O..*.*..O
****.****
O..*.*..O
..P...P..
O..***..O
]],
    variationsLayer = [[
AAA...BBB
AAA...BBB
AAA...BBB
.........
CCC...DDD
CCC...DDD
CCC...DDD
.........
EEE...FFF
EEE...FFF
EEE...FFF
]],
    rooms = {
        A = {
            [1] = true,
            [3] = true,
        },
        B = {
            [2] = true,
            [4] = true,
        },
        C = {
            [5] = true,
            [7] = true,
        },
        D = {
            [6] = true,
            [8] = true,
        },
        E = {
            [9] = true,
            [11] = true,
        },
        F = {
            [10] = true,
            [12] = true,
        },
    },
}

--[[ For these multi tasks:

*   Use all shapes
*   Use 6 colors for objects: RGBCMY
*   No patterns - use solid
*   Only medium size.
]]
local COLORS = {'red', 'green', 'blue', 'cyan', 'magenta', 'yellow'}
local objectContext = object_generator.createContext{
    attributes = {
        color = COLORS,
    },
    -- Default all pickups to be bad unless explicitly set in the group rule.
    attributeDefaults = {
        pattern = 'solid',
        reward = -10,
        size = 'medium'
    }
}

local mapSelectors = {
    [2] = selectors.createRandom{twoAreaMap, twoRoomMap},
    [4] = selectors.createIdentity(fourRoomMap),
    [6] = selectors.createIdentity(sixRoomMap),
}

-- 1 Random goal object.
-- 2 x2 Distractor with different shape and color to the goal.
local twoRoomPickTask = {
    name = 'twoRoomPickTask',
    {reward = 10},
    {color = lcm.differentTo(1), shape = lcm.differentTo(1)},
    key = {'[goal.color]',
           '[goal.shape]',
           '[goal.color] [goal.shape]',
    },
    itemsPerGroup = {1, 2},
    -- +10, -5 to balance reward
    taskMapSelector = mapSelectors[2]
}

-- As twoRoomPickTask with 4 distractors
local fourRoomPickTask = helpers.shallowCopy(twoRoomPickTask)
fourRoomPickTask.name = 'fourRoomPickTask'
fourRoomPickTask.itemsPerGroup = {1, 4}
fourRoomPickTask.taskMapSelector = mapSelectors[4]


-- As twoRoomPickTask with 8 distractors
local sixRoomPickTask = helpers.shallowCopy(twoRoomPickTask)
sixRoomPickTask.name = 'sixRoomPickTask'
sixRoomPickTask.itemsPerGroup = {1, 8}
sixRoomPickTask.taskMapSelector = mapSelectors[6]


-- 1 Random goal object.
-- 2 Distractor with different shape, same color to goal.
-- 3 Distractor with different color, same shape to goal.
local twoRoomPickBothTask = {
    name = 'twoRoomPickBothTask',
    {reward = 10},
    {color = lcm.sameAs(1), shape = lcm.differentTo(1)},
    {color = lcm.differentTo(1), shape = lcm.sameAs(1)},
    key = '[goal.color] [goal.shape]',
    itemsPerGroup = {1, 1, 1},
    taskMapSelector = mapSelectors[2],
}

-- As twoRoomPickBothTask with more distractors.
local fourRoomPickBothTask = helpers.shallowCopy(twoRoomPickBothTask)
fourRoomPickBothTask.name = 'fourRoomPickBothTask'
fourRoomPickBothTask.itemsPerGroup = {1, 3, 3}
fourRoomPickBothTask.taskMapSelector = mapSelectors[4]

-- 1 Random goal object.
-- 2 x8 Distractor with different shape and color to the goal.
local sixRoomPickBothTask = helpers.shallowCopy(twoRoomPickBothTask)
sixRoomPickBothTask.name = 'sixRoomPickBothTask'
sixRoomPickBothTask.itemsPerGroup = {1, 5, 5}
sixRoomPickBothTask.taskMapSelector = mapSelectors[6]

-- 1 x2 Random goal objects with same shape.
-- 2 x2 Distractor with different shape to goal.
local twoRoomAllShapeTask = {
    name = 'twoRoomAllShapeTask',
    {reward = 1, reward2 = 10, shape = lcm.commonRandom()},
    {reward = -10, shape = lcm.differentTo(1)},
    itemsPerGroup = {2, 2},
    key = 'Every [goal.shape]',
    taskMapSelector = mapSelectors[2],
    -- Reward Controller specialised for this task.
    rewardController = reward_controllers.createBalancedCounting(),
}

-- As above, but object counts randomised, between 2 and 6 goal objects, max of
-- 8 objects in total.
local fourRoomAllShapeTask = helpers.shallowCopy(twoRoomAllShapeTask)
fourRoomAllShapeTask.name = 'fourRoomAllShapeTask'
fourRoomAllShapeTask.taskMapSelector = mapSelectors[4]
fourRoomAllShapeTask.itemsPerGroup = item_count.createGroupCounts{
    groupMin = {2, 2},
    maxObjects = 8
}

-- 1 x2 Random goal objects with same color.
-- 2 x2 Distractor with different color to goal.
local twoRoomAllColorTask = {
    name = 'twoRoomAllColorTask',
    {reward = 1, reward2 = 10, color = lcm.commonRandom()},
    {reward = -10, color = lcm.differentTo(1)},
    itemsPerGroup = {2, 2},
    key = 'Every [goal.color] object',
    taskMapSelector = mapSelectors[2],
    -- Reward Controller specialised for this task.
    rewardController = reward_controllers.createBalancedCounting(),
}

-- As above, but object counts randomised, between 2 and 6 goal objects, max of
-- 8 objects in total.
local fourRoomAllColorTask = helpers.shallowCopy(twoRoomAllColorTask)
fourRoomAllColorTask.name = 'fourRoomAllColorTask'
fourRoomAllColorTask.taskMapSelector = mapSelectors[4]
fourRoomAllColorTask.itemsPerGroup = item_count.createGroupCounts{
    groupMin = {2, 2},
    maxObjects = 8
}

--[[ Slightly cheaty 'near' task: since the maps here have 2 objects per room,
use room information rather than adjacency information to specify this
problem, to save time authoring the adjacency info.

1 Random goal object.
2 Random 'anchor' object, different shape to goal.  Next to goal.
3 Distractor with same shape as goal, in different room.
4 Distractor with different shape to 2, in same room as 3.
]]
local GOAL, ANCHOR = 1, 2
local twoRoomShapeNearShapeTask = {
    name = 'twoRoomShapeNearShapeTask',
    {reward = 10, region = lcm.random()},
    {shape = lcm.differentTo(GOAL), region = lcm.sameAs(GOAL)},
    {shape = lcm.sameAs(GOAL), region = lcm.differentTo(GOAL)},
    {shape = lcm.differentTo(ANCHOR), region = lcm.sameAs(3)},
    key = '[goal.shape] near [other.shape]',
    itemsPerGroup = {1, 1, 1, 1},
    taskMapSelector = mapSelectors[2],
    objectPlacer = placers.createRoom(),
}

local fourRoomShapeNearShapeTask = {
    name = 'fourRoomShapeNearShapeTask',
    -- 1, 2: Goal next to anchor
    {reward = 10, region = lcm.random()},
    {shape = lcm.differentTo(GOAL), region = lcm.sameAs(GOAL)},
    -- 3, 4: Goal shape next to not-anchor
    {shape = lcm.sameAs(GOAL), region = lcm.differentTo(GOAL)},
    {shape = lcm.differentTo(ANCHOR), region = lcm.sameAs(3)},
    -- 5, 6: Anchor shape next to Not-goal
    {shape = lcm.sameAs(ANCHOR), region = lcm.differentTo{GOAL, 3}},
    {shape = lcm.differentTo(GOAL), region = lcm.sameAs(5)},
    -- 7 x2: Not-goal-or-anchor next to Not-goal-or-anchor.
    {shape = lcm.differentTo{GOAL, ANCHOR}, region = lcm.differentTo{1, 3, 5}},
    key = '[goal.shape] near [other.shape]',
    itemsPerGroup = {1, 1, 1, 1, 1, 1, 2},
    taskMapSelector = mapSelectors[4],
    objectPlacer = placers.createRoom(),
}

local fourRoomShapeNearColorTask = {
    name = 'fourRoomShapeNearColorTask',
    -- 1, 2: Goal next to anchor
    {region = lcm.random()},
    {color = lcm.differentTo(GOAL), region = lcm.sameAs(GOAL)},
    -- 3, 4: Goal shape next to not-anchor.  Make the goal color different to
    -- anchor so it can't be mistaken for one if 4 happens to match goal shape.
    {shape = lcm.sameAs(GOAL), color = lcm.differentTo(ANCHOR),
     region = lcm.differentTo(GOAL)},
    {color = lcm.differentTo(ANCHOR), region = lcm.sameAs(3)},
    -- 5, 6: Anchor color next to Not-goal.  Again, make sure anchor shape
    -- different to goal.
    {color = lcm.sameAs(ANCHOR), shape = lcm.differentTo(GOAL),
     region = lcm.differentTo{GOAL, 3}},
    {shape = lcm.differentTo(GOAL), region = lcm.sameAs(5)},
    -- 7 x2: Not-goal-or-anchor next to Not-goal-or-anchor.
    {shape = lcm.differentTo(GOAL), color = lcm.differentTo(ANCHOR),
     region = lcm.differentTo{1, 3, 5}},
    key = '[goal.shape] near [other.color] object',
    itemsPerGroup = {1, 1, 1, 1, 1, 1, 2},
    taskMapSelector = mapSelectors[4],
    objectPlacer = placers.createRoom(),
}


-- Generator which defaults to using the shape picked for group 1 unless group
-- rule explicitly specifies otherwise.
local sameShapeContext = object_generator.createContext{
    attributes = {
        color = COLORS,
    },
    -- Default all pickups to be bad unless explicitly set in the group rule.
    attributeDefaults = {
        pattern = 'solid',
        reward = -10,
        shape = lcm.sameAs(1),
        size = 'medium',
    }
}

-- Version of next to shape task queued by 'color next to color'
local fourRoomColorNearColorTask = {
    name = 'fourRoomColorNearColorTask',
    -- 1, 2: Goal next to anchor.
    {reward = 10, shape = lcm.random(), region = lcm.random()},
    {color = lcm.differentTo(GOAL), region = lcm.sameAs(GOAL)},
    -- 3, 4: Goal color next to not-anchor.
    {color = lcm.sameAs(GOAL), region = lcm.differentTo(GOAL)},
    {color = lcm.differentTo(ANCHOR), region = lcm.sameAs(3)},
    -- 5, 6: Anchor color next to Not-goal.
    {color = lcm.sameAs(ANCHOR), region = lcm.differentTo{GOAL, 3}},
    {color = lcm.differentTo(GOAL), region = lcm.sameAs(5)},
    -- 7 x2: Not-goal-or-anchor next to Not-goal-or-anchor.
    {color = lcm.differentTo{GOAL, ANCHOR}, region = lcm.differentTo{1, 3, 5}},
    key = '[goal.color] object near [other.color] object',
    itemsPerGroup = {1, 1, 1, 1, 1, 1, 2},
    taskMapSelector = mapSelectors[4],
    objectPlacer = placers.createRoom(),
    -- objects will have same shape as group 1 unless explicitly choosing.
    objectContext = sameShapeContext
}

return factory.createLevelApi{
    objectContext = objectContext,
    episodeLengthSeconds = 120,
    playerSpawnAngleRange = 360,
    rewardController = reward_controllers.createBalanced(10),
    objectPlacer = placers.createRandom(),

    taskSelector = selectors.createDiscreteDistribution{
        -- Pick object identified by color, shape or both.
        {2, twoRoomPickTask},
        {1, fourRoomPickTask},
        {1, sixRoomPickTask},
        -- Pick object identified by color and shape.
        {2, twoRoomPickBothTask},
        {1, fourRoomPickBothTask},
        {1, sixRoomPickBothTask},
        -- Every <shape>.
        {2, twoRoomAllShapeTask},
        {1, fourRoomAllShapeTask},
        -- Every <color> object.
        {2, twoRoomAllColorTask},
        {1, fourRoomAllColorTask},
        -- Get shape near othershape
        {2, twoRoomShapeNearShapeTask},
        {1, fourRoomShapeNearShapeTask},
        -- Get shape near color object
        {1, fourRoomShapeNearColorTask},
        -- Get color object near othercolor object
        {1, fourRoomColorNearColorTask},
    },
}
