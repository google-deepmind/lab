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
local hrp = require 'common.human_recognisable_pickups'
local tensor = require 'dmlab.system.tensor'
local random = require 'common.random'

local tests = {}

local RED = {255, 0, 0}
local GREEN = {0, 255, 0}

function tests.allPatternsShouldBeValidAndLoad()
  for _, patternName in ipairs(hrp.patterns()) do
    local patternTexture = hrp.getPatternTexture(patternName, 1024, 1024)
    assert(patternTexture)
    asserts.EQ(patternTexture:type(), 'deepmind.lab.tensor.ByteTensor')
    local _, _, c = unpack(patternTexture:shape())
    asserts.tablesEQ(patternTexture:shape(), {1024, 1024, c})
  end
end

function tests.allScalesShouldBeValidToCreate()
  local SHAPE = hrp.shapes()[1]
  local PATTERN = hrp.patterns()[1]
  hrp.reset()
  local DEFAULT_PICKUP = hrp.pickup(hrp.create{
        shape = SHAPE,
        pattern = PATTERN,
        color1 = GREEN,
        color2 = RED,
  })
  for _, scale in ipairs({'small', 'medium', 'large'}) do
    hrp.reset()
    local pickup = hrp.pickup(hrp.create{
        shape = SHAPE,
        pattern = PATTERN,
        color1 = GREEN,
        color2 = RED,
        scale = scale,
    })
    assert(pickup, 'Pickup failed to be created.')
    if scale == 'medium' then
      asserts.tablesEQ(pickup, DEFAULT_PICKUP,
        'Tables of same scales should match.')
    else
      asserts.tablesNE(pickup, DEFAULT_PICKUP,
        'Tables of different scales should not match.')
    end
  end
end


function tests.doubleScalesShouldBeValidToCreate()
  local SHAPE = hrp.shapes()[1]
  local PATTERN = hrp.patterns()[1]
  hrp.reset()
  local DEFAULT_PICKUP = hrp.pickup(hrp.create{
        shape = SHAPE,
        pattern = PATTERN,
        color1 = GREEN,
        color2 = RED,
  })
  for _, scale in ipairs({0.5, 1.0, 2.0}) do
    hrp.reset()
    local pickup = hrp.pickup(hrp.create{
        shape = SHAPE,
        pattern = PATTERN,
        color1 = GREEN,
        color2 = RED,
        scale = scale,
    })
    assert(pickup, 'Pickup failed to be created.')
    if scale == 1.0 then
      asserts.tablesEQ(pickup, DEFAULT_PICKUP,
        'Tables of same scales should match.')
    else
      asserts.tablesNE(pickup, DEFAULT_PICKUP,
        'Tables of different scales should not match.')
    end
  end
end

function tests.allShapesShouldBeValidToCreate()
  local PATTERN = hrp.patterns()[1]
  for _, shape in ipairs(hrp.shapes()) do
    -- Must provide shape, pattern and a pair of colours.
    local pickup = hrp.pickup(hrp.create{
        shape = shape,
        pattern = PATTERN,
        color1 = GREEN,
        color2 = RED
    })
    assert(pickup)
    asserts.EQ(type(pickup), 'table')
  end
end

function tests.applyPatternAndColors_doesNothingIfNoAlphaComponent()
  local texture = tensor.ByteTensor(1024, 1024, 4)
  texture:fill(127)
  texture:select(3, 4):fill(0)
  local expected = texture:clone()
  expected:select(3, 4):fill(255)
  local pattern = tensor.ByteTensor(1024, 1024, 1):fill(255)

  hrp.applyPatternAndColors(texture, pattern, RED, GREEN)
  asserts.EQ(texture, expected)
end

function tests.applyPatternAndColors_ShouldApplyColor1WhenPatternIs1()
  -- Start with a white texture.
  local texture = tensor.ByteTensor(1024, 1024, 4)
  texture:fill(255)

  -- Create pattern of all 1's.
  local pattern = tensor.ByteTensor(1024, 1024, 1):fill(255)

  -- Expect everything to go red.
  local expected = tensor.ByteTensor(1024, 1024, 4)
  expected:select(3, 1):fill(255)
  expected:select(3, 4):fill(255)

  hrp.applyPatternAndColors(texture, pattern, RED, GREEN)
  asserts.EQ(texture, expected)
