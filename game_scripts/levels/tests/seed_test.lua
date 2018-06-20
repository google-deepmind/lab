--[[ Copyright (C) 2018 Google Inc.

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
local random = require 'common.random'

local tests = {}
local testLevelScript = false

function tests.testSeed()
  assert(testLevelScript)

  local seed = 2

  local api = require('levels.' .. testLevelScript:gsub('/', '.'))
  api:init{allowHoldOutLevels = 'true', datasetPath = 'dummy'}
  api:start(0, seed)
  local rand1 = random:uniformReal(0, 1)
  local rand2 = random:uniformReal(0, 1)

  -- If we generate the exact same random numbers again then it's very likely
  -- that the random number generated was seeded.
  api:start(0, seed)
  asserts.EQ(rand1, random:uniformReal(0, 1))
  asserts.EQ(rand2, random:uniformReal(0, 1))
end

local run_tests = test_runner.run(tests)

-- Override the test runner initialiser to set the name of the level to test.
local init = run_tests.init
function run_tests:init(params)
  assert(params.testLevelScript)
  testLevelScript = params.testLevelScript

  return init(run_tests, params)
end

return run_tests
