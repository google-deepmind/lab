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

-- Objects created by this code will only have their correct appearance if the
-- level has decorators/human_recognisable_pickups applied.

local combinatorics = require 'common.combinatorics'
local helpers = require 'common.helpers'
local image = require 'dmlab.system.image'
local pickups = require 'common.pickups'
local random = require 'common.random'
local set = require 'common.set'
local tensor = require 'dmlab.system.tensor'
local game = require 'dmlab.system.game'

local PATTERN_DIR = game:runFiles() .. '/baselab/game_scripts/patterns/'

local hrp = {}

local PREFIX = 'HRP_'

local SHAPES = {
    'apple2',
    'ball',
    'balloon',
    'banana',
    'bottle',
    'cake',
    'can',
    'car',
    'cassette',
    'chair',
    'cherries',
    'cow',
    'flower',
    'fork',
    'fridge',
    'guitar',
    'hair_brush',
    'hammer',
    'hat',
    'ice_lolly',
    'jug',
    'key',
    'knife',
    'ladder',
    'mug',
    'pencil',
    'pig',
    'pincer',
    'plant',
    'saxophone',
    'shoe',
    'spoon',
    'suitcase',
    'tennis_racket',
    'tomato',
    'toothbrush',
    'tree',
    'tv',
    'wine_glass',
    'zebra',
}

local SHAPES_SET = set.Set(SHAPES)

local TWO_COLOR_PATTERNS = {
    'chequered',
    'crosses',
    'diagonal_stripe',
    'discs',
    'hex',
    'pinstripe',
    'spots',
    'swirls',
}

