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
local make = require 'language.make'
local set = require 'common.set'

-- Checks if a value is present in a list
local function contains(seq, item)
  return set.Set(seq)[item]
end

local function createMockContextWithGroups(groups)
  local mockContext = {}
  mockContext.getGroup = function(self, groupNumber)
    return groups[groupNumber]
  end
  return mockContext
end

local tests = {}

function tests.isNot_FunctorWhichRemovesItemsFromList()
  local isNot = make.isNot({'cat', 'hat'})
  asserts.EQ(type(isNot), 'function')
  local result = isNot({'cat', 'sat', 'hat', 'mat'})
  asserts.tablesEQ(result, {'sat', 'mat'})
end

function tests.choices_FunctorWhichOverridesInputList()
  local OVERRIDE = {'cat', 'hat'}
  local choices = make.choices(OVERRIDE)
  asserts.EQ(type(choices), 'function')
  asserts.tablesEQ(choices({'sat', 'vat', 'mat'}), OVERRIDE)
end

function tests.oneOf_FunctorWhichOverridesInputListAndReturnsRandomValue()
  local OVERRIDE = {'cat', 'hat'}
  local oneOf = make.oneOf(OVERRIDE)
  asserts.EQ(type(oneOf), 'function')
  local sample = oneOf({'sat', 'vat', 'mat'})
  asserts.EQ(contains(OVERRIDE, sample), true)
end

function tests.keep_FunctorWhichKeepsCommonValues()
  local KEEP = {'cat', 'hat'}
  local keep = make.keep(KEEP)
  asserts.EQ(type(keep), 'function')
  asserts.tablesEQ(keep({'sat', 'vat', 'mat'}), {})
  asserts.tablesEQ(keep({'cat', 'vat', 'mat'}), {'cat'})
end

function tests.sameAs_FunctorWhichFetchesExisting1stValueFromGroup()
  local group1 = {
    {headgear = 'hat', pet = 'cat'},
    {headgear = 'log', pet = 'dog'},
  }
  local context = createMockContextWithGroups{group1}

  local sameAs = make.sameAs(1)
  asserts.EQ(type(sameAs), 'function')
  asserts.EQ(sameAs({}, context, 'headgear'), 'hat')
  asserts.EQ(sameAs({}, context, 'pet'), 'cat')

  -- Ensure that calling functor with an explicit group ignores the parameter
  -- and uses the one specified above.
  local OTHER_GROUP = 99
  asserts.EQ(sameAs({}, context, 'headgear', OTHER_GROUP), 'hat')
  asserts.EQ(sameAs({}, context, 'pet', OTHER_GROUP), 'cat')
end

function tests.sameAs_FunctorWithNoArgsUsesCallTimeGroup()
  local group1 = {
    {headgear = 'hat', pet = 'cat'},
    {headgear = 'log', pet = 'dog'},
  }
  local group2 = {
    {headgear = 'cap', pet = 'hog'},
  }
  local context = createMockContextWithGroups{group1, group2}

  local sameAs = make.sameAs()
  asserts.EQ(type(sameAs), 'function')
  -- Group 1
  asserts.EQ(sameAs({}, context, 'headgear', 1), 'hat')
  asserts.EQ(sameAs({}, context, 'pet', 1), 'cat')
  -- Group 2
  asserts.EQ(sameAs({}, context, 'headgear', 2), 'cap')
  asserts.EQ(sameAs({}, context, 'pet', 2), 'hog')
end

function tests.notSameAs_FunctorWhichRemovesExistingValuesFromGroup()
  local group1 = {
    {headgear = 'hat', pet = 'cat'},
    {headgear = 'log', pet = 'dog'},
  }
  local context = createMockContextWithGroups{group1}

  local notSameAs = make.notSameAs(1)
  asserts.EQ(type(notSameAs), 'function')
  asserts.tablesEQ(notSameAs({'hat', 'log', 'cap'}, context, 'headgear'),
                   {'cap'})
  asserts.tablesEQ(notSameAs({'dog', 'cat', 'hog', 'pig'}, context, 'pet'),
          {'hog', 'pig'})
end

function tests.notSameAs_FunctorWithNoArgsUsesCallTimeGroup()
  local group1 = {
    {headgear = 'hat', pet = 'cat'},
    {headgear = 'log', pet = 'dog'},
  }
  local group2 = {
    {headgear = 'cap', pet = 'hog'},
  }
  local context = createMockContextWithGroups{group1, group2}

  local notSameAs = make.notSameAs()
  asserts.EQ(type(notSameAs), 'function')
  -- Apply to group 1
  asserts.tablesEQ(notSameAs({'hat', 'log', 'cap'}, context, 'headgear', 1),
                   {'cap'})
  asserts.tablesEQ(notSameAs({'dog', 'cat', 'hog'}, context, 'pet', 1), {'hog'})
  -- Apply to group 2
  asserts.tablesEQ(notSameAs({'hat', 'log', 'cap'}, context, 'headgear', 2),
          {'hat', 'log'})
  asserts.tablesEQ(notSameAs({'dog', 'cat', 'hog'}, context, 'pet', 2),
                   {'dog', 'cat'})
