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

local factory = require 'factories.natlab_factory'

local NUM_MAPS = 100
local MAP_PREFIX = "natlab_forage_variable_"
local NATLAB_MAPS = {}
for i = 1, NUM_MAPS do
  NATLAB_MAPS[i] = string.format("%s%03d", MAP_PREFIX, i - 1)
end

return factory.createLevelApi{
    maps = NATLAB_MAPS,
    episodeLengthSeconds = 120,
    camera = {1000, 1200, 1650}
}
