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

local factory = require 'levels.contributed.psychlab.factories.change_detection_factory'

-- We verified that all pairs of these colors are distinguishable by humans.
local COLORS_TRAIN = {
    {240, 163, 255}, -- Amethyst
    {194, 0, 136}, -- Mallow
    {148, 255, 181}, -- Jade
    {153, 63, 0}, -- Caramel
    {255, 204, 153}, -- Honeydew
}

return factory.createLevelApi{
    -- General kwargs:
    episodeLengthSeconds = 300,
    trialsPerEpisodeCap = 50,
    fractionToPromote = 0.0,
    fractionToDemote = 0.0,
    probeProbability = 1.0,  -- I.e. uniformly sample difficulty levels.
    fixedTestLength = math.huge,
    initialDifficultyLevel = 4,
    correctRewardSequence = 1.0,
    maxStepsOffScreen = 300,

    -- Specific kwargs for this task:
    schema = 'memory_suite_change_detection_train.lua',
    setSizes = {1, 2, 3, 4},
    requireAtCenterPreresponse = true,
    selfPaced = '0',
    allowTranslation = true,
    studyTime = 25,
    grid = {size = 120, step = 20},
    preresponseTime = 8,

    -- Critical kwargs defining the training vs. held-out testing protocol.
    delayTimes = {2, 4, 8, 64, 128},
    colors = COLORS_TRAIN
}
