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