end

function tests.differentTo_FunctorWhichReturnsUnusedValueFromGroup()
  local group1 = {
      {headgear = 'hat', pet = 'cat'},
  }
  local context = createMockContextWithGroups{group1}

  local CHANCE = 0
  local differentTo = make.differentTo(1)
  asserts.EQ(type(differentTo), 'function')

  local sample = differentTo({'hat', 'log', 'cap'}, context, 'headgear')
  asserts.EQ(contains({'log', 'cap'}, sample), true,
             'Value not in expected set')

  sample = differentTo({'dog', 'cat', 'hog'}, context, 'pet')
  asserts.EQ(contains({'dog', 'hog'}, sample), true,
             'Value not in expected set')
end

function tests.maybeSameAs1_FunctorWhichReturnsExisting1stValueFromGroup()
  local group1 = {
    {headgear = 'hat', pet = 'cat'},
    {headgear = 'log', pet = 'dog'},
  }
  local context = createMockContextWithGroups{group1}

  local CHANCE = 1
  local maybeSameAs = make.maybeSameAs(1, CHANCE)
  asserts.EQ(type(maybeSameAs), 'function')
  asserts.EQ(maybeSameAs({'hat', 'log', 'cap'}, context, 'headgear'), 'hat')
  asserts.EQ(maybeSameAs({'dog', 'cat', 'hog'}, context, 'pet'), 'cat')
end

function tests.maybeSameAs0_FunctorWhichReturnsUnusedValueFromGroup()
  local group1 = {
    {headgear = 'hat', pet = 'cat'},
  }
  local context = createMockContextWithGroups{group1}

  local CHANCE = 0
  local maybeSameAs = make.maybeSameAs(1, CHANCE)
  asserts.EQ(type(maybeSameAs), 'function')

  local sample = maybeSameAs({'hat', 'log', 'cap'}, context, 'headgear')
  asserts.EQ(contains({'log', 'cap'}, sample), true,
             'Value not in expected set')

  sample = maybeSameAs({'dog', 'cat', 'hog'}, context, 'pet')
  asserts.EQ(contains({'dog', 'hog'}, sample), true,
             'Value not in expected set')
end

function tests.unique_FunctorWhichSelectsRandomUnsedItemsFromGroupNoChoice()
  local group1 = {
    {headgear = 'hat', pet = 'cat'},
    {headgear = 'log', pet = 'dog'},
  }
  local context = createMockContextWithGroups{group1}

  local unique = make.unique(1)
  asserts.EQ(type(unique), 'function')
  asserts.EQ(unique({'hat', 'log', 'cap'}, context, 'headgear'), 'cap')
  asserts.EQ(unique({'dog', 'hog', 'cat'}, context, 'pet'), 'hog')

  -- Ensure that calling functor with an explicit group ignores the parameter
  -- and uses the one specified above.
  local OTHER_GROUP = 99
  asserts.EQ(unique({'hat', 'log', 'cap'}, context, 'headgear', OTHER_GROUP),
           'cap')
  asserts.EQ(unique({'dog', 'hog', 'cat'}, context, 'pet', OTHER_GROUP), 'hog')
end

function tests.unique_FunctorWhichSelectsRandomUnsedItemsFromGroup()
  local group1 = {
    {headgear = 'hat', pet = 'cat'},
    {headgear = 'log', pet = 'dog'},
  }
  local context = createMockContextWithGroups{group1}

  local INPUT = {'cat', 'pig', 'hog', 'dog'}
  local UNUSED = {'pig', 'hog'}
  local NUM_CALLS = 20

  local unique = make.unique(1)
  for i = 1, NUM_CALLS do
    local sample = unique(INPUT, context, 'pet')
    asserts.EQ(contains(UNUSED, sample), true,
               'Unique value outside expectation')
  end
end

function tests.unique_FunctorReturnsNilIfAllValuesUsed()
  local group1 = {
    {headgear = 'hat', pet = 'cat'},
    {headgear = 'log', pet = 'dog'},
  }
  local context = createMockContextWithGroups{group1}

  local unique = make.unique(1)
  asserts.EQ(type(unique), 'function')
  asserts.EQ(unique({'hat', 'log'}, context, 'headgear'), nil)
end

function tests.unique_FunctorReturnsAnInitialValue()
  local group1 = {}
  local context = createMockContextWithGroups{group1}

  local unique = make.unique(1)
  asserts.EQ(unique({'hat'}, context, 'headgear'), 'hat')
end

return test_runner.run(tests)
