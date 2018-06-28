local image = require 'dmlab.system.image'
local game = require 'dmlab.system.game'
local asserts = require 'testing.asserts'
local test_runner = require 'testing.test_runner'
local tensor = require 'dmlab.system.tensor'
local helpers = require 'common.helpers'

local FILE_NAME = ...

local tests = {}

function tests:loadImageRGB()
  local image = image.load(helpers.dirname(FILE_NAME) .. "data/testRGB.png")
  local expected = tensor.ByteTensor(32, 32 * 3, 3)
  expected:narrow(2, 1, 32):select(3, 1):fill(255)
  expected:narrow(2, 33, 32):select(3, 2):fill(255)
  expected:narrow(2, 65, 32):select(3, 3):fill(255)
  assert(expected == image)
end

function tests:loadImageRGBA()
  local image = image.load(helpers.dirname(FILE_NAME) .. "data/testRGBA.png")
  local expected = tensor.ByteTensor(32 * 2, 32 * 3, 4)
  expected:narrow(2, 1, 32):select(3, 1):fill(255)
  expected:narrow(2, 33, 32):select(3, 2):fill(255)
  expected:narrow(2, 65, 32):select(3, 3):fill(255)
  expected:narrow(1, 1, 32):select(3, 4):fill(255)
  expected:narrow(1, 33, 32):select(3, 4):fill(127)
  assert(expected == image)
end

function tests:loadImageL()
  local image = image.load(helpers.dirname(FILE_NAME) .. "data/testL.png")
  local expected = tensor.ByteTensor(32, 32, 1)
  expected:applyIndexed(function (val, index)
      return index[1] + index[2] - 2
    end)
  assert(expected == image)
end

function tests:loadImageLFromString()
  local filename = helpers.dirname(FILE_NAME) .. "data/testL.png"
  local contents = game:loadFileToString(filename)
  local image = image.load('content:.png', contents)
  local expected = tensor.ByteTensor(32, 32, 1)
  expected:applyIndexed(function (val, index)
      return index[1] + index[2] - 2
    end)
  assert(expected == image)
end

return test_runner.run(tests)
