local asserts = require 'testing.asserts'
local test_runner = require 'testing.test_runner'
local helpers = require 'common.helpers'
local io = require 'io'

local game = require 'dmlab.system.game'

local tests = {}

function tests.tempFolder()
  asserts.NE(game:tempFolder(), '')
  local tempFile = helpers.pathJoin(game:tempFolder(), 'temp.txt')
  assert(not helpers.fileExists(tempFile), 'Temp folder should be empty!')
  local file = io.open(tempFile, 'w')
  asserts.NE(file, nil, 'Failed to create file.')
  io.close(file)
  assert(helpers.fileExists(tempFile), 'tempFolder file cannot be found!')
end

function tests.runFiles()
  asserts.NE(game:runFiles(), '')
  local thisFile = helpers.pathJoin(game:runFiles(), 'lua_tests/game_test.lua')
  assert(helpers.fileExists(thisFile), 'runFiles path is incorrect!')
end

return test_runner.run(tests)
