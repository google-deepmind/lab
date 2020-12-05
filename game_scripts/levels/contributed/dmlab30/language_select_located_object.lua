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
local region_colors = require 'language.region_colors'
local reward_controllers = require 'language.reward_controllers'
local texture_sets = require 'themes.texture_sets'

local twoAreaMap = {
    name = 'twoAreaMap_customFloors',
    textureSet = texture_sets.CUSTOMIZABLE_FLOORS,
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
    -- TODO(simongreen): Add methods to compute room information automatically.
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
    name = 'twoRoomMap_customFloors',
    textureSet = texture_sets.CUSTOMIZABLE_FLOORS,
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
    name = 'fourRoomMap_customFloors',
    textureSet = texture_sets.CUSTOMIZABLE_FLOORS,
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
    name = 'sixRoomMap_customFloors',
    textureSet = texture_sets.CUSTOMIZABLE_FLOORS,
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

--[[ For the in room tasks:

*   Use all shapes.
*   Use 6 colors for floors and objects: RGBCMY.
*   No patterns - use solid.
*   Only mediumn objects.
]]
local GOAL_REWARD = 10
local COLORS = {'red', 'green', 'blue', 'cyan', 'magenta', 'yellow'}
local objectContext = object_generator.createContext{
    attributes = {
        color = COLORS,
        pattern = {'solid'},
    },
    attributeDefaults = {
        size = 'medium',
    },
}
local regionColorSelector = region_colors.createShuffledFloors(COLORS)

--[[ Task Description:
1 Random goal object in one room, identified by object and floor color.
2 Distractor with different color to the goal in same room.
3 Distractor with same color as the goal but in a different room.
4 Distractor in the 2nd room with same color as 2 to prevent counting most
. common color being a strategy for 50/50 chance.
]]
local twoRoomColorTask = {
    {region = lcm.random()},
    {color = lcm.differentTo(1), region = lcm.sameAs(1)},
    {color = lcm.sameAs(1), region = lcm.differentTo(1)},
    {color = lcm.sameAs(2), region = lcm.sameAs(3)},
    key = "Pick the [goal.color] object in the [goal.roomColor.floor] room",
    itemsPerGroup = {1, 1, 1, 1},
    taskMapSelector = selectors.createRandom{twoAreaMap, twoRoomMap},
}

--[[ Task Description:
1 Random goal object in one room, identified by object and floor color.
2 Distractor with different color to the goal in same room.
3 Distractor with same color as the goal but in a different room.
4 Distractor in any other room with same color as 2 to prevent counting most
.  common color being a strategy for 50/50 chance.
5 x4 random distractors spread across the rooms
]]
local fourRoomColorTask = {
    {region = lcm.random()},
    {color = lcm.differentTo(1), region = lcm.sameAs(1)},
    {color = lcm.sameAs(1), region = lcm.differentTo(1)},
    {color = lcm.sameAs(2), region = 'any'},
    {region = 'any'},
    key = "Pick the [goal.color] object in the [goal.roomColor.floor] room",
    itemsPerGroup = {1, 1, 1, 1, 4},
    taskMapSelector = selectors.createRandom{fourRoomMap},
}

--[[ Task Description:
1 Random goal object in one room, identified by object and floor color.
2 Distractor with different color to the goal in same room.
3 Distractor with same color as the goal but in a different room.
4 Distractor in any other room with same color as 2 to prevent counting most
. common color being a strategy for 50/50 chance.
5 x8 random distractors spread across the rooms
]]
local sixRoomColorTask = {
    {region = lcm.random()},
    {color = lcm.differentTo(1), region = lcm.sameAs(1)},
    {color = lcm.sameAs(1), region = lcm.differentTo(1)},
    {color = lcm.sameAs(2), region = 'any'},
    {region = 'any'},
    key = "Pick the [goal.color] object in the [goal.roomColor.floor] room",
    itemsPerGroup = {1, 1, 1, 1, 8},
    taskMapSelector = selectors.createRandom{sixRoomMap},
}

return factory.createLevelApi{
    objectContext = objectContext,
    episodeLengthSeconds = 120,
    playerSpawnAngleRange = 360,
    rewardController = reward_controllers.createBalanced(GOAL_REWARD),
    objectPlacer = placers.createRoom(),
    levelRegionColorSelector = regionColorSelector,
    taskSelector = selectors.createDiscreteDistribution{
        {2, twoRoomColorTask},
        {1, fourRoomColorTask},
        {1, sixRoomColorTask},
    },
}
