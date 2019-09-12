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
local random = require 'common.random'
local reward_controllers = require 'language.reward_controllers'
local set = require 'common.set'

local tests = {}

local function createGameMock()
  local mock = {}
  mock._callCounts = {}
  mock._rewardGiven = {}

  function mock:_incrementCallCount(name)
    self._callCounts[name] = self:callCount(name) + 1
  end

  function mock:callCount(name)
    return self._callCounts[name] or 0
  end

  function mock:rewardGiven(player)
    return self._rewardGiven[player or 1] or 0
  end


  function mock:finishMap()
    self:_incrementCallCount('finishMap')
  end

  -- Note `player` is optional.
  function mock:addScore(...)
    local args = {...}
    local player, score
    if #args == 2 then
      player, score = unpack(args)
    elseif #args == 1 then
      player = 1
      score = unpack(args)
    else
      error("Invalid argument count sent to addScore.")
    end
    self:_incrementCallCount('addScore')
    self._rewardGiven[player] = self:rewardGiven(player) + score
  end

  return mock
end

-- Appends `element` to `seq` `count` times.
local function appendN(seq, count, element)
  for _ = 1, count do
    seq[#seq + 1] = element
  end
  return seq
end

function tests.simple_shouldAddRewardAndFinishMap()
  local gameMock = createGameMock()
  reward_controllers.setGameObject(gameMock)

  local REWARD = 5
  local controller = reward_controllers.createSimple()
  controller:handlePickup({reward = REWARD})

  asserts.EQ(gameMock:callCount('finishMap'), 1)
  asserts.EQ(gameMock:callCount('addScore'), 1)

  asserts.EQ(gameMock:rewardGiven(), REWARD)
end

function tests.simple_shouldAcceptNegativeReward()
  local gameMock = createGameMock()
  reward_controllers.setGameObject(gameMock)

  local REWARD = -15
  local controller = reward_controllers.createSimple()
  controller:handlePickup({reward = REWARD})

  asserts.EQ(gameMock:callCount('finishMap'), 1)
  asserts.EQ(gameMock:callCount('addScore'), 1)
  asserts.EQ(gameMock:rewardGiven(), REWARD)
end

function tests.counting_nonGoalPickupShouldGiveRewardAndEndMap()
  local gameMock = createGameMock()
  reward_controllers.setGameObject(gameMock)

  local GOAL_GROUP = 1
  local REWARD = -2
  local REWARD2 = 16

  local controller = reward_controllers.createCounting()
  controller:init{goalGroup = GOAL_GROUP}

  local nonGoalPickupDescription = {
      reward = REWARD,
      reward2 = REWARD2,
      _group = GOAL_GROUP + 1, -- Different group
      _groupSize = 2,
  }
  controller:handlePickup(nonGoalPickupDescription)
  asserts.EQ(gameMock:callCount('finishMap'), 1)
  asserts.EQ(gameMock:rewardGiven(), REWARD)
end

local function countingCheckGoalReward(controller)
  local gameMock = createGameMock()
  reward_controllers.setGameObject(gameMock)

  local GOAL_GROUP = 1
  local REWARD = 2
  local REWARD2 = 14

  local goalPickupDescription = {
      reward = REWARD,
      reward2 = REWARD2,
      _group = GOAL_GROUP,
      _groupSize = 3,
  }
  local description = appendN({}, 3, goalPickupDescription)

  controller:init(
      {goalGroup = GOAL_GROUP},
      {description = description})

  controller:handlePickup(goalPickupDescription)
  asserts.EQ(gameMock:callCount('finishMap'), 0)
  asserts.EQ(gameMock:rewardGiven(), REWARD)

  controller:handlePickup(goalPickupDescription)
  asserts.EQ(gameMock:callCount('finishMap'), 0)
  asserts.EQ(gameMock:rewardGiven(), REWARD + REWARD)

  controller:handlePickup(goalPickupDescription)
  asserts.EQ(gameMock:callCount('finishMap'), 1)
  asserts.EQ(gameMock:rewardGiven(), REWARD + REWARD + REWARD + REWARD2)
end

function tests.counting_eachGoalShouldGiveRewardLastOneAlsoReward2()
  countingCheckGoalReward(reward_controllers.createCounting())
end

function tests.counting_incompleteGoalPickupThenDistractorShouldEndMap()
  local gameMock = createGameMock()
  reward_controllers.setGameObject(gameMock)

  local GOAL_GROUP = 1
  local REWARD = 2
  local DISTRACTOR_REWARD = -10
  local DISTRACTOR_REWARD2 = -15
  local REWARD2 = 16

  local controller = reward_controllers.createCounting()

  local goalPickupDescription = {
      reward = REWARD,
      reward2 = REWARD2,
      _group = GOAL_GROUP,
      _groupSize = 3,
  }
  local distractorPickupDescription = {
      reward = DISTRACTOR_REWARD,
      reward2 = DISTRACTOR_REWARD2,
      _group = GOAL_GROUP + 1,
      _groupSize = 1,
  }
  local descriptions = appendN({}, 3, goalPickupDescription)
  appendN(descriptions, 1, distractorPickupDescription)

  local task = {goalGroup = GOAL_GROUP}
  local taskData = {descriptions = descriptions}
  controller:init(task, taskData)

  controller:handlePickup(goalPickupDescription)
  asserts.EQ(gameMock:callCount('finishMap'), 0)
  asserts.EQ(gameMock:rewardGiven(), REWARD)

  controller:handlePickup(distractorPickupDescription)
  asserts.EQ(gameMock:callCount('finishMap'), 1)
  asserts.EQ(gameMock:rewardGiven(), REWARD + DISTRACTOR_REWARD)
end

function tests.balancedCounting_nonGoalShouldGiveProportionalRewardEndMap()
  local gameMock = createGameMock()
  reward_controllers.setGameObject(gameMock)

  local GOAL_GROUP = 1
  local IGNORED_REWARD = -100
  local REWARD = 1
  local REWARD2 = 10
  local NUM_DISTRACTORS = 3
  local NUM_GOAL_OBJECTS = 2

  local goalPickupDescription = {
      reward = REWARD,
      reward2 = REWARD2,
      _group = GOAL_GROUP,
      _groupSize = NUM_GOAL_OBJECTS,
  }

  local distractorDescription = {
      reward = IGNORED_REWARD,
      _group = GOAL_GROUP + 1, -- Different group
      _groupSize = NUM_DISTRACTORS,
  }

  local controller = reward_controllers.createBalancedCounting()
  local task = {goalGroup = GOAL_GROUP}

  local descriptions = appendN({}, NUM_GOAL_OBJECTS, goalPickupDescription)
  appendN(descriptions, NUM_DISTRACTORS, distractorDescription)
  local taskData = {descriptions = descriptions}

  controller:init(task, taskData)

  controller:handlePickup(distractorDescription)
  asserts.EQ(gameMock:callCount('finishMap'), 1)
  asserts.EQ(gameMock:rewardGiven(), -4)

  local gameMock = createGameMock()
  reward_controllers.setGameObject(gameMock)
end

function tests.balancedCounting_eachGoalShouldGiveRewardLastOneAlsoReward2()
  countingCheckGoalReward(reward_controllers.createBalancedCounting())
end

function tests.balancedCounting_incompleteGoalPickupThenDistractorShouldEndMap()
  local gameMock = createGameMock()
  reward_controllers.setGameObject(gameMock)

  local GOAL_GROUP = 1
  local REWARD = 2
  local REWARD2 = 16

  local controller = reward_controllers.createBalancedCounting()

  local goalPickupDescription = {
      reward = REWARD,
      reward2 = REWARD2,
      _group = GOAL_GROUP,
      _groupSize = 3,
  }
  local distractorPickupDescription = {
      _group = GOAL_GROUP + 1,
      _groupSize = 1,
  }
  local descriptions = appendN({}, 3, goalPickupDescription)
  appendN(descriptions, 1, distractorPickupDescription)

  local task = {goalGroup = GOAL_GROUP}
  local taskData = {descriptions = descriptions}
  controller:init(task, taskData)

  controller:handlePickup(goalPickupDescription)
  asserts.EQ(gameMock:callCount('finishMap'), 0)
  asserts.EQ(gameMock:rewardGiven(), REWARD)

  local EXPECTED_DISTRACTOR_REWARD = -(3 * REWARD + REWARD2)

  controller:handlePickup(distractorPickupDescription)
  asserts.EQ(gameMock:callCount('finishMap'), 1)
  asserts.EQ(gameMock:rewardGiven(), REWARD + EXPECTED_DISTRACTOR_REWARD)
end


function tests.ordered_shouldGiveRewardIfPickupsInOrder()
  local gameMock = createGameMock()
  reward_controllers.setGameObject(gameMock)

  local REWARD2 = -10
  local FIRST_PICKUP_GROUP = 2
  local FIRST_PICKUP_REWARD = 10
  local SECOND_PICKUP_GROUP = 1
  local SECOND_PICKUP_REWARD = 15

  local controller = reward_controllers.createOrdered(
    {FIRST_PICKUP_GROUP, SECOND_PICKUP_GROUP})
  controller:init{}

  local firstPickupDescription = {
      reward = FIRST_PICKUP_REWARD,
      reward2 = REWARD2,
      _group = FIRST_PICKUP_GROUP,
      _groupSize = 1,
  }
  local secondPickupDescription = {
      reward = SECOND_PICKUP_REWARD,
      reward2 = REWARD2,
      _group = SECOND_PICKUP_GROUP,
      _groupSize = 1,
  }

  controller:handlePickup(firstPickupDescription)
  asserts.EQ(gameMock:callCount('finishMap'), 0)
  asserts.EQ(gameMock:rewardGiven(), FIRST_PICKUP_REWARD)

  controller:handlePickup(secondPickupDescription)
  asserts.EQ(gameMock:callCount('finishMap'), 1)
  asserts.EQ(gameMock:rewardGiven(),
             FIRST_PICKUP_REWARD + SECOND_PICKUP_REWARD)
end

function tests.ordered_shouldGiveReward2AndFinishMapIfPickedOutOfOrder()
  local gameMock = createGameMock()
  reward_controllers.setGameObject(gameMock)

  local REWARD2 = -10
  local FIRST_PICKUP_GROUP = 2
  local SECOND_PICKUP_GROUP = 1
  local SECOND_PICKUP_REWARD = 15

  local controller = reward_controllers.createOrdered(
    {FIRST_PICKUP_GROUP, SECOND_PICKUP_GROUP})
  controller:init{}

  local secondPickupDescription = {
      reward = SECOND_PICKUP_REWARD,
      reward2 = REWARD2,
      _group = SECOND_PICKUP_GROUP,
      _groupSize = 1,
  }

  controller:handlePickup(secondPickupDescription)
  asserts.EQ(gameMock:callCount('finishMap'), 1)
  asserts.EQ(gameMock:rewardGiven(), REWARD2)
end

function tests.ordered_shouldGiveRewardAndFinishMapIfDistractorPicked()
  local gameMock = createGameMock()
  reward_controllers.setGameObject(gameMock)

  local DISTRACTOR_PICKUP_GROUP = 1
  local FIRST_PICKUP_GROUP = 2
  local SECOND_PICKUP_GROUP = 3
  local DISTRACTOR_REWARD2 = -10
  local DISTRACTOR_REWARD = -15

  local controller = reward_controllers.createOrdered(
    {FIRST_PICKUP_GROUP, SECOND_PICKUP_GROUP})
  controller:init{}

  local distractorPickupDescription = {
      reward = DISTRACTOR_REWARD,
      reward2 = DISTRACTOR_REWARD2,
      _group = DISTRACTOR_PICKUP_GROUP,
      _groupSize = 1,
  }

  controller:handlePickup(distractorPickupDescription)
  asserts.EQ(gameMock:callCount('finishMap'), 1)
  asserts.EQ(gameMock:rewardGiven(), DISTRACTOR_REWARD2)
end

function tests.ordered_shouldGiveRewardAndFinishMapIfDistractorPicked2nd()
  local gameMock = createGameMock()
  reward_controllers.setGameObject(gameMock)

  local DISTRACTOR_PICKUP_GROUP = 1
  local FIRST_PICKUP_GROUP = 2
  local SECOND_PICKUP_GROUP = 3
  local REWARD2 = -10
  local FIRST_PICKUP_REWARD = 10
  local DISTRACTOR_REWARD2 = -10
  local DISTRACTOR_REWARD = -15

  local controller = reward_controllers.createOrdered(
    {FIRST_PICKUP_GROUP, SECOND_PICKUP_GROUP})
  controller:init{}

  local firstPickupDescription = {
      reward = FIRST_PICKUP_REWARD,
      reward2 = REWARD2,
      _group = FIRST_PICKUP_GROUP,
      _groupSize = 1,
  }
  local distractorPickupDescription = {
      reward = DISTRACTOR_REWARD,
      reward2 = DISTRACTOR_REWARD2,
      _group = DISTRACTOR_PICKUP_GROUP,
      _groupSize = 1,
  }

  controller:handlePickup(firstPickupDescription)
  asserts.EQ(gameMock:callCount('finishMap'), 0)
  asserts.EQ(gameMock:rewardGiven(), FIRST_PICKUP_REWARD)

  controller:handlePickup(distractorPickupDescription)
  asserts.EQ(gameMock:callCount('finishMap'), 1)
  asserts.EQ(gameMock:rewardGiven(), FIRST_PICKUP_REWARD + DISTRACTOR_REWARD2)
end

function tests.balanced_shouldGiveConfiguredRewardForGoalObject()
  local gameMock = createGameMock()
  reward_controllers.setGameObject(gameMock)

  local GOAL_REWARD = 20
  local IGNORED_OBJECT_REWARD = 5
  local GOAL_GROUP = 1
  local controller = reward_controllers.createBalanced(GOAL_REWARD)

  local goalPickupDescription = {
      reward = IGNORED_OBJECT_REWARD,
      _group = GOAL_GROUP,
  }
  -- Goal plus distractor object descriptions.
  local descriptions = {goalPickupDescription}
  appendN(descriptions, 2, {_group = GOAL_GROUP + 1})

  controller:init(
      {goalGroup = GOAL_GROUP},
      {descriptions = descriptions})

  controller:handlePickup(goalPickupDescription)

  asserts.EQ(gameMock:callCount('finishMap'), 1)
  asserts.EQ(gameMock:callCount('addScore'), 1)
  asserts.EQ(gameMock:rewardGiven(), GOAL_REWARD)
end

function tests.balanced_distractorRewardShouldBeAFunctionOfObjectCount()
  local GOAL_REWARD = 20
  local IGNORED_OBJECT_REWARD = 5
  local GOAL_GROUP = 1
  local controller = reward_controllers.createBalanced(GOAL_REWARD)

  local goalPickupDescription = {
      reward = IGNORED_OBJECT_REWARD,
      _group = GOAL_GROUP,
  }

  local distractorPickupDescription = {
      reward = IGNORED_OBJECT_REWARD,
      _group = GOAL_GROUP + 1,
  }

  -- Where the number of distractors is a factor of the goal reward, the reward
  -- should be exactly predicatable.
  for _, distractorCount in ipairs({1, 2, 4, 5, 10, 20}) do
    local gameMock = createGameMock()
    reward_controllers.setGameObject(gameMock)

    local descriptions = {goalPickupDescription}
    appendN(descriptions, distractorCount, distractorPickupDescription)
    controller:init(
        {goalGroup = GOAL_GROUP},
        {descriptions = descriptions})

    controller:handlePickup(distractorPickupDescription)

    asserts.EQ(gameMock:callCount('finishMap'), 1)
    asserts.EQ(gameMock:callCount('addScore'), 1)
    asserts.EQ(gameMock:rewardGiven(), -GOAL_REWARD / distractorCount)
  end

  -- If not a factor, reward should be close to reward/count:
  for _, distractorCount in ipairs({3, 6, 7, 8, 9, 11, 12, 21}) do
    local gameMock = createGameMock()
    reward_controllers.setGameObject(gameMock)

    local descriptions = {goalPickupDescription}
    appendN(descriptions, distractorCount, distractorPickupDescription)

    controller:init(
        {goalGroup = GOAL_GROUP},
        {descriptions = descriptions})

    controller:handlePickup(distractorPickupDescription)

    asserts.EQ(gameMock:callCount('finishMap'), 1)
    asserts.EQ(gameMock:callCount('addScore'), 1)

    local expectedLowerBound = math.floor(-GOAL_REWARD / distractorCount)
    local expectedUpperBound = math.ceil(-GOAL_REWARD / distractorCount)

    asserts.GE(gameMock:rewardGiven(), expectedLowerBound)
    asserts.LE(gameMock:rewardGiven(), expectedUpperBound)
  end
end

function tests.balanced_distractorRewardCountsAllNonGoalGroups()
  local GOAL_REWARD = 20
  local GOAL_GROUP = 1
  local controller = reward_controllers.createBalanced(GOAL_REWARD)

  local distractorPickupDescription = {
      _group = GOAL_GROUP + 1,
  }
  local gameMock = createGameMock()
  reward_controllers.setGameObject(gameMock)

  -- Define 10 distractors in 2 groups, so reward should be -2
  local descriptions = {_group = GOAL_GROUP}
  appendN(descriptions, 3, {_group = GOAL_GROUP + 1})
  appendN(descriptions, 7, {_group = GOAL_GROUP + 2})

  controller:init(
      {goalGroup = GOAL_GROUP},
      {descriptions = descriptions})

    controller:handlePickup(distractorPickupDescription)

    asserts.EQ(gameMock:callCount('finishMap'), 1)
    asserts.EQ(gameMock:callCount('addScore'), 1)
    asserts.EQ(gameMock:rewardGiven(), -2)
end

function tests.balanced_goalGroupIsUsed()
  local gameMock = createGameMock()
  reward_controllers.setGameObject(gameMock)

  local GOAL_REWARD = 10
  local GOAL_GROUP = 2

  -- 2 distractors, so reward should be -5
  local DISTRACTOR_REWARD = -5
  local DESCRIPTIONS = {
      {_group = GOAL_GROUP - 1},
      {_group = GOAL_GROUP},
      {_group = GOAL_GROUP + 1},
  }

  local controller = reward_controllers.createBalanced(GOAL_REWARD)
  controller:init(
      {goalGroup = GOAL_GROUP},
      {descriptions = DESCRIPTIONS})

  controller:handlePickup({_group = 1})
  asserts.EQ(gameMock:rewardGiven(), DISTRACTOR_REWARD)

  controller:handlePickup({_group = 2})
  asserts.EQ(gameMock:rewardGiven(), DISTRACTOR_REWARD + GOAL_REWARD)

  controller:handlePickup({_group = 3})
  asserts.EQ(gameMock:rewardGiven(), 2 * DISTRACTOR_REWARD + GOAL_REWARD)
end

function tests.answer_givesRewardIfFunctionMatchesAnswer()
  local REWARD = 5

  local yesIsCorrect = function(_) return true end
  local noIsCorrect = function(_) return false end

  local gameMock = createGameMock()
  reward_controllers.setGameObject(gameMock)

  local controller = reward_controllers.createAnswer{
      truth = yesIsCorrect,
      groupToAnswer = {true, false},
      reward = REWARD,
  }
  controller:init({}, {objectsPerGroup = {}})

  -- Picking up group 1 object is saying yes - should add reward.
  controller:handlePickup({_group = 1})
  asserts.EQ(gameMock:callCount('finishMap'), 1)
  asserts.EQ(gameMock:callCount('addScore'), 1)
  asserts.EQ(gameMock:rewardGiven(), REWARD)

  -- Picking up group 2 object is saying no - should remove reward.
  controller:handlePickup({_group = 2})
  asserts.EQ(gameMock:callCount('finishMap'), 2)
  asserts.EQ(gameMock:callCount('addScore'), 2)
  asserts.EQ(gameMock:rewardGiven(), 0)

  -- Now check when 'no' is correct answer.
  gameMock = createGameMock()
  reward_controllers.setGameObject(gameMock)
  controller = reward_controllers.createAnswer{
      truth = noIsCorrect,
      groupToAnswer = {true, false},
      reward = REWARD,
  }
  controller:init({}, {objectsPerGroup = {}})

  controller:handlePickup({_group = 2})
  asserts.EQ(gameMock:callCount('finishMap'), 1)
  asserts.EQ(gameMock:callCount('addScore'), 1)
  asserts.EQ(gameMock:rewardGiven(), REWARD)

  controller:handlePickup({_group = 1})
  asserts.EQ(gameMock:callCount('finishMap'), 2)
  asserts.EQ(gameMock:callCount('addScore'), 2)
  asserts.EQ(gameMock:rewardGiven(), 0)

  -- Finally check with the yes / no group mappings swapped.
  gameMock = createGameMock()
  reward_controllers.setGameObject(gameMock)
  controller = reward_controllers.createAnswer{
      truth = noIsCorrect,
      groupToAnswer = {false, true},
      reward = REWARD,
  }
  controller:init({}, {objectsPerGroup = {}})

  controller:handlePickup({_group = 1})
  asserts.EQ(gameMock:callCount('finishMap'), 1)
  asserts.EQ(gameMock:callCount('addScore'), 1)
  asserts.EQ(gameMock:rewardGiven(), REWARD)
end

function tests.answer_defaultRewardIsPlusMinus10()
  local DEFAULT_REWARD = 10
  local gameMock = createGameMock()
  reward_controllers.setGameObject(gameMock)
  local controller = reward_controllers.createAnswer{
      truth = function(_) return true end,
      groupToAnswer = {true, false},
  }
  controller:init({}, {objectsPerGroup = {}})
  controller:handlePickup({_group = 1})
  asserts.EQ(gameMock:rewardGiven(), DEFAULT_REWARD)
end

function tests.answer_truthFunctionCalledWithObjectCounts()
  local calledWith = {}
  local truth = function(v) calledWith = v return true end

  local gameMock = createGameMock()
  reward_controllers.setGameObject(gameMock)
  local controller = reward_controllers.createAnswer{
      truth = truth,
      groupToAnswer = {true, false},
  }

  local OBJECTS_PER_GROUP = {2, 4, 1, 1}
  controller:init({}, {objectsPerGroup = OBJECTS_PER_GROUP})
  controller:handlePickup({_group = 1})

  asserts.tablesEQ(calledWith, OBJECTS_PER_GROUP)
end

function tests.answer_shouldFailIfPickupDoesNotMapToAnAnswer()
  local gameMock = createGameMock()
  reward_controllers.setGameObject(gameMock)
  local controller = reward_controllers.createAnswer{
      truth = function (_) return true end,
      groupToAnswer = {true, false},
  }
  controller:init({}, {objectsPerGroup = {}})

  asserts.shouldFail(function () controller:handlePickup({_group = 3}) end)
end

function tests.answer_canHandleNonBoolAnswers()
  local DEFAULT_REWARD = 10
  local CORRECT_ANSWER = 'Professor Truffles'

  local gameMock = createGameMock()
  reward_controllers.setGameObject(gameMock)
  local controller = reward_controllers.createAnswer{
      truth = function (_) return CORRECT_ANSWER end,
      groupToAnswer = {'Fido', CORRECT_ANSWER, 'Ajax'},
  }
  controller:init({}, {objectsPerGroup = {}})

  controller:handlePickup({_group = 2})
  asserts.EQ(gameMock:callCount('finishMap'), 1)
  asserts.EQ(gameMock:callCount('addScore'), 1)
  asserts.EQ(gameMock:rewardGiven(), DEFAULT_REWARD)

  -- Check the reverse is true
  controller:handlePickup({_group = 1})
  asserts.EQ(gameMock:callCount('finishMap'), 2)
  asserts.EQ(gameMock:callCount('addScore'), 2)
  asserts.EQ(gameMock:rewardGiven(), 0)
end


return test_runner.run(tests)
