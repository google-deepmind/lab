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
local helpers = require 'common.helpers'
local test_runner = require 'testing.test_runner'
local Timer = require 'common.timer'

local tests = {}

-- Create and remove a timer multiple times.
function tests.testRemove()
  local fired = false
  local timer = Timer.new()

  for i = 1, 3 do
    -- Create.
    assert(not timer:exists("testTimer"))
    timer:start{
        name = "testTimer",
        time = 2,
        callback = function() fired = true end,
    }
    assert(not fired)
    assert(timer:exists("testTimer"))

    -- Remove.
    timer:remove("testTimer")
    assert(not fired)
    assert(not timer:exists("testTimer"))
  end
end

-- Set up one timer and verify it fires when time is exactly correct.
function tests.testSingleExact()
  local timer = Timer.new()

  -- Create timer to fire at second 1.
  -- Ensure it doesn't fire immediately.
  assert(not timer:exists("testTimer"))
  local fired = false
  timer:start{
      name = "testTimer",
      time = 1,
      callback = function() fired = true end,
  }
  assert(not fired)
  assert(timer:exists("testTimer"))
  asserts.EQ(timer:timeRemaining("testTimer"), 1)

  -- Update time to exactly 1 and verify that timer fired.
  timer:update(1)
  assert(fired)
  assert(not timer:exists("testTimer"))
  asserts.EQ(timer:timeRemaining("testTimer"), 0)
end

-- Set up one timer and verify it fires when time is exactly correct.
function tests.testSingleOver()
  local timer = Timer.new()

  -- Create timer to fire at second 1.
  assert(not timer:exists("testTimer"))
  local fired = false
  timer:start{
      name = "testTimer",
      time = 1,
      callback = function() fired = true end,
  }

  -- Nothing should happen here, since we're < the time for testTimer.
  timer:update(0.99999)
  assert(not fired)
  assert(timer:exists("testTimer"))
  asserts.EQ(timer:timeRemaining("testTimer"), 1 - 0.99999)

  -- Timer should fire now.
  timer:update(1000)
  assert(fired)
  assert(not timer:exists("testTimer"))
  asserts.EQ(timer:timeRemaining("testTimer"), 0)
end

-- Set up one timer and verify it fires over and over again when persisting.
function tests.testPersist()
  local timer = Timer.new()

  -- Create timer to fire at second 1.
  assert(not timer:exists("testTimer"))
  local fired = false
  local persist = true
  timer:start{
      name = "testTimer",
      time = 1,
      callback = function()
          fired = true
          return persist  -- Indicate if this timer should be set again.
      end,
  }

  -- Assert the timer fires but remains active.
  timer:update(1)
  assert(fired)
  assert(timer:exists("testTimer"))
  asserts.EQ(timer:timeRemaining("testTimer"), 1)

  -- Update some more and assert it fires again.
  fired = false
  timer:update(2.0001)
  assert(fired)

  -- Update not quite enough and assert it doesn't fire.
  fired = false
  timer:update(2.9999)
  assert(not fired)

  -- Update again, this time requesting it not to persist.
  persist = false
  timer:update(3)
  assert(fired)
  assert(not timer:exists("testTimer"))
end

-- `timerTimes` (array) times that timers are fired off at.
-- `updateTimes` (array) times used in call to timer:update(). Must be
-- increasing.
local function testMultiple(kwargs)
  local timer = Timer.new()

  -- Create timers.
  local fired = {}
  for i = 1, #kwargs.timerTimes do
    fired[i] = false
    timer:start{
        name = "testTimer" .. tostring(i),
        time = kwargs.timerTimes[i],
        callback = function() fired[i] = true end,
    }
  end

  -- Repeatedly call timer:update() and verify that the appropriate timers
  -- have been fired.
  for j = 1, #kwargs.updateTimes do
    local currentTime = kwargs.updateTimes[j]
    timer:update(currentTime)
    for i = 1, #kwargs.timerTimes do
      local timerTime = kwargs.timerTimes[i]
      local shouldBeComplete = timerTime <= currentTime
      asserts.EQ(fired[i], shouldBeComplete)
      asserts.EQ(timer:exists("testTimer" .. tostring(i)), not shouldBeComplete)
    end
  end
end

-- Set up multiple timers that complete in the order they're created.
function tests.testMultipleInOrder()
  testMultiple{
    timerTimes = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
    updateTimes = {0, 1, 3, 3.1, 5, 8.1, 11},
  }
end

-- Set up multiple timers that complete in the reverse order they're created.
function tests.testMultipleReverseOrder()
  testMultiple{
    timerTimes = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1},
    updateTimes = {0, 1, 3, 3.1, 5, 8.1, 11},
  }
end

-- Set up multiple timers that complete in a mixed up order.
function tests.testMultipleMixedOrder()
  testMultiple{
    timerTimes = {4.2, 3.3, 8, 2, 2, 3.1, 5, 8, 4.2, 10},
    updateTimes = {0, 1, 3, 3.1, 5, 8.1, 11},
  }
end

-- Test formatting.
function tests.testSecondsToString()
  asserts.EQ(helpers.secondsToTimeString(0), "0")
  asserts.EQ(helpers.secondsToTimeString(3), "3")
  asserts.EQ(helpers.secondsToTimeString(21), "21")
  asserts.EQ(helpers.secondsToTimeString(59), "59")
  asserts.EQ(helpers.secondsToTimeString(60), "1:00")
  asserts.EQ(helpers.secondsToTimeString(61), "1:01")
  asserts.EQ(helpers.secondsToTimeString(61.1), "1:02")
  asserts.EQ(helpers.secondsToTimeString(60 * 2), "2:00")
  asserts.EQ(helpers.secondsToTimeString(60 * 60), "1:00:00")
  asserts.EQ(helpers.secondsToTimeString(60 * 60 + 1), "1:00:01")
  asserts.EQ(helpers.secondsToTimeString(60 * 60 + 61), "1:01:01")
  asserts.EQ(helpers.secondsToTimeString(100 * 60 * 60), "100:00:00")
end

return test_runner.run(tests)
