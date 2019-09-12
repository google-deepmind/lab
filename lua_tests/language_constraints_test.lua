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
local constraints = require 'language.constraints'
local set = require 'common.set'

local function countEntriesInTable(tbl)
  local count = 0
  for _, _ in pairs(tbl) do
    count = count + 1
  end
  return count
end

local function createMockContextWhereGetGroupXReturnsY(x, y)
  local mockContext = {}
  mockContext.getGroup = function(self, groupNumber)
    if groupNumber == x then
      return y
    end
  end
  return mockContext
end

local function createMockContextWhereUsedGroupXPetsAre(group, listOfUsedPets)
  local values = {}
  for _, value in ipairs(listOfUsedPets) do
    values[#values + 1] = {pet = value}
  end
  return createMockContextWhereGetGroupXReturnsY(group, values)
end

local function combineGetGroupMocks(listOfMocks)
  local mockContext = {}
  mockContext.getGroup = function(self, groupNumber)
    for _, mock in ipairs(listOfMocks) do
      local value = mock:getGroup(groupNumber)
      if value then return value end
    end
  end
  return mockContext
end


local tests = {}

function tests.removeFromEmptyList()
  asserts.tablesEQ(constraints.remove({}, 'anything'), {})
end

function tests.removeSingleItemAndPreservesOrder()
  asserts.tablesEQ(constraints.remove({'cat', 'hat', 'bra'}, 'hat'),
                   {'cat', 'bra'})
end

function tests.removeSingleItemWithMultipleOccurences()
  asserts.tablesEQ(constraints.remove({'cat', 'hat', 'bra', 'hat'}, 'hat'),
          {'cat', 'bra'})
end

function tests.removeListOfItems()
  asserts.tablesEQ(constraints.remove({'car', 'hat', 'bra'}, {'bra', 'car'}),
                   {'hat'})
end

function tests.removeListOfItemsShouldDoNothingIfItemsNotPresent()
  asserts.tablesEQ(constraints.remove({'cat', 'hat'}, {'bra', 'car'}),
                   {'cat', 'hat'})
end


function tests.intersect_withEitherArgEmptyIsEmpty()
  asserts.tablesEQ(constraints.intersect({}, {}), {})
  asserts.tablesEQ(constraints.intersect({}, {'value'}), {})
  asserts.tablesEQ(constraints.intersect({'value'}, {}), {})
end

function tests.intersect_keepsOnlyCommonItems()
  asserts.tablesEQ(constraints.intersect({'cat', 'hat'}, {'hat', 'car'}),
                   {'hat'})
end

function tests.intersect_convertsNonListItem()
  asserts.tablesEQ(constraints.intersect({'cat', 'hat'}, 'cat'), {'cat'})
end

function tests.intersect_preservesOrderOfFirstSequence()
  asserts.tablesEQ(constraints.intersect({'cat', 'hat'}, {'hat', 'cat'}),
                   {'cat', 'hat'})
  asserts.tablesEQ(constraints.intersect({'cat', 'hat', 'car'}, {'car', 'cat'}),
       {'cat', 'car'})
end


function tests.copy_returnsFirstArgument()
  asserts.tablesEQ(constraints.copy({'cat', 'hat'}), {'cat', 'hat'})
end

function tests.copy_returnsACopyNotTheSameTable()
  local INPUT = {'cat', 'hat'}
  local result = constraints.copy(INPUT)
  -- Initial result is equal
  asserts.tablesEQ(result, INPUT)
  -- Modified result is no longer equal, therefore not same table object
  result[1] = 'changedValue'
  asserts.tablesNE(result, INPUT)
end

function tests.copy_convertsNonListToList()
  asserts.tablesEQ(constraints.copy('cat'), {'cat'})
end


function tests.first_ShouldReturnFirstListElement()
  local FIRST_ELEMENT = 'one'
  asserts.EQ(constraints.first({FIRST_ELEMENT, 'two', 'three'}), FIRST_ELEMENT)
end

function tests.first_ShouldReturnNilGivenEmptyList()
  asserts.EQ(constraints.first({}), nil)
end


function tests.random_WithEmptyInputShouldReturnNil()
  asserts.EQ(constraints.random({}), nil)
end

function tests.random_ShouldReturnSingleElementFromInput()
  -- Ensure results are always from the input list.
  local LIST = {'cat', 'hat', 'bra', 'car'}
  local VALUE_SET = set.Set(LIST)
  local NUM_CALLS = 20
  for i = 1, NUM_CALLS do
    local sample = constraints.random(LIST)
    asserts.EQ(VALUE_SET[sample], true, 'random value outside input sequence')
  end
end

function tests.sameAs_ShouldReturnAttributeOfFirstGroupMember()
  local group1 = {
    {headgear = 'hat', pet = 'cat'},
    {headgear = 'log', pet = 'dog'},
  }

  -- when(context):getGroup(1).thenReturn(group1)
  local context = createMockContextWhereGetGroupXReturnsY(1, group1)
  local result = constraints.sameAs(context, 'pet', 1)
  asserts.EQ(result, 'cat',
           'result expected to be the pet of the first group entry')
end

function tests.sameAs_ShouldReturnAttributeOfCorrectGroup()
  local group2 = {
    {headgear = 'hat', pet = 'cat'},
    {headgear = 'log', pet = 'dog'},
  }
  -- when(context):getGroup(2).thenReturn(group2)
  local context = createMockContextWhereGetGroupXReturnsY(2, group2)
  local result = constraints.sameAs(context, 'pet', 2)
  asserts.EQ(result, 'cat', 'result expected to be 1st pet of the 2nd group')
end


function tests.notSameAs_ShouldRemoveAttributeValueFromList()
  local group1 = {
    {headgear = 'hat', pet = 'cat'},
  }
  -- when(context):getGroup(1).thenReturn(group1)
  local context = createMockContextWhereGetGroupXReturnsY(1, group1)
  local list = {'cat', 'hat', 'bra', 'car'}
  local result = constraints.notSameAs(list, context, 'pet', 1)
  asserts.tablesEQ(result, {'hat', 'bra', 'car'},
       'result expected to remove the pet of the first group entry')
end

function tests.notSameAs_ShouldRemoveAttributeValueFromList()
  local group1 = {
      {headgear = 'hat', pet = 'cat'},
  }
  local group2 = {
      {headgear = 'cap', pet = 'cat'},
  }
  local context = combineGetGroupMocks{
      createMockContextWhereGetGroupXReturnsY(1, group1),
      createMockContextWhereGetGroupXReturnsY(2, group2)
  }
  local list = {'cap', 'fez', 'hat', 'sombrero'}
  local result = constraints.notSameAs(list, context, 'headgear', {1, 2})
  asserts.tablesEQ(result, {'fez', 'sombrero'},
          'result expected to exclude used values from group 1 and 2')
end


function tests.notSameAs_ShouldRemoveMultipleAttributeValuesFromList()
  local context = createMockContextWhereUsedGroupXPetsAre(1, {'cat', 'pig'})
  local list = {'cat', 'hat', 'bra', 'car', 'pig'}
  local result = constraints.notSameAs(list, context, 'pet', 1)
  asserts.tablesEQ(result, {'hat', 'bra', 'car'},
       'result expected to remove the pet of all group entries')
end

function tests.differentTo_ShouldReturnValueExcludingUsedOnes()
  local context = createMockContextWhereUsedGroupXPetsAre(1, {'cat', 'pig'})
  local list = {'cat', 'dog', 'pig'}
  local result = constraints.differentTo(list, context, 'pet', 1)
  asserts.EQ(result, 'dog', 'result expected to be only unused pet')
end

function tests.differentTo_ShouldReturnValueExcludingUsedOnesMultiGroup()
  local context = combineGetGroupMocks{
      createMockContextWhereUsedGroupXPetsAre(1, {'cat', 'pig'}),
      createMockContextWhereUsedGroupXPetsAre(2, {'dog', 'pig'})
  }
  local list = {'cat', 'dog', 'pig', 'womble'}
  local result = constraints.differentTo(list, context, 'pet', {1, 2})
  asserts.EQ(result, 'womble', 'result expected to be only unused pet')
end

function tests.maybeSameAs_ShouldEqualSameAsWhenChance1()
  local PETS = {'cat', 'dog', 'hog'}
  local group1 = {
    {pet = 'cat'},
  }
  -- when(context):getGroup(1).thenReturn(group1)
  local context = createMockContextWhereGetGroupXReturnsY(1, group1)

  local CHANCE = 1
  local result = constraints.maybeSameAs(PETS, context, 'pet', 1, CHANCE)
  asserts.EQ(result, 'cat', 'result expected to be the same when chance is 1')
end

function tests.maybeSameAs_ShouldNotReturnSameValueWhenChance0()
  local PETS = {'cat', 'dog'}
  local group1 = {
    {pet = 'cat'},
  }
  --when(context):getGroup(1).thenReturn(group1)
  local context = createMockContextWhereGetGroupXReturnsY(1, group1)

  local CHANCE = 0
  local result = constraints.maybeSameAs(PETS, context, 'pet', 1, CHANCE)
  asserts.EQ(result, 'dog', 'result expected to be different when chance is 0')
end

function tests.maybeSameAs_ShouldReturnMixedValuesWhenChanceHalf()
  local PETS = {'dog', 'hog'}
  local group1 = {
    {pet = 'cat'},
  }
  --when(context):getGroup(1).thenReturn(group1)
  local context = createMockContextWhereGetGroupXReturnsY(1, group1)
  local CHANCE = 0.5

  local results = {cat = 0, dog = 0, hog = 0}
  for ii = 1, 100 do
    local result = constraints.maybeSameAs(PETS, context, 'pet', 1, CHANCE)
    results[result] = results[result] + 1
  end
  asserts.GT(results.cat, 0, 'Expected some cats when randomly matching')
  asserts.GT(results.dog, 0, 'Expected some dogs when randomly matching')
  asserts.GT(results.hog, 0, 'Expected some hogs when randomly matching')
  asserts.EQ(countEntriesInTable(results), 3, 'Unexpected results')
end


function tests.unique_ShouldReturnAnUnusedValue()
  local ALL_PETS = {'cat', 'dog', 'hog'}
  -- Mock such that cat and dog are in use, so only hog should be available.
  local context = createMockContextWhereUsedGroupXPetsAre(1, {'cat', 'dog'})

  local result = constraints.unique(ALL_PETS, context, 'pet', 1)
  asserts.EQ(result, 'hog', 'result expected to be the only used pet')
end

function tests.unique_ShouldWorkWhenNoValuesHaveYetBeenUsed()
  local ALL_PETS = {'cat', 'dog', 'hog'}
  local PET_SET = set.Set(ALL_PETS)
  -- Mock such that nothing has yet been used
  --when(context):getGroupAttributeValues(1, 'pet').thenReturn({})
  local context = createMockContextWhereUsedGroupXPetsAre(1, {})

  local result = constraints.unique(ALL_PETS, context, 'pet', 1)
  assert(PET_SET[result], 'result expected to be in the list of pets')
end


function tests.commonRandom_ShouldReturnValueFromSequenceIfNothingInGroup()
  -- Mock such that nothing is in use yet:
  --when(context):getGroupAttributeValues(1, 'pet').thenReturn({})
  local context = createMockContextWhereUsedGroupXPetsAre(1, {})

  local result = constraints.commonRandom({'cat'}, context, 'pet', 1)
  asserts.EQ(result, 'cat', 'result expected to be the only available pet')
end

function tests.commonRandom_ShouldReturnExistingValueWhenThereIsOne()
  -- Mock such that 'dog' is already in use:
  local context = createMockContextWhereUsedGroupXPetsAre(1, {'dog'})

  local result = constraints.commonRandom({'cat'}, context, 'pet', 1)
  asserts.EQ(result, 'dog', 'result expected to be already used pet')
end

return test_runner.run(tests)
