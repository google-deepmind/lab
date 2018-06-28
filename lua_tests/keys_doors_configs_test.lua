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
