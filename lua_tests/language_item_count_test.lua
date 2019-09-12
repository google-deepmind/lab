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
local item_count = require 'language.item_count'
local random = require 'common.random'
local set = require 'common.set'

local function accumulate(seq)
  local sum = 0
  for _, value in pairs(seq) do sum = sum + value end
  return sum
end

local tests = {}

function tests.concat_shouldReturnFunction()
  local countGenerator = item_count.concat({1}, {2,3}) --
  asserts.EQ(type(countGenerator), 'function')
end

function tests.concat_shouldConcatListsOnCall()
  asserts.tablesEQ(item_count.concat({1, 3}, {2, 4})(), {1, 3, 2, 4})
  asserts.tablesEQ(item_count.concat({1, 3}, {}, {2, 4})(), {1, 3, 2, 4})
  asserts.tablesEQ(item_count.concat({1}, {2, 4}, {9, 7})(), {1, 2, 4, 9, 7})
end

function tests.concat_shouldCallFunctionArgsOnCall()
  local called = false
  local function func()
    called = true
    return {1}
  end

  local countGenerator = item_count.concat(func)
  asserts.tablesEQ(countGenerator(), {1})
  asserts.EQ(called, true)
end

function tests.concat_shouldConcatListAndFunctionResults()
  local function func()
    return {2, 3}
  end

  local countGenerator = item_count.concat({1,1}, func, func, {44})
  asserts.tablesEQ(countGenerator(), {1, 1, 2, 3, 2, 3, 44})
end

function tests.createGroupCounts_ShouldFailUnlessOneOfTotalOrMaxGiven()
  asserts.shouldFail(function ()
        item_count.createGroupCounts{totalObjects = 4,
                                     maxObjects = 4,
                                     groupMin = {1}}
  end)
  asserts.shouldFail(function ()
        item_count.createGroupCounts{groupMin = {1}}
  end)
end

function tests.createGroupCounts_ShouldFailIfGroupMaxSetAndNotSameSizeAsMin()
  asserts.shouldFail(function ()
        item_count.createGroupCounts{totalObjects = 4,
                                     groupMin = {1},
                                     groupMax = {3, 1},
        }
  end)
  asserts.shouldFail(function ()
        item_count.createGroupCounts{totalObjects = 4,
                                     groupMin = {1, 2},
                                     groupMax = {4},
        }
  end)
end

function tests.createGroupCounts_ShouldFailIfGroupMaxLessThanCorrespondingMin()
  asserts.shouldFail(function ()
        item_count.createGroupCounts{totalObjects = 6,
                                     groupMin = {3},
                                     groupMax = {2},
        }
  end)
end

function tests.createGroupCounts_ShouldFailIfGroupMaxSumsToLessThanTotal()
  asserts.shouldFail(function ()
        item_count.createGroupCounts{totalObjects = 6,
                                     groupMin = {1, 1},
                                     groupMax = {3, 2},
        }
  end)
  asserts.shouldFail(function ()
        item_count.createGroupCounts{maxObjects = 5,
                                     groupMin = {1, 2},
                                     groupMax = {4},
        }
  end)
end

