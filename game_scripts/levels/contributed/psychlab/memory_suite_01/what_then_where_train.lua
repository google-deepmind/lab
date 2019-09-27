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

local factory = require 'levels.contributed.psychlab.factories.what_then_where_factory'

-- Indices into constantImagePerCategory list below.
local CATEGORIES = {0, 1, 2, 3, 4}

return factory.createLevelApi{
    -- General kwargs:
    episodeLengthSeconds = 600,
    trialsPerEpisodeCap = 50,
    fractionToPromote = 0.0,
    fractionToDemote = 0.0,
    probeProbability = 1.0,  -- I.e. uniformly sample difficulty levels.
    fixedTestLength = math.huge,
    initialDifficultyLevel = 1,
    correctRewardSequence = 1.0,
    maxStepsOffScreen = 300,

    -- Specific kwargs for this task:
    schema = 'memory_suite_what_then_where_train',
    requireAtCenterPreresponse = true,
    studyWhatTime = 25,  -- How long (frames) the "what" stimulus appears.
    studyWhereTime = 25,  -- How long (frames) the "where" stimulus appears.

    -- Critical kwargs defining the training vs. held-out testing protocol.
    -- Delays and images differ for train/test_interpolate/test_extrapolate.
    sequence = {{categories = CATEGORIES, delayTimes = {4, 8, 32, 128}}},
    -- Declare specific image to use from each category here.
    constantImagePerCategory = {
        39824, 41968, 1439, 40746, 56827
    },
}