local PATTERNS = helpers.shallowCopy(TWO_COLOR_PATTERNS)
PATTERNS[#PATTERNS + 1] = 'solid'

local PATTERNS_SET = set.Set(PATTERNS)

local SCALES = {
    small = 0.62,
    medium = 1.0,
    large = 1.62,
}

-- Per object scaling for objects where the default large size exceeds the
-- bounds of a maze cell.
local CUSTOM_SCALES = {
    banana = {large = 1.58},
    car = {large = 1.02, medium = 0.8},
    cassette = {large = 1.45, medium = 0.95},
    chair = {large = 1.15, medium = 0.84},
    cow = {large = 1.06, medium = 0.81},
    hat = {large = 1.55},
    pig = {large = 1.52},
}

local COLORS = {
    {0, 0, 0},
    {0, 0, 170},
    {0, 170, 0},
    {0, 170, 170},
    {170, 0, 0},
    {170, 0, 170},
    {170, 85, 0},
    {170, 170, 170},
    {85, 85, 85},
    {85, 85, 255},
    {85, 255, 85},
    {85, 255, 255},
    {255, 85, 85},
    {255, 85, 255},
    {255, 255, 85},
    {255, 255, 255},
}

-- Store loaded textures.
hrp._patternTextures = {}

hrp._currentPickups = {}
hrp._currentPickupsId = 0

-- Convert a scale name to a number, accounting for per-shape overrides.
local function scaleNameToNumber(scale, shape)
  return CUSTOM_SCALES[shape] and CUSTOM_SCALES[shape][scale] or SCALES[scale]
end

-- Returns unsigned byte texture, loading once from disk if need be.
function hrp.getPatternTexture(patternName, width, height)
  if not PATTERNS_SET[patternName] then
    error("Unknown pattern: " .. tostring(patternName))
  end

  if not hrp._patternTextures[patternName] then
    local byteImage
    if patternName == 'solid' then
      byteImage = tensor.ByteTensor(width, height, 4):fill(255)
    else
      local path = PATTERN_DIR .. patternName .. '_d.png'
      byteImage = image.load(path)
      local patHeight, patWidth = unpack(byteImage:shape())
      if patWidth ~= width or patHeight ~= height then
        byteImage =
            image.scale(byteImage:narrow(3, 1, 1):clone(), width, height)
      end
    end
    hrp._patternTextures[patternName] = byteImage
  end
  return hrp._patternTextures[patternName]
end


--[[ Create a single object.

The keywords are an object specification returned from specifyUniquePickups()
or similar functions.

Keyword arguments:

    * `color1` (table, required) Red-green-blue color triplet e.g.
       {0, 128, 255}.
    * `color2` (table, required) Ignored if `pattern` is solid. If equal to
      `color1`, the object will be solid-colored regardless of the value of
      `pattern`.
    * `moveType` (number, default=pickups.moveType.BOB) Whether the object
      stays exactly still or bobs up and down slightly.
    * `pattern` (string, required) Must occur in PATTERNS.
    * `quantity` (number, default=0) For some levels, the reward for picking
      up this object.
    * `scale` (number or string, default=1) Uniform scale transformation
      applied to object; can be 'small', 'medium', or 'large'.
    * `shape` (string, required) Must occur in SHAPES.

Returns classname of described pickup.
--]]
function hrp.create(kwargs)
  local shape = kwargs.shape or error("Missing shape")
  if not SHAPES_SET[shape] then error("Unknown shape: " .. tostring(shape)) end

  local scale = 1.0
  if kwargs.scale then
    if type(kwargs.scale) == 'number' then
      scale = kwargs.scale
    else
      assert(type(kwargs.scale) == 'string')
      scale = scaleNameToNumber(kwargs.scale, shape)
      assert(scale, '"' .. tostring(kwargs.scale) .. '" is not a valid scale.')
    end
  end

  -- Create a key for caching that encompasses all possible values.
  local color1Key = table.concat(kwargs.color1, ',')
  local color2Key = kwargs.pattern == 'solid' and color1Key
                    or table.concat(kwargs.color2, ',')
  local key = table.concat(
    {shape, tostring(scale), kwargs.pattern, color1Key, color2Key}, ",")

  if hrp._modelCache[key] == nil then
    local transformSuffix = scale ~= 1 and '%scale{' .. scale .. '}' or ''

    -- Register the requested configuration against the ID for later lookup by
    -- modifyTexture.
    hrp._objectConfigs[hrp._nextId] = {
        pattern = kwargs.pattern or error("Missing pattern"),
        color1 = kwargs.color1 or error("Missing color1"),
        color2 = kwargs.color2 or error("Missing color2"),
    }

    hrp._modelCache[key] = string.format('%s%d:models/hr_%s.md3%s',
                                         PREFIX, hrp._nextId, shape,
                                         transformSuffix)
    hrp._nextId = hrp._nextId + 1
  end
  hrp._currentPickupsId = hrp._currentPickupsId + 1
  local classname = 'pu:' .. hrp._currentPickupsId
  hrp._currentPickups[classname] = {
      name = classname,
      classname = classname,
      model = hrp._modelCache[key],
      quantity = kwargs.quantity or 0,
      type = pickups.type.REWARD,
      moveType = kwargs.moveType or pickups.moveType.BOB,
  }
  return classname
end

function hrp.pickup(classname)
  return hrp._currentPickups[classname]
end

function hrp.replaceModelName(modelName)
  if modelName:sub(1, #PREFIX) == PREFIX then
    local prefixTexture, newModelName = modelName:match('(.*:)(.*)')
    return newModelName, prefixTexture
  end
end

function hrp.replaceTextureName(textureName)
  if textureName:sub(1, #PREFIX) == PREFIX then
    -- Strip prefix and id.
    return textureName:match('.*:(.*)')
  end
end

function hrp.applyPatternAndColors(objTexture, patternTexture, color1, color2)
  image.setMaskedPattern(objTexture, patternTexture, color1, color2)
end

function hrp.modifyTexture(textureName, texture)
  if textureName:sub(1, #PREFIX) == PREFIX then
    local id = tonumber(textureName:match('(%d+):'))
    local config = hrp._objectConfigs[id]
    local shape = texture:shape()
    local patternTexture = hrp.getPatternTexture(config.pattern, shape[1],
                                                 shape[2])
    hrp.applyPatternAndColors(texture, patternTexture, config.color1,
                              config.color2)
    return true
  end
  return false
end

function hrp.shapes()
  return SHAPES
end

function hrp.patterns()
  return PATTERNS
end

function hrp.twoColorPatterns()
  return TWO_COLOR_PATTERNS
end

function hrp.colors()
  return COLORS
end


-- Return an array of n object specifications, all of which have a unique set
-- of colors, shapes, and patterns.
function hrp.uniquePickups(n)
  local shapes = hrp.shapes()
  local patterns = hrp.twoColorPatterns()
  local colors = hrp.colors()

  return hrp.specifyUniquePickups(n, shapes, patterns, colors)
end


-- Return an array of n object specifications, all of which have a unique set
-- of colors, shapes, and patterns.
function hrp.specifyUniquePickups(n, shapes, patterns, colors, colorNames)
  -- There are `count` unique pickups: #shapes * #patterns * choose(2, #colors)
  -- where choose(2, #colors) is the number of ways to select two different
  -- colors, ignoring order.
  local count = #shapes * #patterns * combinatorics.choose(2, #colors)
  assert(n <= count, "Requesting more unique pickups than can be generated.")
  assert(colorNames == nil or #colorNames == #colors,
      "Must have as many color names as colors.")

  local result = {}
  local idGenerator = random:shuffledIndexGenerator(count)
  for i = 1, n do
    -- id represents one of the unique pickups. 0 <= id < count.
    -- We will convert id to its shape, pattern, and two colors.
    local id = idGenerator() - 1

    -- Modulo-out the shape and pattern dimensions from id.
    local shape = (id % #shapes) + 1
    id = math.floor(id / #shapes)

    local pattern = (id % #patterns) + 1
    id = math.floor(id / #patterns)

    -- Remaining id value is between 0 and C(#colors,2) - 1.
    -- colors[0] < colors[1], so randomly swap them for more variability.
    local colorPair = combinatorics.twoItemSelection(id + 1, #colors)
    local swap = random:uniformInt(0, 1) == 0
    local color1 = swap and colorPair[2] or colorPair[1]
    local color2 = swap and colorPair[1] or colorPair[2]
    result[i] = {
        shape = shapes[shape],
        pattern = patterns[pattern],
        color1 = colors[color1],
        color2 = colors[color2]
    }
    if colorNames then
      result[i].colorName1 = colorNames[color1]
      result[i].colorName2 = colorNames[color2]
    end
  end
  return result
end

-- Return an array of n object specifications, all of which have a unique
-- shape, and which also have patterns of colors applied.
function hrp.uniquelyShapedPickups(n)
  local shapes = hrp.shapes()
  local patterns = hrp.twoColorPatterns()
  local colors = hrp.colors()

  return hrp.specifyUniquelyShapedPickups(n, shapes, patterns, colors)
end

-- Return an array of n object specifications, all of which have a unique
-- shape, and which also have patterns of colors applied.
function hrp.specifyUniquelyShapedPickups(n, shapes, patterns, colors,
                                          colorNames)
  assert(n <= #shapes, "Requesting more unique pickups than can be generated.")
  assert(colorNames == nil or #colorNames == #colors,
      "Must have as many color names as colors.")

  local result = {}
  local idGenerator = random:shuffledIndexGenerator(#shapes)
  for i = 1, n do
    -- id represents one of the unique pickups. 0 <= id < count.
    -- We will convert id to its shape, pattern, and two colors.
    local shape = idGenerator()

    local pattern = random:uniformInt(1, #patterns)
    local colorPairIndex =
        random:uniformInt(1, combinatorics.choose(2, #colors) - 1)

    -- Remaining id value is between 0 and C(#colors,2) - 1.
    -- colors[0] < colors[1], so randomly swap them for more variability.
    local colorPair = combinatorics.twoItemSelection(colorPairIndex, #colors)
    local swap = random:uniformInt(0, 1) == 0
    local color1 = swap and colorPair[2] or colorPair[1]
    local color2 = swap and colorPair[1] or colorPair[2]
    result[i] = {
        shape = shapes[shape],
        pattern = patterns[pattern],
        color1 = colors[color1],
        color2 = colors[color2]
    }
    if colorNames then
      result[i].colorName1 = colorNames[color1]
      result[i].colorName2 = colorNames[color2]
    end
  end
  return result
end

function hrp.reset()
  hrp._nextId = 1
  hrp._objectConfigs = {}
  hrp._modelCache = {}
  hrp._currentPickups = {}
  hrp._currentPickupsId = 0
end

function hrp.pickupCount()
  return hrp._currentPickupsId
end

hrp.reset()

return hrp
