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

local colors = require 'common.colors'
local generator = require 'language.generator'
local random = require 'common.random'

local object_generator = {
    SHAPES = {
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
    },

    PATTERNS = {
        'chequered',
        'crosses',
        'diagonal_stripe',
        'discs',
        'hex',
        'pinstripe',
        'solid',
        'spots',
        'swirls',
    },

    SIZES = {
        'small',
        'medium',
        'large'
    },

    -- Will be populated with color names from COLOR_NAME_TO_HSL below.
    COLORS = {}
}

-- Named colors in HSL space currently supported.  HSL allows us to produce
-- light and dark versions easily by modifying the L component.
local COLOR_NAME_TO_HSL = {  -- 0-360, 0-100, 0-100
    black = {0, 0, 0},
    gray = {0, 0, 50},
    white = {0, 0, 100},
    red = {0, 100, 50},
    orange = {30, 100, 50},
    brown = {30, 51, 40},
    yellow = {60, 100, 50},
    green = {120, 100, 50},
    cyan = {180, 100, 50},
    blue = {240, 100, 50},
    purple = {270, 100, 50},
    magenta = {300, 100, 50},
    pink = {330, 100, 50},
}


-- Populate the default set of color names for use by objects.
for name, _ in pairs(COLOR_NAME_TO_HSL) do
  object_generator.COLORS[#object_generator.COLORS + 1] = name
end

function object_generator.pluralizeShape(count, singular, plural)
  local exceptions = {
      cherries = 'cherries',
      hair_brush = 'hair_brushes',
      ice_lolly = 'ice_lollies',
      knife = 'knives',
      toothbrush = 'toothbrushes',
      wine_glass = 'wine_glasses',
  }
  if count == 1 then
    return singular
  elseif not plural then
    plural = exceptions[singular] or singular .. 's'
  end
  return plural
end


--[[ Returns a new language.generator context specialised for language object
attributes.

With no arguments, produces a generator which will return tables with the
following keys and possible values:

*   `color` values from known color names, object_generator.COLORS
*   `color2` values from known color names, object_generator.COLORS
*   `nextTo` nil
*   `noise` 1
*   `pattern` values from known pattern names, object_generator.PATTERNS
*   `region` 'all'
*   `requires` nil
*   `reward` 0 -- the default change to score when picked up
*   `reward2` 0 -- auxiliary value used by some reward controllers
*   `shade` ''
*   `shape` name of known recognisable pickups, object_generator.SHAPES
*   `size` size from object_generator.SIZES

Keyword arguments:

*   `attributes` (table) if supplied, can be used with any of the above keys to
    provide an alternative list of candidate values for that attribute.
*   `attributeDefaults` (table) if supplied, keys are attribute names, values
    are used as the default spec for that attribute if none is supplied
    explicitly at processing time.
]]
function object_generator.createContext(kwargs)
  local kwargs = kwargs or {}
  local choices = kwargs.attributes or {}
  local attributes = {
      color = choices.color or object_generator.COLORS,
      color2 = choices.color2 or object_generator.COLORS,
      nextTo = choices.nextTo or {nil},
      noise = choices.noise or {1},  -- stddev of noise
      pattern = choices.pattern or object_generator.PATTERNS,
      region = choices.region or {nil},
      requires = choices.requires or {nil},
      shade = choices.shade or {''},
      shape = choices.shape or object_generator.SHAPES,
      size = choices.size or object_generator.SIZES,
  }
  local defaults = {
      region = 'any',
      reward = 0,
      reward2 = 0,
      static = false,
      canPickup = true,
  }
  for attribute, default in pairs(kwargs.attributeDefaults or {}) do
    defaults[attribute] = default
  end

  return generator.createContext{
      attributes = attributes,
      process = {
          'color', 'color2', 'nextTo', 'noise', 'pattern', 'region', 'requires',
          'reward', 'reward2', 'shade', 'shape', 'size', 'static', 'canPickup'
      },
      defaults = defaults,
  }
end

--[[ Produces customised colours by shading and adding noise.

Arguments:

*   `colorName` (string) Colour name string - corresponds to color table above.
*   `noiseStddev` (number) Standard Deviation of the Gaussian colour noise.
*   `shade` (one of {"dark", "light", ""}) Optional light or dark shading.

Returns array containing R,G,B ∈ [0, 255].
]]
function object_generator.generateRgbColor(colorName, noiseStddev, shade)
  local h, s, l = unpack(COLOR_NAME_TO_HSL[colorName])
  if not h then
    error('No representation found for color: ' .. colorName)
  end

  noiseStddev = noiseStddev or 0
  shade = shade or ''

  if shade == 'dark' then
    l = l * 0.5
  elseif shade == 'light' then
    l = l * 1.5
    if l > 100 then l = 100 end
  elseif shade ~= '' then
    error('Invalid value for shade: ' .. shade)
  end

  local function clamp(v)
    if v < 0 then v = 0 elseif v > 255 then v = 255 end
    return v
  end

  local r, g, b = colors.hslToRgb(h, s / 100, l / 100)
  -- Apply Gaussian random noise to the colour in rgb space
  if noiseStddev > 0 then
    r = math.floor(clamp(r + random:normalDistribution(0, noiseStddev)))
    g = math.floor(clamp(g + random:normalDistribution(0, noiseStddev)))
    b = math.floor(clamp(b + random:normalDistribution(0, noiseStddev)))
  end

  return {r, g, b}
end

return object_generator
