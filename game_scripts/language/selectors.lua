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

local random = require 'common.random'

-- Selectors return a choice from a list of possible choices when called.
local selectors = {}

local function validateChoices(choices, name)
  if #choices == 0 then error('No choices given to selector: ' .. name) end
  local count = 0
  for index, value in pairs(choices) do
    count = count + 1
    if not value then
      error('Missing choice in ' .. name .. ' selector at index ' .. index)
    end
  end
  if count ~= #choices then
    error('Mismatch between #choices and item count in ' .. name ..
              ' selector - is one of the choices nil?')
  end
end

-- Given choice, return choice.
function selectors.createIdentity(choice)
  assert(choice, 'Missing choice in identity selector')

  return function()
    return choice
  end
end

-- Given choices {a,b,c} returns a, b, c, a, b, c, a ....
function selectors.createOrdered(choices)
  validateChoices(choices, 'ordered')

  local persistIndex = 0
  return function()
    persistIndex = persistIndex % #choices + 1
    return choices[persistIndex]
  end
end

function selectors.createRandom(choices)
  validateChoices(choices, 'random')

  return function()
    return random:choice(choices)
  end
end

-- Given list pairs of the form {weight, choice}, returns a choice with
-- probability of choosing choice[i] being weight[i] / ∑(weights)
function selectors.createDiscreteDistribution(weightChoicePairs)
  validateChoices(weightChoicePairs, 'discreteDistribution')
  local weights = {}
  local choices = {}
  for index, pair in ipairs(weightChoicePairs) do
    local weight = pair[1] or error('Missing weight at index ' .. index)
    local choice = pair[2] or error('Missing choice at index ' .. index)
    weights[index] = weight
    choices[index] = choice
  end

  return function()
    return choices[random:discreteDistribution(weights)]
  end
end


-- Wraps a selector that returns functions, returning the result of calling the
-- selected choice.
function selectors.createCalling(selector)
  return function()
    return selector()()
  end
end

return selectors
