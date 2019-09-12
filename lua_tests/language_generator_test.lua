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
local texter = require 'language.texter'

local EQ = asserts.EQ
local NE = asserts.NE

local tests = {}

local constraints = require 'language.constraints'
local generator = require 'language.generator'
local make = require 'language.make'
local set = require 'common.set'

-- Given a list, return a table mapping list element -> number of occurrences.
local function listToHistogram(list)
  local count = {}
  for _, value in ipairs(list) do
    count[value] = (count[value] or 0) + 1
  end
  return count
end

function tests.applyConstraints_checkNumericConstant()
  local NUMERIC_VALUE = 12
  local context = generator.createContext()
  EQ(context:applyConstraints(NUMERIC_VALUE, {}, 'pet', 1), NUMERIC_VALUE)
end

function tests.applyConstraints_checkStringConstant()
  local STRING_VALUE = 'pig'
  local context = generator.createContext()
  EQ(context:applyConstraints(STRING_VALUE, {}, 'pet', 1), STRING_VALUE)
end

function tests.applyConstraints_checkBoolConstantTrue()
  local BOOL_VALUE = true
  local context = generator.createContext()
  EQ(context:applyConstraints(BOOL_VALUE, {}, 'pet', 1), BOOL_VALUE)
end

function tests.applyConstraints_checkBoolConstantFalse()
  local BOOL_VALUE = false
  local context = generator.createContext()
  EQ(context:applyConstraints(BOOL_VALUE, {}, 'pet', 1), BOOL_VALUE)
end

function tests.applyConstraints_checkFunction()
  local func = function(candidates, context, attribute, group)
    return {
        candidates = candidates,
        context = context,
        attribute = attribute,
        group = group
    }
  end
  local context = generator.createContext()
  local CANDIDATES = {'Hello', 'Mum'}
  local ATTRIBUTE = 'greeting'
  local GROUP = 1
  local EXPECTATION = {
      candidates = CANDIDATES,
      context = context,
      attribute = ATTRIBUTE,
      group = GROUP
  }
  local result = context:applyConstraints(func, CANDIDATES, ATTRIBUTE, GROUP)
  asserts.tablesEQ(result, EXPECTATION)
end

function tests.applyConstraints_checkSequenceOfFunctions()
  -- Reverses the input.
  local reverse = function(candidates, context, attribute, group)
    local output = {}
    for i = #candidates, 1, -1 do
      table.insert(output, candidates[i])
    end
    return output
  end

  -- Returns the first two elements only.
  local pick2 = function(candidates, context, attribute, group)
    return {candidates[1], candidates[2]}
  end

  local ATTRIBUTE = 'unused'
  local GROUP = 1
  local CANDIDATES = {'one', 'two', 'three', 'four'}

  -- Sanity check the helper functions first.
  asserts.tablesEQ(reverse(CANDIDATES), {'four', 'three', 'two', 'one'})
  asserts.tablesEQ(pick2(CANDIDATES), {'one', 'two'})

  local context = generator.createContext()
  -- The output of each step should be passed to the next as candidates
  asserts.tablesEQ(
    context:applyConstraints({reverse, pick2}, CANDIDATES, ATTRIBUTE, GROUP),
    {'four', 'three'})
  asserts.tablesEQ(
    context:applyConstraints({pick2, reverse}, CANDIDATES, ATTRIBUTE, GROUP),
    {'two', 'one'})
end

function tests.getChoicesFor_shouldReturnPossibleAttributeValues()
  local CANDIDATES = {'one', 'two', 'three', 'four'}
  local context = generator.createContext{
      attributes = {numbers = CANDIDATES},
  }

  local CHANGE_CANDIDATES = {'six', 'seven', 'eight'}
  context:setChoicesFor('numbers', CHANGE_CANDIDATES)
  asserts.tablesEQ(context:getChoicesFor('numbers'), CHANGE_CANDIDATES)
end

function tests.processSpec_shouldGetCandidatesFromContext()
  local function identity(candidates, context, attribute, group)
    return candidates
  end

  local CANDIDATES = {'one', 'two', 'three', 'four'}

  -- Sanity check the helper function first.
  asserts.tablesEQ(identity(CANDIDATES), CANDIDATES)

  local context = generator.createContext()
  context:setChoicesFor('numbers', CANDIDATES)
  asserts.tablesEQ(context:processSpec(identity, 'numbers', 1), CANDIDATES)

  local CHANGE_CANDIDATES = {'six', 'seven', 'eight'}
  context:setChoicesFor('numbers', CHANGE_CANDIDATES)
  asserts.tablesEQ(context:processSpec(identity, 'numbers', 1),
                   CHANGE_CANDIDATES)
