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

local factory = require 'levels.contributed.psychlab.factories.visual_search_factory'

-- These are RGB values, they have a fixed luminance in HSL space.
-- The first color is the target color.
local COLORS = {
    {255, 0, 191}, -- magenta
    {255, 191, 0}, -- sunflower yellow
}

-- The first shape is the target shape.
local ORIENTATIONS = {
    {{0, 1, 0, 0},
     {0, 1, 0, 0},
     {0, 1, 0, 0},
     {0, 0, 0, 0}},  -- vertical

    {{1, 1, 1, 0},
     {0, 0, 0, 0},
     {0, 0, 0, 0},
     {0, 0, 0, 0}},  -- horizontal1

    {{0, 0, 0, 0},
     {1, 1, 1, 0},
     {0, 0, 0, 0},
     {0, 0, 0, 0}},  -- horizontal2 (slightly offset horizontal bar)

    {{0, 0, 0, 0},
     {0, 0, 0, 0},
     {1, 1, 1, 0},
     {0, 0, 0, 0}},  -- horizontal3 (slightly offset horizontal bar)
}

return factory.createLevelApi{
    schema = 'visual_search',
    episodeLengthSeconds = 180,
    shapes = ORIENTATIONS,
    colors = COLORS,
    targetSize = 48,
    setSizes = {1, 1, 2, 4, 8, 16, 24, 32, 40, 48, 56, 64}
}
