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

local color_dataset = require 'datasets.color_dataset'
local selector = require 'datasets.selector'

local tests = {}

function tests.colours()
  local ds = color_dataset(4, 4, 5000)
  asserts.EQ(5000, ds:getSize())
  local image1 = ds:getImage(1)
  asserts.tablesEQ({4, 4, 3}, image1:shape())
  local imageLast = ds:getImage(ds:getSize())
  asserts.tablesEQ({4, 4, 3}, imageLast:shape())
  asserts.NE(image1, imageLast)
end

function tests.selector()
  local ds = selector.loadDataset('color')
  asserts.EQ(1500, ds:getSize())
  local image1 = ds:getImage(1)
  asserts.tablesEQ({8, 8, 3}, image1:shape())
  local imageLast = ds:getImage(ds:getSize())
  asserts.tablesEQ({8, 8, 3}, imageLast:shape())
  asserts.NE(image1, imageLast)
end

return test_runner.run(tests)
