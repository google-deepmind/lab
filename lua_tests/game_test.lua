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
