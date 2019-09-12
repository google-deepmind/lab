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
