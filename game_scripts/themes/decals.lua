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

local NUMBER_OF_STYLES = 4
local NUMBER_OF_IMAGES_PER_STYLE = 20
local DEFAULT_WALL_DECALS = {}
local DEFAULT_WALL_IMAGES = {}
for i = 1, NUMBER_OF_STYLES do
  for j = 1, NUMBER_OF_IMAGES_PER_STYLE do
    local img = string.format('decal/lab_games/dec_img_style%02d_%03d', i, j)
    DEFAULT_WALL_DECALS[#DEFAULT_WALL_DECALS + 1] = {
        tex = img .. '_nonsolid'
    }
    DEFAULT_WALL_IMAGES[#DEFAULT_WALL_IMAGES + 1] = 'textures/' .. img
  end
end

return {
    decals = DEFAULT_WALL_DECALS,
    images = DEFAULT_WALL_IMAGES,
}
