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
local random = require 'common.random'
local test_runner = require 'testing.test_runner'
local map_maker = require 'dmlab.system.map_maker'
local randomMap = random(map_maker:randomGen())
local set = require 'common.set'

local tests = {}

function tests.testRandomChoice()
  random:seed(1)
  asserts.EQ(random:choice({1, 1, 1}), 1)
  asserts.EQ(random:choice({1, 2, 2, 1}, 2, 3), 2)
  asserts.EQ(random:choice(nil), nil)
  asserts.EQ(random:choice({}), nil)
  asserts.EQ(random:choice({1, 2, 2, 1}, 3, 2), nil)
end

function tests.testRandomChoiceRandomness()
  local t = {1, 2, 3}
  local count = {0, 0, 0}
  random:seed(1)
  for _ = 1, 100 do
    local val = random:choice(t)
    count[val] = count[val] + 1
  end
  asserts.EQ(count[1] + count[2] + count[3], 100)
  assert(count[1] > 0)
  assert(count[2] > 0)
  assert(count[3] > 0)
end

function tests.testShuffle()
  random:seed(2)
  asserts.tablesEQ(random:shuffle{}, {})
  asserts.tablesEQ(random:shuffle{1}, {1})
  local shuffled = random:shuffle{1, 2, 3, 4, 5}
  asserts.tablesNE(shuffled, {1, 2, 3, 4, 5})
  table.sort(shuffled)
  asserts.tablesEQ(shuffled, {1, 2, 3, 4, 5})
end

function tests.testShuffleInPlace()
  random:seed(2)
  local seq = {}
  random:shuffleInPlace(seq)
  asserts.tablesEQ(seq, {})
  seq = {1}
  random:shuffleInPlace(seq)
  asserts.tablesEQ(seq, {1})
  seq = {1, 2, 3, 4, 5}
  random:shuffleInPlace(seq)
  asserts.tablesNE(seq, {1, 2, 3, 4, 5})
  table.sort(seq)
  asserts.tablesEQ(seq, {1, 2, 3, 4, 5})
end

function tests.SameSqueunce()
  for seed = 1, 100 do
    random:seed(seed)
    randomMap:seed(seed)
    for seq = 1, 1000 do
      asserts.EQ(random:normalDistribution(0, 1000),
                 randomMap:normalDistribution(0, 1000))
    end
  end
end

function tests.testShuffleMap()
  randomMap:seed(1)
  random:seed(1)
  asserts.tablesEQ(randomMap:shuffle{1, 2, 3, 4, 5},
                   random:shuffle{1, 2, 3, 4, 5})
  randomMap:seed(1)
  random:seed(2)
  asserts.tablesNE(randomMap:shuffle{1, 2, 3, 4, 5},
                   random:shuffle{1, 2, 3, 4, 5})
end

function tests.testShuffleInPlaceN()
  random:seed(2)
  local seq = {}
  random:shuffleInPlace(seq, 1)
  asserts.tablesEQ(seq, {})
  seq = {1}
  random:shuffleInPlace(seq, 1)
  asserts.tablesEQ(seq, {1})
  seq = {1, 2, 3, 4, 5, 6}
  random:shuffleInPlace(seq, 1)
  asserts.tablesNE(seq, {1, 2, 3, 4, 5, 6})
  table.sort(seq)
  asserts.tablesEQ(seq, {1, 2, 3, 4, 5, 6})
end

function tests.testNormal()
  random:seed(2)
  local norm = math.floor(random:normalDistribution(0, 1000))
  -- When complied with libstdc++
  local libstdcxxExpected = -592
  -- When complied with libc++
  local libcxxExpected = -402
  assert(norm == libstdcxxExpected or norm == libcxxExpected,
         'Norm actual ' .. norm)

  asserts.shouldFail(function () random:normalDistribution(3, 'cat') end)
  asserts.shouldFail(function () random:normalDistribution('cat', 3) end)
  asserts.shouldFail(function () random:normalDistribution('cat', 'pig') end)

  asserts.shouldFail(function () random:normalDistribution(nil, 3) end)
  asserts.shouldFail(function () random:normalDistribution(0, nil) end)
  asserts.shouldFail(function () random:normalDistribution(nil, nil) end)
