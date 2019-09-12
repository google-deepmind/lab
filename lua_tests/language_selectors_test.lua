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
local selectors = require 'language.selectors'
local set = require 'common.set'

local tests = {}

function tests.identity_ShouldReturnArgument()
  local selector = selectors.createIdentity(4)
  asserts.EQ(selector(), 4)
  asserts.EQ(selector(), 4)
  asserts.EQ(selector(), 4)

  local TABLE = {'fred', 'wilma'}
  local selector = selectors.createIdentity(TABLE)
  asserts.EQ(selector(), TABLE)
  asserts.EQ(selector(), TABLE)
  asserts.EQ(selector(), TABLE)
end

function tests.identity_ShouldFailIfNil()
  local aTypoWithNoValue
  asserts.shouldFail(function ()
        local selector = selectors.createIdentity(aTypoWithNoValue)
  end)
end


function tests.ordered_ShouldRoundRobinOverChoices()
  local selector = selectors.createOrdered{2, 4, 8}
  asserts.EQ(selector(), 2)
  asserts.EQ(selector(), 4)
  asserts.EQ(selector(), 8)
  asserts.EQ(selector(), 2)
  asserts.EQ(selector(), 4)
  asserts.EQ(selector(), 8)
  asserts.EQ(selector(), 2)
end

function tests.ordered_ShouldFailIfAChoiceIsNil()
  asserts.shouldFail(function ()
        local selector = selectors.createOrdered({nil})
  end)
  asserts.shouldFail(function ()
        local selector = selectors.createOrdered({'valid', nil, 'otherValid'})
  end)
end


function tests.random_ShouldReturnSameValueIfOnlyOneChoice()
  local VALUE = 31
  local selector = selectors.createRandom{VALUE}
  for i = 1, 10 do
    asserts.EQ(selector(), VALUE)
  end
end

function tests.random_ShouldReturnValuesFromGivenChoices()
  local CHOICES = {'move', 37}
  local selector = selectors.createRandom(CHOICES)
  local CHOICE_SET = set.Set(CHOICES)
  for i = 1, 20 do
    assert(CHOICE_SET[selector()])
  end
end

function tests.random_ShouldFailIfAChoiceIsNil()
  asserts.shouldFail(function ()
        local selector = selectors.createRandom({nil})
  end)
  asserts.shouldFail(function ()
        local selector = selectors.createRandom({'valid', nil, 'otherValid'})
  end)
end

function tests.discreteDistribution_shouldReturnValuesFromGivenChoices()
  local WEIGHT_CHOICE_PAIRS = {
      {2, 'move'},
      {1, 37}
  }
  local CHOICE_SET = set.Set{'move', 37}

  local selector = selectors.createDiscreteDistribution(WEIGHT_CHOICE_PAIRS)
  for i = 1, 20 do
    assert(CHOICE_SET[selector()])
  end
end

function tests.discreteDistribution_shouldFailIfAWeightOrChoiceIsNil()
  local aTypoWithNoValue
  local NIL_WEIGHT = {
      {aTypoWithNoValue, 'move'},
      {1, 37}
  }
  local NIL_CHOICE = {
      {2, 'move'},
      {1, aTypoWithNoValue}
  }

  asserts.shouldFail(function ()
        local selector = selectors.createDiscreteDistribution(NIL_WEIGHT)
  end)
  asserts.shouldFail(function ()
        local selector = selectors.createDiscreteDistribution(NIL_CHOICE)
  end)
end

function tests.calling_shouldReturnTheResultOfCallingItsChoice()
  local function returnA() return 'A' end
  local function returnZ() return 'Z' end

  local ordered = selectors.createOrdered{returnA, returnZ}
  local calling = selectors.createCalling(ordered)
  asserts.EQ(calling(), 'A')
  asserts.EQ(calling(), 'Z')
  asserts.EQ(calling(), 'A')
end

return test_runner.run(tests)