function tests.createGroupCounts_ResultShouldHaveSameSizeAsGroupMin()
  for _, size in ipairs({1, 2, 3, 7, 20}) do
    local groupMin = {}
    for ii = 1, size do groupMin[ii] = 1 end

    local gen = item_count.createGroupCounts{totalObjects = size * 2,
                                             groupMin = groupMin}
    for _ = 1, 5 do
      asserts.EQ(#gen(), size)
    end
  end
end

function tests.createGroupCounts_OneGroupWithTotalShouldReturnTotal()
  local TOTAL = 4
  for min = 0, TOTAL do
    local gen = item_count.createGroupCounts{totalObjects = TOTAL,
                                             groupMin = {min}
    }
    for _ = 1, 5 do
      asserts.tablesEQ(gen(), {TOTAL})
    end
  end
end

function tests.createGroupCounts_OneGroupWithMaxShouldReturnInRange()
  local MAX = 4
  for min = 0, MAX do
    -- Force a seed to ensure the test doesn't flake
    random:seed(1)
    local gen = item_count.createGroupCounts{maxObjects = MAX,
                                             groupMin = {min}
    }
    local seenValues = {}
    -- Ensure generated value is in range
    for _ = 1, 20 do
      local value = gen()[1]
      asserts.GE(value, min)
      asserts.LE(value, MAX)
      seenValues[value] = 1
    end
    -- Ensure all values in possible range were returned.
    asserts.EQ(accumulate(seenValues), MAX - min + 1)
  end
end

function tests.createGroupCounts_SumOfResultsShouldEqualTotal()
  for _, size in ipairs({1, 2, 3, 7, 20}) do
    local groupMin = {}
    for ii = 1, size do groupMin[ii] = 0 end

    local totalObjects = random:uniformInt(1, size * 3)
    local gen = item_count.createGroupCounts{totalObjects = totalObjects,
                                             groupMin = groupMin}
    for _ = 1, 20 do
      asserts.EQ(accumulate(gen()), totalObjects)
    end
  end
end

function tests.createGroupCounts_ResultsShouldHonourMin()
  for _, size in ipairs({1, 2, 3, 7, 20}) do
    local groupMin = {}
    for ii = 1, size do groupMin[ii] = random:uniformInt(0, size) end
    local minSum = accumulate(groupMin)

    -- If total < sum of minimum values, should get an error on construction.
    asserts.shouldFail(function ()
          item_count.createGroupCounts{totalObjects = minSum - 1,
                                       groupMin = groupMin}
    end)

    -- If the total == sum of minimum values, should get the min values back.
    local gen = item_count.createGroupCounts{totalObjects = minSum,
                                             groupMin = groupMin}
    local values = gen()
    for ii = 1, size do
      asserts.EQ(values[ii], groupMin[ii])
    end

    -- If the total > sum of minimum values, should get the min or higher back.
    gen = item_count.createGroupCounts{totalObjects = minSum * 2,
                                       groupMin = groupMin}
    values = gen()
    for ii = 1, size do
      asserts.GE(values[ii], groupMin[ii])
    end

    -- The same should be true if using max not total.
    gen = item_count.createGroupCounts{maxObjects = minSum * 2,
                                       groupMin = groupMin}
    for _ = 1, 5 do
      values = gen()
      for ii = 1, size do
        asserts.GE(values[ii], groupMin[ii])
      end
    end
  end
end

function tests.createGroupCounts_ResultsShouldHonourGroupMax()
  for _, size in ipairs({1, 2, 3, 7, 20}) do
    local groupMin = {}
    local groupMax = {}
    for ii = 1, size do
      groupMin[ii] = random:uniformInt(0, size)
      groupMax[ii] = random:uniformInt(groupMin[ii], size * 2)
    end
    local minSum = accumulate(groupMin)
    local maxSum = accumulate(groupMax)

    -- Set total between min and max values, and ensure results are within the
    -- bounds for each position.
    local total = math.floor(minSum + (maxSum - minSum) / 2)
    local gen = item_count.createGroupCounts{totalObjects = total,
                                             groupMin = groupMin,
                                             groupMax = groupMax}
    local values = gen()
    for ii = 1, size do
      asserts.GE(values[ii], groupMin[ii])
      asserts.LE(values[ii], groupMax[ii])
    end
  end
end

function tests.createGroupCounts_SumOfResultsShouldHonourMax()
  for _, size in ipairs({1, 2, 3, 7, 20}) do
    local groupMin = {}
    for ii = 1, size do
      groupMin[ii] = random:uniformInt(0, size)
    end
    local minSum = accumulate(groupMin)

    -- If max < sum of minimum values, should get an error on construction.
    asserts.shouldFail(function ()
          item_count.createGroupCounts{maxObjects = minSum - 1,
                                       groupMin = groupMin}
    end)

    -- If max == sum of minimum values, should get the min values back.
    local gen = item_count.createGroupCounts{maxObjects = minSum,
                                             groupMin = groupMin}
    local values = gen()
    for ii = 1, size do
      asserts.EQ(values[ii], groupMin[ii])
    end

    -- If max > sum of minimum values, sum of results should be between minSum
    -- and max.
    local max = minSum * 2
    gen = item_count.createGroupCounts{maxObjects = max,
                                       groupMin = groupMin}
    for _ = 1, 5 do
      local sum = accumulate(gen())
      asserts.GE(sum, minSum)
      asserts.LE(sum, max)
    end
  end
end

function tests.createGroupCounts_EqualGroupMinAndMaxShouldReturnSame()

  random:seed(1)
  local gen = item_count.createGroupCounts{totalObjects = 10,
                                           groupMin = {1, 2, 3, 4},
                                           groupMax = {1, 2, 3, 4}
  }
  local out = gen()
  asserts.tablesEQ(out, {1, 2, 3, 4})
end

function tests.createGroupCounts_GroupMaxIsResultIfTotalEqualsSumOfGroupMax()
  local groupMax = {1, 2}
  random:seed(1)
  local out = item_count.createGroupCounts{totalObjects = accumulate(groupMax),
                                           groupMin = {0, 0},
                                           groupMax = groupMax}()
  asserts.tablesEQ(out, groupMax)

  groupMax = {7, 9, 3, 1, 0}
  out = item_count.createGroupCounts{totalObjects = accumulate(groupMax),
                                     groupMin = {0, 0, 0, 0, 0},
                                     groupMax = groupMax}()
  asserts.tablesEQ(out, groupMax)
end

function tests.createGroupCounts_CheckEvenDistribution()
  local counter = item_count.createGroupCounts{
      totalObjects = 2,
      groupMin = {0, 0, 0},
      groupMax = {1, 1, 1}
  }
  local TRIALS = 900
  local DISTINCT_VALUES = 3
  local EXPECTED_HITS = TRIALS / DISTINCT_VALUES
  local TOLERANCE = EXPECTED_HITS * 0.11 -- 11% tolerance

  for seed = 1, 10 do
    random:seed(seed)

    local results = {}
    for i = 1, TRIALS do
      local val = counter()
      local key = string.format('%03d', val[1] * 100 + val[2] * 10 + val[3])
      results[key] = (results[key] or 0) + 1
    end

    for key, count in pairs(results) do
      asserts.GE(count, EXPECTED_HITS - TOLERANCE)
      asserts.LE(count, EXPECTED_HITS + TOLERANCE)
    end
  end
end

return test_runner.run(tests)
