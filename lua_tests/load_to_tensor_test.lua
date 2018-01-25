local asserts = require 'testing.asserts'
local test_runner = require 'testing.test_runner'
local helpers = require 'common.helpers'
local tensor = require 'dmlab.system.tensor'
local game = require 'dmlab.system.game'

local tests = {}

function tests.loadFileToByteTensor()
  local tempFile = helpers.pathJoin(game:tempFolder(), 'temp.txt')
  local file = io.open(tempFile, 'wb')
  asserts.NE(file, nil, 'Failed to create file.')
  file:write('aaabbbccc')
  file:close(file)
  local tensor = game:loadFileToByteTensor(tempFile)
  for c = 1, 3 do
    asserts.EQ(string.byte('a'), tensor(c):val())
    asserts.EQ(string.byte('b'), tensor(c + 3):val())
    asserts.EQ(string.byte('c'), tensor(c + 6):val())
  end
end

return test_runner.run(tests)
