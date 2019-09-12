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
local geometric_pickups = require 'common.geometric_pickups'
local random = require 'common.random'

local tests = {}

local CounterMT = {
    add = function(self, k) self[k] = (self[k] or 0) + 1 end
}

local function Counter()
  local counter = {}
  setmetatable(counter, CounterMT)
  return counter
end

function tests.randomPickupsTest()
  random:seed(1)
  local allPickups = geometric_pickups.createPickups()
  for nTypes = 1, #allPickups do
    for nRepeats = 1, 10 do
      local keys = geometric_pickups.randomPickups(allPickups, nTypes, nRepeats)
      asserts.EQ(nTypes * nRepeats, #keys)
      local counter = Counter()
      for _, k in ipairs(keys) do
        counter:add(k)
      end
      for k, count in pairs(counter) do
        asserts.EQ(nRepeats, count)
      end
    end
  end
end

return test_runner.run(tests)
