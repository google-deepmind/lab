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

local pickups = {}

-- Must match itemType_t in engine/code/game/bg_public.h.
pickups.type = {
    INVALID = 0,
    WEAPON = 1,
    AMMO = 2,
    ARMOR = 3,
    HEALTH = 4,
    POWER_UP = 5,
    HOLDABLE = 6,
    PERSISTANT_POWERUP = 7,
    TEAM = 8,
    REWARD = 9,
    GOAL = 10
}

-- Must match reward_mv_t in engine/code/game/bg_public.h.
pickups.moveType = {
    BOB = 0,
    STATIC = 1
}

pickups.defaults = {
    apple_reward = {
        name = 'Apple',
        classname = 'apple_reward',
        model = 'models/apple.md3',
        quantity = 1,
        type = pickups.type.REWARD
    },
    lemon_reward = {
        name = 'Lemon',
        classname = 'lemon_reward',
        model = 'models/lemon.md3',
        quantity = -1,
        type = pickups.type.REWARD
    },
    strawberry_reward = {
        name = 'Strawberry',
        classname = 'strawberry_reward',
        model = 'models/strawberry.md3',
        quantity = 2,
        type = pickups.type.REWARD
    },
    fungi_reward = {
        name = 'Fungi',
        classname = 'fungi_reward',
        model = 'models/toadstool.md3',
        quantity = -10,
        type = pickups.type.REWARD
    },
    watermelon_goal = {
        name = 'Watermelon',
        classname = 'watermelon_goal',
        model = 'models/watermelon.md3',
        quantity = 20,
        type = pickups.type.GOAL
    },
    goal = {
        name = 'Goal',
        classname = 'goal',
        model = 'models/goal_object_02.md3',
        quantity = 10,
        type = pickups.type.GOAL
    },
    mango_goal = {
        name = 'Mango',
        classname = 'mango_goal',
        model = 'models/mango.md3',
        quantity = 100,
        type = pickups.type.GOAL
    }
}

return pickups
