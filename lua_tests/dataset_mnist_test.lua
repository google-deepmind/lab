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

local mnist = require 'datasets.mnist'

local tests = {}

function tests.mnistTrain()
  local ds = mnist{}
  asserts.EQ(60000, ds:getSize())

  local image1 = ds:getImage(1)
  asserts.tablesEQ({28, 28, 3}, image1:shape())
  local imageLast = ds:getImage(ds:getSize())
  asserts.tablesEQ({28, 28, 3}, imageLast:shape())
  asserts.NE(image1, imageLast)

  local label1 = ds:getLabel(1)
  local labelLast = ds:getLabel(ds:getSize())
  asserts.NE(label1, labelLast)
end

function tests.mnistTest()
  local ds = mnist{test = true}
  asserts.EQ(10000, ds:getSize())
  local image1 = ds:getImage(1)
  asserts.tablesEQ({28, 28, 3}, image1:shape())
  local imageLast = ds:getImage(ds:getSize())
  asserts.tablesEQ({28, 28, 3}, imageLast:shape())
  asserts.NE(image1, imageLast)

  local label1 = ds:getLabel(1)
  local labelLast = ds:getLabel(ds:getSize())
  asserts.NE(label1, labelLast)
end

return test_runner.run(tests)
