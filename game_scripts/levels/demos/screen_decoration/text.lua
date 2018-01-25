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
local screen_message = require 'common.screen_message'

local api = {}

function api:nextMap()
  return make_map.makeMap{mapName = 'text', mapEntityLayer = " P "}
end

--[[ Renders fixed width strings to the screen.

Coordinate system is 0,0 Top Left. Width and height is always 640, 480.

Keyword Args

    * max_string_length(79) Strings will be truncated to this length.
    * line_height(20) - Multiple are best separated by this distance.
    * width(640) - Ideal screen width textures are streched to match this.
    * height(480) - Ideal screen height textures are streched to match this.

Returns text to be rendered.
]]
function api:screenMessages(args)
  return {
      -- Right text with shadow.
      {
          x = screen_message.BORDER_SIZE,
          y = args.height * 0.5 - args.line_height * 0.5,
          message = 'Right Text',
          alignment = screen_message.ALIGN_RIGHT,
          rgba = {1, 1, 1, 1},
      },
      -- Center text custom drop shadow.
      {
          x = args.width * 0.5 + 1,
          y = args.height * 0.5 - args.line_height * 0.5 + 1,
          message = 'Center Text',
          alignment = screen_message.ALIGN_CENTER,
          rgba = {0.5, 0, 0, 1},
          shadow = false,
      },
      -- Center text white
      {
          x = args.width * 0.5 - 1,
          y = args.height * 0.5 - args.line_height * 0.5 - 1,
          message = 'Center Text',
          alignment = screen_message.ALIGN_CENTER,
          rgba = {1, 1, 1, 1},
          shadow = false,
      },
      -- Right text with shadow.
      {
          x = args.width - screen_message.BORDER_SIZE,
          y = args.height * 0.5 - args.line_height * 0.5,
          message = 'Right Text',
          alignment = screen_message.ALIGN_RIGHT,
          rgba = {1, 1, 1, 1},
      },
      -- Multiline text
      {
          x = args.width * 0.5 - 1,
          y = args.BORDER_SIZE,
          message = 'Multiline Line 1',
          alignment = screen_message.ALIGN_LEFT,
          rgba = {0.5, 1, 0.5, 1},
      },
      {
          x = args.width * 0.5 - 1,
          y = screen_message.BORDER_SIZE + args.line_height,
          message = 'Multiline Line 2',
          alignment = screen_message.ALIGN_LEFT,
          rgba = {0.5, 1, 0.5, 1},
      },
  }
end

return api

