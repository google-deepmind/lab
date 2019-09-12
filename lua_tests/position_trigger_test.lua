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
local PositionTrigger = require 'common.position_trigger'
local set = require 'common.set'
local test_runner = require 'testing.test_runner'

local tests = {}

local HAPPINESS_LAYER = [[
..LL.LL..
.L..L..L.
..L...L..
...L.L...
....L....
]]
local UNHAPPY_POSITION = {50, 50}  -- bottom left
local HAPPY_POSITION = {150, 350}  -- left-most `L`

-- Update without any triggers.
function tests.testEmptyUpdate()
  local triggers = PositionTrigger.new()
  triggers:update(HAPPY_POSITION)
end

-- Create and remove a trigger multiple times.
function tests.testRemove()
  local fired = false
  local triggers = PositionTrigger.new()

  for i = 1, 3 do
    -- Create.
    assert(not triggers:exists("testTrigger"))
    triggers:start{
        name = 'testTrigger',
        maze = HAPPINESS_LAYER,
        triggerWhenEnter = 'L',
        callback = function() fired = true end,
    }
    assert(not fired)
    assert(triggers:exists("testTrigger"))

    -- Remove.
    triggers:remove("testTrigger")
    assert(not fired)
    assert(not triggers:exists("testTrigger"))
  end
end

-- Set up two triggers and verify the enter one fires when the first update is
-- on the trigger, and the exit one fires when the second update is off the
-- trigger.
function tests.testFirstUpdateTrigger()
  local triggers = PositionTrigger.new()

  -- Create trigger to fire .
  -- Ensure it doesn't fire immediately.
  local firedEnter = false
  local firedExit = false
  triggers:start{
      name = 'testEnterTrigger',
      maze = HAPPINESS_LAYER,
      triggerWhenEnter = 'L',
      callback = function() firedEnter = true end,
  }
  triggers:start{
      name = 'testExitTrigger',
      maze = HAPPINESS_LAYER,
      triggerWhenExit = 'L',
      callback = function() firedExit = true end,
  }
  assert(not firedEnter)
  assert(not firedExit)
  assert(triggers:exists("testEnterTrigger"))
  assert(triggers:exists("testExitTrigger"))

  -- Update position to one that should fire and check that it fired on enter
  -- trigger, but not on exit trigger.
  triggers:update(HAPPY_POSITION)
  assert(firedEnter)
  assert(not triggers:exists("testEnterTrigger"))
  assert(not firedExit)
  assert(triggers:exists("testExitTrigger"))

  -- Update position to one that's outside, and check that the exit trigger
  -- fired.
  triggers:update(UNHAPPY_POSITION)
  assert(firedExit)
  assert(not triggers:exists("testExitTrigger"))
end

-- Set up two triggers and verify that neither fire when the first update is
-- off the trigger, the enter one fires when the second update is on the
-- trigger, and the exit one fires when the third update is off the trigger.
function tests.testSecondUpdateTrigger()
  local triggers = PositionTrigger.new()

  -- Create trigger to fire .
  -- Ensure it doesn't fire immediately.
  local firedEnter = false
  local firedExit = false
  triggers:start{
      name = 'testEnterTrigger',
      maze = HAPPINESS_LAYER,
      triggerWhenEnter = 'L',
      callback = function() firedEnter = true end,
  }
  triggers:start{
      name = 'testExitTrigger',
      maze = HAPPINESS_LAYER,
      triggerWhenExit = 'L',
      callback = function() firedExit = true end,
  }
  assert(not firedEnter)
  assert(not firedExit)
  assert(triggers:exists("testEnterTrigger"))
  assert(triggers:exists("testExitTrigger"))

  -- Update position to one that's outside, and check that nothing triggers.
  triggers:update(UNHAPPY_POSITION)
  assert(not firedEnter)
  assert(not firedExit)
  assert(triggers:exists("testEnterTrigger"))
  assert(triggers:exists("testExitTrigger"))

  -- Update position to one that should fire and check that it fired on enter
  -- trigger, but not on exit trigger.
  triggers:update(HAPPY_POSITION)
  assert(firedEnter)
  assert(not triggers:exists("testEnterTrigger"))
  assert(not firedExit)
  assert(triggers:exists("testExitTrigger"))

  -- Update position to one that's outside, and check that the exit trigger
  -- fired.
  triggers:update(UNHAPPY_POSITION)
  assert(firedExit)
  assert(not triggers:exists("testExitTrigger"))
end

