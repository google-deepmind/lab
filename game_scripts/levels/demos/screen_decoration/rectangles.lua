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

-- Demonstration of rendering rectangles in screen space.
local make_map = require 'common.make_map'

local api = {}

function api:nextMap()
  return make_map.makeMap{mapName = 'rectangles', mapEntityLayer = " P "}
end

--[[ Renders rectangles to the screen.

Coordinate system is 0,0 Top Left. Width and height is always 640, 480.

Keyword Args

    * width(640) - Ideal screen width textures are streched to match this.
    * height(480) - Ideal screen height textures are streched to match this.

Returns list of rectangles to be rendered.
]]

function api:filledRectangles(args)
  local rectangles = {
      -- Green rectangle in top right 8th of the screen.
      {
          x = args.width * 0.75,
          y = 0,
          width = args.width * 0.25,
          height = args.height * 0.25,
          rgba = {0.0, 1.0, 0.0, 1.0}
      },
      -- Blue semi-transparent top left 8th of the screen.
      {
          x = 0,
          y = 0,
          width = args.width * 0.25,
          height = args.height * 0.25,
          rgba = {0.0, 0.0, 1.0, 0.5}
      },
      -- Red semi-transparent center 8th of the screen.
      {
          x = args.width * 0.375,
          y = args.height * 0.375,
          width = args.width * 0.25,
          height = args.height * 0.25,
          rgba = {1.0, 0.0, 0.0, 0.5}
      },
  }
  return rectangles
end

return api