end

function tests.applyPatternAndColors_ShouldApplyColor2WhenPatternIs0()
  -- Start with a white texture.
  local texture = tensor.ByteTensor(1024, 1024, 4)
  texture:fill(255)

  -- Create pattern of all 0's.
  local pattern = tensor.ByteTensor(1024, 1024, 1)

  -- Expect everything to go green.
  local expected = tensor.ByteTensor(1024, 1024, 4)
  expected:select(3, 2):fill(255)
  expected:select(3, 4):fill(255)

  hrp.applyPatternAndColors(texture, pattern, RED, GREEN)
  asserts.EQ(texture, expected)
end

function tests.applyPatternAndColors_ShouldApplyBothColors()
  -- Start with a white texture, with the bottom half transparent.
  local texture = tensor.ByteTensor(1024, 1024, 4)
  texture:fill(255)
  texture:narrow(1, 513, 512):select(3, 4):fill(0)

  -- Create pattern with 1's in the left half, zero's in the right.
  local pattern = tensor.ByteTensor(1024, 1024, 1)
  pattern:narrow(2, 1, 512):fill(255)

  -- Expect top half to be red and green, bottom half still white.
  local expected = tensor.ByteTensor(1024, 1024, 4)
  local top = expected:narrow(1, 1, 512)
  top:narrow(2, 1, 512):select(3, 1):fill(255)
  top:narrow(2, 513, 512):select(3, 2):fill(255)
  top:select(3, 4):fill(255)

  expected:narrow(1, 513, 512):fill(255)

  hrp.applyPatternAndColors(texture, pattern, RED, GREEN)
  asserts.EQ(texture, expected)
end

-- Should succeed and return an empty table.
function tests.uniquePickups_empty()
  local pickups = hrp.uniquePickups(0)
  assert(next(pickups) == nil)
end

-- Should generate an array with one pickup.
function tests.uniquePickups_one()
  local pickups = hrp.uniquePickups(1)
  asserts.EQ(1, #pickups)
end

-- Ensure the generation is random.
function tests.uniquePickups_random()
  random:seed(1)
  local pickups1 = hrp.uniquePickups(100)

  random:seed(2)
  local pickups2 = hrp.uniquePickups(100)

  asserts.tablesNE(pickups1, pickups2)
end

-- Ensure the randomness is deterministic.
function tests.uniquePickups_repeatable()
  random:seed(1)
  local pickups1 = hrp.uniquePickups(100)

  random:seed(1)
  local pickups2 = hrp.uniquePickups(100)

  asserts.tablesEQ(pickups1, pickups2)
end

-- Should succeed and return an empty table.
function tests.uniquelyShapedPickups_empty()
  local pickups = hrp.uniquelyShapedPickups(0)
  assert(next(pickups) == nil)
end

-- Should generate an array with one pickup.
function tests.uniquelyShapedPickups_one()
  local pickups = hrp.uniquelyShapedPickups(1)
  asserts.EQ(1, #pickups)
end

-- Ensure the generation is random.
function tests.uniquelyShapedPickups_random()
  random:seed(1)
  local pickups1 = hrp.uniquelyShapedPickups(#hrp.shapes())

  random:seed(2)
  local pickups2 = hrp.uniquelyShapedPickups(#hrp.shapes())

  asserts.tablesNE(pickups1, pickups2)
end

-- Ensure the randomness is deterministic.
function tests.uniquelyShapedPickups_repeatable()
  random:seed(1)
  local pickups1 = hrp.uniquelyShapedPickups(#hrp.shapes())

  random:seed(1)
  local pickups2 = hrp.uniquelyShapedPickups(#hrp.shapes())

  asserts.tablesEQ(pickups1, pickups2)
end

-- Ensure shapes aren't repeated.
function tests.uniquelyShapedPickups_unique()
  random:seed(1)
  local pickups = hrp.uniquelyShapedPickups(#hrp.shapes())

  local shapes = {}
  for i, o in ipairs(pickups) do
    asserts.EQ(shapes[o.shape], nil)
    shapes[o.shape] = true
  end
end

return test_runner.run(tests)
