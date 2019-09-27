--[[ Copyright (C) 2019 Google LLC

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

-- !! This is not currently what gets used on borg, is only for local testing

local factory = require 'levels.contributed.psychlab.factories.temporal_discrimination_factory'

return factory.createLevelApi{
    schema = 'temporal_discrimination',
    targetDurations = {50, 100},
    randomTargetPlacements = true,
    targetFlashDuration = 10,

}
