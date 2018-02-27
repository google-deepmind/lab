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

local game = require 'dmlab.system.game'
local helpers = require 'common.helpers'
local random = require 'common.random'

--[[ Customise the pickup actions and reward logic for language levels.

See language_factory for a more complete picture.

A reward controller is expected to implement the following API:

*   init(task, taskData) -- Init will be called each time a new task is started,
    and passed the processed task description from which it may choose to
    extract any needed state.  taskData may include generated state for this
    instance of the task (since task descriptions are intended to be
    read-only).

*   handlePickup(description) required -- When an agent collides with a pickup,
    this function will be called with the table describing the pickup object
    passed as an argument, as described in language.object_generator.
]]
local reward_controllers = {}

-- Supports mocking 'game' for tests.
function reward_controllers.setGameObject(newGame)
  game = newGame
end

-- The simple controller is for tasks like 'Get the hat'.
-- Any object collected will award 'reward' points, and end the level.
function reward_controllers.createSimple()
  local simpleRewardController = {}

  function simpleRewardController:init(task, taskData) end

  function simpleRewardController:handlePickup(description)
    game:finishMap()
    game:addScore(description.reward)
    return -1 -- Prevent re-spawning of pickup.
  end

  return simpleRewardController
end

local function calculateBalancedNegativeReward(goalGroup, goalReward,
                                               descriptions)
  local numDistractors = 0
  for index, description in ipairs(descriptions) do
    if description._group ~= goalGroup then
      numDistractors = numDistractors + 1
    end
  end
  return -math.floor(goalReward / numDistractors + 0.5)
end


--[[ A balanced reward controller gives `goalReward` for collecting the goal
object.  The reward for a distractor is -(goalReward / #distractors).

If the distractor reward would be non-integral, use whichever of the upper or
lower possible values would be closer to the goalReward if collected
#numDistractors times.
]]
function reward_controllers.createBalanced(goalReward)
  local balancedRewardController = {}

  function balancedRewardController:init(task, taskData)
    self._goalGroup = task.goalGroup or 1
    self._descriptions = taskData.descriptions
  end

  function balancedRewardController:handlePickup(description)
    local reward
    if description._group == self._goalGroup then
      reward = goalReward
    else
      reward = calculateBalancedNegativeReward(self._goalGroup, goalReward,
                                               self._descriptions)
    end

    game:finishMap()
    game:addScore(reward)
    return -1 -- Prevent re-spawning of pickup.
  end

  return balancedRewardController
end


--[[ The counting controller is for tasks like 'Get all the hats'.

*   Each object collected will award 'reward' points.
*   The final goal object will award 'reward + reward 2' points and end the
    level.
*   Any non-goal object collected will end the level.
]]
local function createCountingImpl(balanced)
  local countingRewardController = {}

  function countingRewardController:init(task, taskData)
    self._goalGroup = task.goalGroup or 1
    self._pickupCount = 0
    if balanced then
      self._descriptions = taskData.descriptions
    end
  end

  function countingRewardController:_calculateGoalReward()
    for _, description in ipairs(self._descriptions) do
      if description._group == self._goalGroup then
        return description.reward * description._groupSize + description.reward2
      end
    end
  end

  function countingRewardController:handlePickup(description)
    local finishMap = true
    local reward = description.reward

    if description._group == self._goalGroup then
      self._pickupCount = self._pickupCount + 1
      if self._pickupCount >= description._groupSize then
        reward = reward + (description.reward2 or 0)
      else
        finishMap = false
      end
    elseif balanced then
      reward = calculateBalancedNegativeReward(
          self._goalGroup, self:_calculateGoalReward(), self._descriptions)
    end

    game:addScore(reward)
    if finishMap then
      game:finishMap()
    end
    return -1 -- Prevent re-spawning of pickup.
  end

  return countingRewardController
end


--[[ The counting controller is for tasks like 'Get all the hats'.

*   Each object collected will award 'reward' points.
*   The final goal object will award 'reward + reward2' points and end the
    level.
*   Any non-goal object collected will give the reward associated with that
    object and end the level.
]]
function reward_controllers.createCounting()
  return createCountingImpl(false)
end


--[[ The balanced counting controller is for tasks like 'Get all the hats'.

*   It behaves like the the Counting controller, except that the reward for
    collecting a distractor is calculated as a function of the total goal reward
    and the number of distractors, like the balancedRewardController.
]]
function reward_controllers.createBalancedCounting()
  return createCountingImpl(true)
end



--[[ The ordered controller is for tasks like 'get the hat and then the pig'. It
is constructed with a list of group ids in the order which they should be
collected.

*   Objects collected in the correct order will award 'reward' points.
    Otherwise 'reward2' points will be awarded.
*   The level will end if any incorrect object is picked up, or when the last
    goal object is collected.
]]
function reward_controllers.createOrdered(order)
  local orderedRewardController = {}

  orderedRewardController._requiredOrder = order

  function orderedRewardController:init(task, taskData)
    self._nextIndex = 1
  end

  function orderedRewardController:handlePickup(description)
    local finishMap = true
    local scoreChange = description.reward2  -- Assume penalty

    -- If picked up in the correct order, add reward to score.
    if description._group == self._requiredOrder[self._nextIndex] then
      scoreChange = description.reward or 0
      if self._nextIndex < #self._requiredOrder then
        self._nextIndex = self._nextIndex + 1
        finishMap = false
      end
    end

    game:addScore(scoreChange)
    if finishMap then
      game:finishMap()
    end
    return -1 -- Prevent re-spawning of pickup.
  end

  return orderedRewardController
end


--[[ The answer controller is for tasks which ask a question of the agent.

The controller maps object groups to an associated answer. It also has a
function to calculate the correct answer for the current task.  Comparing the
two based on the group of a picked up object determines whether positive or
negative reward is given.

Keyword Arguments:

*   `groupToAnswer` (table) Maps from a group number to the associated answer.
*   `truth` (function) Called on pickup, with the number of objects generated
    per group for the current task instance as a parameter.
*   `reward` (number, default 10) Reward for correct answer. -reward used if
    incorrect.
]]
function reward_controllers.createAnswer(kwargs)
  assert(kwargs and type(kwargs) == 'table')
  assert(type(kwargs.truth) == 'function')
  assert(type(kwargs.groupToAnswer) == 'table')

  local answerController = {
      _truthFn = kwargs.truth,
      _groupToAnswer = kwargs.groupToAnswer,
      _reward = kwargs.reward or 10
  }

  function answerController:init(task, taskData)
    self._objectsPerGroup = taskData.objectsPerGroup
  end

  function answerController:handlePickup(description)
    local answer = self._groupToAnswer[description._group]
    if answer == nil then
      error('Picked up an object, but do not have a mapping for meaning.' ..
                helpers.tostring(description))
    end

    local correctAnswer = self._truthFn(self._objectsPerGroup)
    local reward = answer == correctAnswer and self._reward or -self._reward
    game:addScore(reward)
    game:finishMap()
    return -1
  end

  return answerController
end

return reward_controllers
