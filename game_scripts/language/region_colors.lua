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

local random = require 'common.random'


--[[ Region (aka room) coloring.

Defines functions which will be called to decide on the mapping between map
variation regions and the colors the should take.

Functions are called with the following keyword arguments:

*   `regions` (list) List of regions present in the current map.

Returns a table, keyed by region.  Values are tables indicating how to color
components of that region.  Currently 'floor' is the only supported component.
Note that these methods should return color names, not RGB values.
]]
local region_colors = {}

function region_colors.createShuffledFloors(colorNames)
  return function(kwargs)
    assert(kwargs.regions)
    assert(#colorNames >= #kwargs.regions)
    local nextColorIndexGen = random:shuffledIndexGenerator(#colorNames)
    local regionColors = {}
    for ii, region in ipairs(kwargs.regions) do
      regionColors[region] = {floor = colorNames[nextColorIndexGen()]}
    end
    return regionColors
  end
end

return region_colors
