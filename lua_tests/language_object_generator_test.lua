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
local object_generator = require 'language.object_generator'
local set = require 'common.set'

local EQ = asserts.EQ
local NE = asserts.NE

local tests = {}

local generator = require 'language.generator'
local make = require 'language.make'

function tests.processObjectSpecs_SingleGoalSpec()
  local spec = {
      {reward = 10, size = "large", shape = make.random()}
  }

  local context = object_generator.createContext()
  context:processSpecs(spec, {1})

  NE(context, nil)
  EQ(context:groupCount(), 1)
  local group = context:getGroup(1)
  EQ(#group, 1)
  local values = group[1]
  EQ(values.reward, 10)
  EQ(values.size, "large")
  local expectedShapes = set.Set(context.attributes.shape)
  -- Check that the random shape that was chosen is in the selection set
  assert(expectedShapes[values.shape], true)
end

function tests.processObjectSpecs_TwoGroupsMatchingShapes()
  local spec = {
      {reward = 10, size = "large", shape = make.random()},
      {reward = -4, size = {make.isNot("large"), make.random()},
       shape = make.sameAs(1)}
  }

  local context = object_generator.createContext()
  context:processSpecs(spec, {1, 2})

  EQ(context:groupCount(), 2)
  EQ(#context:getFlattenedOutput(), 3, "Expected 3 output values")
  local group1 = context:getGroup(1)
  EQ(#group1, 1)

  local group2 = context:getGroup(2)
  EQ(#group2, 2)
  local EXPECTED_GROUP2_SIZES = set.Set{'small', 'medium'}
  for _, values in ipairs(group2) do
    EQ(values.reward, -4)
    NE(values.size, "large")
    assert(EXPECTED_GROUP2_SIZES[values.size])
    EQ(values.shape, group1[1].shape)
  end
end

function tests.processObjectSpecs_TwoGroupsUniqueSizes()
  local spec = {
      {reward = 10, size = "large", shape = make.random()},
      {reward = -4, size = {make.notSameAs(1), make.unique()},
       shape = make.sameAs(1)}
  }

  local context = object_generator.createContext()
  context:processSpecs(spec, {1, 2})

  local seenSizes = {}
  for _, values in ipairs(context:getFlattenedOutput()) do
    local seen_so_far = seenSizes[values.size] or 0
    seenSizes[values.size] = seen_so_far + 1
  end
  local EXPECTATION = {small = 1, medium = 1, large = 1}
  asserts.tablesEQ(seenSizes, EXPECTATION)
end

function tests.processObjectSpecs_NegationExample()
  -- Generalised case of "find the object that isn't blue" without hard-coding
  -- the color.
  local spec = {
      {reward = 10, color = make.random()()},
      {reward = -4, color = {make.notSameAs(1), make.commonRandom()}},
  }

  local context = object_generator.createContext()
  context:processSpecs(spec, {1, 49})

  local seenColors = {}
  for _, values in ipairs(context:getFlattenedOutput()) do
    local seen_so_far = seenColors[values.color] or 0
    seenColors[values.color] = seen_so_far + 1
  end

  local EXPECTATION = {}
  EXPECTATION[context:getGroup(1)[1].color] = 1
  EXPECTATION[context:getGroup(2)[1].color] = 49

  asserts.tablesEQ(seenColors, EXPECTATION)
end

function tests.processObjectSpecs_ShouldNotRequireReward()
  local spec = {
      {reward = 10},
      {},
  }

  local context = object_generator.createContext()
  context:processSpecs(spec, {1, 1})

  EQ(context:getGroup(1)[1].reward, 10)
  EQ(context:getGroup(2)[1].reward, 0)
end

function tests.createContext_supplyCustomChoices()
  -- Restrict the choice of shapes and colors to choose from:
  local SHAPES = {'cat', 'dog', 'frog'}
  local COLORS = {'pink', 'brown'}

  local context = object_generator.createContext{
      attributes = {
          shape = SHAPES,
          color = COLORS,
      },
  }
  local spec = {
      {shape = make.random(), color = make.random()}
  }

  context:processSpecs(spec, {10})
  local group = context:getGroup(1)
  EQ(#group, 10)
  local EXPECTED_SHAPES = set.Set(SHAPES)
  local EXPECTED_COLORS = set.Set(COLORS)
  for _, values in ipairs(group) do
    assert(EXPECTED_SHAPES[values.shape])
    assert(EXPECTED_COLORS[values.color])
  end
end

function tests.createContext_attributeDefaultsAllowsSelectiveOverrides()
  local DEFAULT_REWARD = 0
  local DEFAULT_REGION = 'any'
  -- Check defaults
  local context = object_generator.createContext()
  EQ(context:defaultSpecForAttribute('reward'), DEFAULT_REWARD)
  EQ(context:defaultSpecForAttribute('region'), DEFAULT_REGION)

  -- Override reward, check region is unchanged.
  local NEW_REWARD = -10
  context = object_generator.createContext{
      attributeDefaults = {
          reward = NEW_REWARD
      }
  }
  EQ(context:defaultSpecForAttribute('reward'), NEW_REWARD)
  EQ(context:defaultSpecForAttribute('region'), DEFAULT_REGION)
end

return test_runner.run(tests)
