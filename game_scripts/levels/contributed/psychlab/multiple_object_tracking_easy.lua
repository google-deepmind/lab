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

local factory = require 'levels.contributed.psychlab.factories.multiple_object_tracking_factory'

--[[ An easy version of the multiple_object_tracking task.
In this version the objects move much more slowly, and the duration of the
tracking interval is much shorter. This means that instead of needing to track
moving objects, the agent only needs to maintain activations for nearly
stationary objects.
]]
return factory.createLevelApi{
    schema = 'multiple_object_tracking_easy',
    videoLength = 5,
    motionSpeeds = {1, 2, 3, 4, 5},
    numToTrackSequence = {1},
    ballDiameter = 80,
    allowableDistanceToWall = 41,
    minDistanceBetweenBalls = 161,
}
