--[[ Copyright (C) 2017-2019 Google Inc.

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

local asserts = require 'testing.asserts'
local test_runner = require 'testing.test_runner'
local configs = require 'map_generators.keys_doors.configs'

local tests = {}

function tests.testDefaultConfig()
  local config = configs.getConfig()
  assert(config.coreSequenceLength >= 1 and config.coreSequenceLength <= 3)
  asserts.EQ(config.numTrapRooms, 1)
  asserts.EQ(config.probTrapRoomDuringCore, 0.5)
  asserts.EQ(config.numRedHerringRooms, 1)
end

function tests.testOverrideConfig()
  local overrideConfig = {
      coreSequenceLength = {4, 4},
      numTrapRooms = 2,
  }
  local config = configs.getConfig(overrideConfig)
  asserts.EQ(config.coreSequenceLength, 4)
  asserts.EQ(config.numTrapRooms, 2)
  asserts.EQ(config.numRedHerringRooms, 1)

  local config2 = configs.getConfig()
  assert(config2.coreSequenceLength >= 1 and config2.coreSequenceLength <= 3)
  asserts.EQ(config2.numTrapRooms, 1)
  asserts.EQ(config2.probTrapRoomDuringCore, 0.5)
  asserts.EQ(config2.numRedHerringRooms, 1)
end

return test_runner.run(tests)