end

function tests.testDiscreteDistribution()
  random:seed(2)
  for i = 1, 10 do
    asserts.EQ(random:discreteDistribution{0, 1}, 2)
    asserts.EQ(random:discreteDistribution{0.5, 0}, 1)
    asserts.EQ(random:discreteDistribution{0, 0, 5}, 3)
  end

  local hits = {0, 0}
  for i = 1, 100 do
    local index = random:discreteDistribution{1, 1}
    assert(index == 1 or index == 2)
    hits[index] = hits[index] + 1
  end

  -- Should have roughly 50-50 hits from equal weights
  local ratio = hits[1] / hits[2]
  asserts.GT(ratio, 0.8)
  asserts.LT(ratio, 1.2)

  asserts.shouldFail(function () random:discreteDistribution({}) end)
  asserts.shouldFail(function () random:discreteDistribution(3) end)
  asserts.shouldFail(function () random:discreteDistribution('spoon') end)
  asserts.shouldFail(function () random:discreteDistribution({1, 'x'}) end)
end

-- Ensure that generating a complete shuffling generates a valid shuffling.
-- Also tests that we return nil after a complete shuffling.
function tests.testShuffledIndexGeneratorShuffling()
  local counts = {1, 2, 3, 10, 16, 100, 997}

  for _, count in ipairs(counts) do
    random:seed(1)
    local gen = random:shuffledIndexGenerator(count)

    -- Generate a complete shuffling of all numbers 1..count.
    local result = {}
    for i = 1, count do
      local index = gen()
      -- Generated values must be in the shuffling range.
      assert(1 <= index and index <= count)
      set.insert(result, {index})
    end

    -- After a complete shuffling, the generator should return nil for one
    -- call.
    assert(gen() == nil)

    -- The next call should not be nil, however.
    local nextShuffleFirstValue = gen()
    assert(nextShuffleFirstValue ~= nil and 1 <= nextShuffleFirstValue and
           nextShuffleFirstValue <= count)

    -- Set values are unique and in the range 1..count, resultList will be
    -- a shuffling if and only if it is the correct length.
    local resultList = set.toList(result)
    assert(#resultList == count)
  end
end

-- Test how random the 1st number generated is. Also test the 2nd and later
-- numbers, too.
function tests.testShuffledIndexGeneratorRandomness()
  random:seed(2)

  local indices = {1, 2, 7}
  local count = 4

  -- The first value generated should be a number between 1..4, with uniform
  -- distribution. We sample the first value `trials` times.
  --
  -- We expect 1/4 of our tests to return value 1 the first time we call gen().
  -- For example, if trials is 4000, then we expect about 1000 to return 1.
  -- The probability of 890~1110 tests returning 1 is 0.999948 so we use 110
  -- as our error tolerance.
  -- This probability is calculated with a Binomial Calculator
  -- (e.g. http://stattrek.com/online-calculator/binomial.aspx),
  -- using probability-of-success=1/4, trials=4000, successes=890.
  local trials = 4000
  local expectedValue = trials / count
  local errorTolerance = 110

  -- We test the randomness of the first value generated, but we also want to
  -- test the second and others, too.
  for _, index in ipairs(indices) do
    -- Count how often each value is generated as the index'th value.
    local occurrance = {}
    for value = 1, count do
      occurrance[value] = 0
    end

    for trial = 1, trials do
      local gen = random:shuffledIndexGenerator(count)

      -- Grab the index'th value generated.
      for i = 1, index - 1 do gen() end
      local value = gen()
      assert(1 <= value and value <= count)

      -- Increment the count of occurrences.
      occurrance[value] = (occurrance[value] or 0) + 1
    end

    -- Pass if the occurrences are within our error tolerances.
    for value = 1, count do
      local occurrances = occurrance[value] or 0
      assert(expectedValue - errorTolerance <= occurrances and
             occurrances <= expectedValue + errorTolerance)
    end
  end
end

return test_runner.run(tests)
