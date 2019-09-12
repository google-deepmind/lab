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

local combinatorics = require 'common.combinatorics'
local set = require 'common.set'

local tests = {}

-- Return a set containing every possible result of
-- combinatorics.twoItemSelection(idx, n) for the given `n`.
local function allTwoItemSelections(n)
  local result = {}
  local resultSize = combinatorics.choose(2, n)
  for i = 1, resultSize do
    local selection = combinatorics.twoItemSelection(i, n)
    set.insert(result, {selection})
  end
  return result
end


--[[ Return true if `selections` contains all possible 2-number combinations of
the values 1..n.

Since selections is a set, items are unique. It's sufficient to check that
the size matches, and that all 2-number values are in the range 1..n.
--]]
local function isCompleteTwoItemSelection(selections, n)
  local s = set.toList(selections)

  -- Size should be "n choose 2".
  local expectedSize = combinatorics.choose(2, n)
  if expectedSize ~= #s then return false end

  -- All entries should satisfy 1 <= a < b <= n, for {a, b} in `selections`.
  for _, k in pairs(s) do
    local valid = 1 <= k[1] and k[1] < k[2] and k[2] <= n
    if not valid then return false end
  end

  return true
end

-- Test degenerate cases of the choose function.
function tests.choose_degenerate()
  -- "n choose n" = 1
  assert(combinatorics.choose(0, 0) == 1)
  assert(combinatorics.choose(1, 1) == 1)
  assert(combinatorics.choose(2, 2) == 1)
  assert(combinatorics.choose(100, 100) == 1)

  -- "n choose 0" = 1
  assert(combinatorics.choose(0, 1) == 1)
  assert(combinatorics.choose(0, 2) == 1)
  assert(combinatorics.choose(0, 100) == 1)

  -- "n choose 1" = n
  assert(combinatorics.choose(1, 1) == 1)
  assert(combinatorics.choose(1, 2) == 2)
  assert(combinatorics.choose(1, 100) == 100)

  -- "n choose n-1" = n
  assert(combinatorics.choose(1 - 1, 1) == 1)
  assert(combinatorics.choose(2 - 1, 2) == 2)
  assert(combinatorics.choose(100 - 1, 100) == 100)
end

-- Test more involved cases of the choose function.
function tests.choose_various()
  -- "4 choose 2" = 4! / (2! * 2!) = 6
  assert(combinatorics.choose(2, 4) == 6)

  -- "30 choose 5" = 30! / (25! * 5!) = 142506
  assert(combinatorics.choose(5, 30) == 142506)
end

-- Generate all possible values for a range of `n`s, and verify with brute force
-- that they're correct.
function tests.twoItemSelection_smallN()
  local nToTest = {2, 3, 4, 9, 15, 16}
  for _, n in ipairs(nToTest) do
    assert(isCompleteTwoItemSelection(allTwoItemSelections(n), n))
  end
end

return test_runner.run(tests)
