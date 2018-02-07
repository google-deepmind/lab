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

local factory = require 'factories.lasertag.procedural_factory'

return factory.createLevelApi{
    episodeLengthSeconds = 240,
    botCount = 1,
    color = true,
    mazeGenerationParams = {
        height = 11,
        width = 11,
        maxRooms = 3,
        roomMinSize = 3,
        roomMaxSize = 5,
        roomSpawnCount = 3,
    },
    pickupParams = {
        pickupCount = 2,
        weaponCount = 1,
    },
}
