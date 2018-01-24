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

-- A user-controlled system for pseudo-random number generation.
-- Helper functions also use this as a source of randomness.

local sys_random = require 'dmlab.system.random'

local MAP_COUNT = 100000
local THEME_COUNT = 1000

local random = {}

--[[ Set the seed of the underlying pseudo-random-bit generator. The argument
may be a number or a string representation of a number. Beware of precision loss
when using very large numeric values.

It is probably useful to call this function with the per-episode seed in the
"start" callback so that episodes play out reproducibly. ]]
function random:seed(value)
  return self._rng:seed(value)
end

-- Returns an integer sampled uniformly at random from the closed range
-- [1, MAP_COUNT].
function random:mapGenerationSeed()
  return self._rng:uniformInt(1, MAP_COUNT)
end


-- Returns an integer sampled uniformly at random from the closed range
-- [1, THEME_COUNT].
function random:themeGenerationSeed()
  return self._rng:uniformInt(1, THEME_COUNT)
end

-- Returns an integer sampled uniformly at random from the closed range
-- [lower, upper].
function random:uniformInt(lower, upper)
  return self._rng:uniformInt(lower, upper)
end

-- Returns a real number sampled uniformly at random from the half-open range
-- [lower, upper).
function random:uniformReal(lower, upper)
  return self._rng:uniformReal(lower, upper)
end

-- Returns an integer in the range [1, n] where the probability of each
-- integer i is the ith weight divided by the sum of the n weights.
function random:discreteDistribution(weights)
  return self._rng:discreteDistribution(weights)
end

-- Returns a real number sampled from the random distrubution centered around
-- mean with standard distribution stddev.
function random:normalDistribution(mean, stddev)
  return self._rng:normalDistribution(mean, stddev)
end

-- Returns an 8-bit triplet representing a random RGB color:
-- {0~255, 0~255, 0~255}.
function random:color()
  local function uniform255() return self._rng:uniformInt(0, 255) end
  return {uniform255(), uniform255(), uniform255()}
end

-- Returns an element sampled uniformly at random from a table. You can specify
-- a consecutive part of the table to sample by specifying startIndex and
-- endIndex. By default the entire table will be used.
function random:choice(t, startIndex, endIndex)
  t = t or {}
  if not startIndex or startIndex < 1 then
    startIndex = 1
  end
  if not endIndex or endIndex > #t then
    endIndex = #t
  end
  if startIndex > endIndex then
    return nil
  end
  return t[self._rng:uniformInt(startIndex, endIndex)]
end

-- Shuffles a Lua array in place. If n is given, the shuffle stops after the
-- first n elements have been placed. 'n' is clamped to the size of the array.
function random:shuffleInPlace(array, n)
  local c = (n and n < #array) and n or #array - 1
  for i = 1, c do
    local j = self._rng:uniformInt(i, #array)
    array[j], array[i] = array[i], array[j]
  end
  return array
end

-- Returns a shuffled copy of a Lua array.
function random:shuffle(array)
  local ret = {}
  for i, obj in ipairs(array) do
    ret[i] = obj
  end
  return self:shuffleInPlace(ret)
end

function random:generator()
  return self._rng
end

--[[ Returns a 'sampling without replacement' number generator with the integer
range range [1, count].

Generator's memory grows linearly. Initialization and calling costs are both
O(1).

The generator returns nil when no new samples are available and resets to its
initial state.
--]]
function random:shuffledIndexGenerator(count)
  -- The number of values that have been generated so far.
  local current = 0

  -- A sparse array of shuffled values.
  local elements = {}

  --[[
                             All values <= current that are *not* in
                             the completed shuffling.
                completed    These values are at indices that *are* in
                shuffling    the completed shuffling.

     elements [+++++++++++|--+---------+--++----------+----------+--]
               .         .                                         .
               .         .                                         .
               1      current                                    count

   Invariant A: All values <= current will be in `elements`.
   Invariant B: Every value in the completed shuffling has
                elements[value] ~= nil.

   The algorithm starts with current = 0 and elements empty, so the
   invariants start true.

   The algorithm increments as follows,
      1. Increment current
          current = current + 1

      2. Generate `random`, a random value in the range [current, count]
          random = randomNumberInRange(current, count)

      3. Basic operation:
           (a) output `random` as the next completed shuffling value; that is,
               insert `random` at elements[current].
           elements[random] is nil ==> elements[current] = random

           (b) insert `current` at elements[random].
           elements[current] is nil ==> elements[random] = current

         Exception operations:
           - If elements[current] already has a value, then (by invariant B),
             current is already in the completed shuffling. In this case we
             should avoid (b) and instead push the existing elements[current]
             back into elements[random].
           elements[current] has a value ==>
               elements[random] = elements[current]

           - If elements[random] already has a value, then (by invariant B),
             random is already in the completed shuffling. In this case we
             should avoid (a) and instead output the deferred value
             elements[random] in the completed shuffling.
           elements[random] has a value ==>
               elements[current] = elements[random]

   Note that both the basic and exception operations maintain the invariants,
   so by induction the invariants are always true.
  --]]
  return function()
    -- If we've shuffled all the elements, return nil for one call and reset
    -- to initial conditions. The caller can start again if a new shuffling is
    -- desired.
    if count == current then
      elements = {}
      current = 0
      return nil
    end

    -- Step 1.
    current = current + 1

    -- Step 2.
    local random = self._rng:uniformInt(current, count)

    -- Step 3.
    local currentStartValue = elements[current]
    elements[current] = elements[random] or random
    elements[random] = currentStartValue or current

    -- Return the tail of the completed shuffling that we just generated.
    return elements[current]
  end
end

local randomMT = {__index = random}

function randomMT:__call(rng)
  return setmetatable({_rng = rng}, randomMT)
end

return setmetatable({_rng = sys_random}, randomMT)