-- When we return true from the callback, the trigger should be reset so that
-- it fires again.
function tests.testPersist()
  local triggers = PositionTrigger.new()

  local fired = false
  triggers:start{
      name = 'testTrigger',
      maze = HAPPINESS_LAYER,
      triggerWhenEnter = 'L',
      callback = function()
          fired = true
          return true    -- request trigger to persist.
      end,
  }

  -- Update position and assert trigger gets triggered.
  assert(not fired)
  triggers:update(HAPPY_POSITION)
  assert(fired)

  -- Reset trigger, move off the position and back on.
  -- Assert trigger gets triggered again.
  fired = false
  triggers:update(UNHAPPY_POSITION)
  assert(not fired)
  triggers:update(HAPPY_POSITION)
  assert(fired)
end

--[[ Test starting and triggering multiple triggers in the same maze.

Key word argements:

*   `maze` (string) text representation of the maze.
*   `triggers` (array) values for triggerWhenEnter and triggerWhenExit for
    series of triggers.
*   `updatePositions` (array) {x,y} positions used in call to triggers:update().
*   `fired` (array) indices of {x,y} positions used in call to
    triggers:update().

--]]
local function testMultiple(kwargs)
  local triggers = PositionTrigger.new()

  -- Create triggers.
  local fired = {}
  for i = 1, #kwargs.triggers do
    triggers:start{
        name = "testTrigger" .. tostring(i),
        maze = kwargs.maze,
        triggerWhenEnter = kwargs.triggers[i].triggerWhenEnter,
        triggerWhenExit = kwargs.triggers[i].triggerWhenExit,
        callback = function()
            fired[i] = true
            return kwargs.triggers[i].persist
        end
    }
  end

  -- Repeatedly call triggers:update() and verify that the appropriate triggers
  -- have been fired.
  for j = 1, #kwargs.updatePositions do
    local currentPosition = kwargs.updatePositions[j]
    triggers:update(currentPosition)

    -- Ensure exactly the correct triggers have fired.
    assert(set.isSame(fired, kwargs.fired[j]))

    -- Reset which triggers were fired.
    fired = {}
  end
end

-- Set up multiple triggers that complete in the order they're created.
function tests.testMultipleInOrder()
  testMultiple{
      maze = 'CD\n' ..
             'AB',
      triggers = {
          {triggerWhenEnter = 'A'}, -- trigger 1
          {triggerWhenEnter = 'B'}, -- trigger 2
          {triggerWhenEnter = 'C'}, -- trigger 3
          {triggerWhenEnter = 'D'}, -- trigger 4
      },
      updatePositions = {
          {0, 0},
          {100, 0},
          {0, 100},
          {100, 100},
      },
      fired = {
          {[1] = true},
          {[2] = true},
          {[3] = true},
          {[4] = true},
      },
  }
end

-- Set up multiple triggers, where some trigger at the same time.
function tests.testMultipleSimultaneous()
  testMultiple{
      maze = 'AA\n' ..
             'AB',
      triggers = {
          {triggerWhenEnter = 'A'}, -- trigger 1
          {triggerWhenEnter = 'B'}, -- trigger 2
      },
      updatePositions = {
          {100, 100},
          {100, 0},
          {100, 0},
      },
      fired = {
          {[1] = true},
          {[2] = true},
          {},
      },
  }
end

-- Set up multiple triggers, where some trigger on exit.
function tests.testMultipleEnterExit()
  testMultiple{
      maze = 'AB',
      triggers = {
          {triggerWhenEnter = 'A'}, -- trigger 1
          {triggerWhenExit = 'B'}, -- trigger 2
          {triggerWhenExit = 'A'}, -- trigger 3
      },
      updatePositions = {
          {100, 0},
          {0, 0},
          {100, 0},
      },
      fired = {
          {},
          {[1] = true, [2] = true},
          {[3] = true},
      },
  }
end

-- Set up multiple triggers, where some are reset after they trigger.
function tests.testMultiplePersist()
  testMultiple{
      maze = 'AB',
      triggers = {
          {triggerWhenEnter = 'A', persist = true}, -- trigger 1
          {triggerWhenEnter = 'B', persist = true}, -- trigger 2
          {triggerWhenExit = 'A', persist = true}, -- trigger 3
          {triggerWhenExit = 'B', persist = true}, -- trigger 4
      },
      updatePositions = {
          {0, 0},
          {100, 0},
          {100, 0},
          {0, 0},
          {100, 0},
      },
      fired = {
          {[1] = true},
          {[2] = true, [3] = true},
          {},
          {[1] = true, [4] = true},
          {[2] = true, [3] = true},
      },
  }
end

return test_runner.run(tests)