end

function tests.processSpec_shouldUseDefaultIfNoExplicitSpec()
  local DEFAULT_NUMBER = 31
  local DEFAULT_STRING = 'default_numberwang'
  local DEFAULT_FALSE = false
  local context = generator.createContext{
      defaults = {
          numbers = DEFAULT_NUMBER,
          strings = DEFAULT_STRING,
          liars = DEFAULT_FALSE
      },
  }
  local nilSpec = nil
  EQ(context:processSpec(nilSpec, 'numbers', 1), DEFAULT_NUMBER)
  EQ(context:processSpec(nilSpec, 'strings', 1), DEFAULT_STRING)
  EQ(context:processSpec(nilSpec, 'liars', 1), DEFAULT_FALSE)
end

function tests.processSpecs_shouldReturnConstrainedItemsPerGroup()
  local FRUIT = {'apple', 'orange'}
  local FRUIT_SET = set.Set(FRUIT)
  local COLOR = {'blue', 'green'}
  local COLOR_SET = set.Set(COLOR)

  local context = generator.createContext{
      attributes = {
          fruit = FRUIT,
          color = COLOR,
      },
  }

  local spec = {
      {fruit = 'banana'},
      {},  -- No explicit specs should default to random choice.
      {color = constraints.first}
  }
  local itemsPerGroup = {10, 1, 7}
  context:processSpecs(spec, itemsPerGroup)
  EQ(context:groupCount(), #spec)

  local group1 = context:getGroup(1)
  EQ(#group1, itemsPerGroup[1])
  for _, generatedTable in ipairs(group1) do
    EQ(generatedTable.fruit, 'banana')
    assert(COLOR_SET[generatedTable.color])
  end

  local group2 = context:getGroup(2)
  EQ(#group2, itemsPerGroup[2])
  for _, generatedTable in ipairs(group2) do
    assert(FRUIT_SET[generatedTable.fruit])
    assert(COLOR_SET[generatedTable.color])
  end

  local group3 = context:getGroup(3)
  EQ(#group3, itemsPerGroup[3])
  for _, generatedTable in ipairs(group3) do
    assert(FRUIT_SET[generatedTable.fruit])
    EQ(generatedTable.color, COLOR[1])
  end
end

function tests.processSpecs_shouldOnlyPutAttributesMarkedProcessInOutput()
  local function tableSize(t)
    local count = 0
    for _ in pairs(t) do count = count + 1 end
    return count
  end

  local TO_PROCESS = {'fruit'}
  local context = generator.createContext{
      attributes = {
          fruit = {'apple', 'orange'},
          color = {'blue', 'green'},
      },
      process = TO_PROCESS,
  }

  local spec = {
      {fruit = 'banana'},
      {ignored = 'true'},
      {color = constraints.first}
  }
  local itemsPerGroup = {10, 1, 7}

  context:processSpecs(spec, itemsPerGroup)
  EQ(context:groupCount(), #spec)
  for _, generatedTable in ipairs(context:getFlattenedOutput()) do
    -- Generated tables should contain nothing but fruit.
    NE(generatedTable.fruit, nil)
    EQ(tableSize(generatedTable), #TO_PROCESS)
  end
end

function tests.vocabulary_shouldReturnDeDuplicatedAttributeValues()
  local context = generator.createContext{
    attributes = {fruit = {'apple', 'orange'},
                  color = {'orange', 'red'}},
    process = {'fruit', 'color'},
  }
  local vocab = context:vocabulary()
  local EXPECTATION = listToHistogram({'apple', 'orange', 'red'})
  asserts.tablesEQ(listToHistogram(vocab), EXPECTATION)
end

function tests.vocabulary_shouldIncludeUserDictEntriesOnceOnly()
  local context = generator.createContext{
    attributes = {fruit = {'apple', 'orange'},
                  color = {'orange', 'red'}},
    process = {'fruit', 'color'},
  }
  local vocab = context:vocabulary{userDict = {'red', 'fox'}}
  local EXPECTATION = listToHistogram({'apple', 'orange', 'red', 'fox'})
  asserts.tablesEQ(listToHistogram(vocab), EXPECTATION)
end

function tests.vocabulary_shouldRestrictOutputIfExplicitAttributesGiven()
  local context = generator.createContext{
    attributes = {fruit = {'apple', 'orange'},
                  color = {'orange', 'red'}},
    process = {'fruit', 'color'},
  }
  local vocab = context:vocabulary{attributes = {'fruit'}}
  local EXPECTATION = listToHistogram({'apple', 'orange'})
  asserts.tablesEQ(listToHistogram(vocab), EXPECTATION)
end

return test_runner.run(tests)
