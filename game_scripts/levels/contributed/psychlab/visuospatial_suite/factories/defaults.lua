--[[ Copyright (C) 2019 Google LLC

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

--[[ This file declares general defaults to be used for the entire
visuospatial task suite.
]]

local defaults = {
    TIME_TO_FIXATE_CROSS = 1, -- In frames.
    FAST_INTER_TRIAL_INTERVAL = 1, -- In frames.
    TRIAL_TIMEOUT = 300, -- In frames.
    EPISODE_LENGTH_SECONDS = 300,
    MAX_STEPS_OFF_SCREEN = 300,  -- 5 seconds.
    TRIALS_PER_EPISODE_CAP = 50,

    BG_COLOR = {0, 0, 0},
    FIXATION_COLOR = {255, 255, 255}, -- RGB white.
    WHITE_BUTTON_COLOR = {255, 255, 255},
    GREEN_BUTTON_COLOR = {100, 255, 100},
    RED_BUTTON_COLOR = {255, 100, 100},
    -- We verified that all pairs of these colors are distinguishable by humans.
    COLORS = {
        {240, 163, 255}, -- Amethyst
        {194, 0, 136}, -- Mallow
        {116, 10, 255}, -- Violet
        {153, 63, 0}, -- Caramel
        {255, 204, 153}, -- Honeydew
        {148, 255, 181}, -- Jade
        {157, 204, 0}, -- Lime
        {255, 164, 5}, -- Orange
        {94, 241, 242}, -- Sky
        {255, 255, 0}, -- Yellow
    },

    SCREEN_SIZE = {width = 512, height = 512},
    FIXATION_SIZE = 0.1,
    BUTTON_SIZE = 0.1,

    FIXATION_REWARD = 0,
    CORRECT_REWARD = 1,
    INCORRECT_REWARD = 0,
}

return defaults
