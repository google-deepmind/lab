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

--[[ Support generating random quantities of objects for language levels.

Functors will be called with no arguments, and should return a list of integer
values corresponding to the number of objects to generate for each group.  See
also object_generator.lua.
]]
local random = require 'common.random'

local item_count = {}

--[[ Returns a function which combines multiple counts into a single list.

Given arguments which are lists and/or functions, returns a nullary function
which copies elements of each list, in order, into the output list.  Functions
are called and expected to return a list which is copied in the same manner.

Examples:

    concat({1, 1}, {2, 4, 2}) => {1, 1, 2, 4, 2}
    concat({1}, function() return {2, 3, 4} end) => {1, 2, 3, 4}

Returns a nullary function which returns combined list of input parts.
]]
function item_count.concat(...)
  local inputs = {...}
  return function()
    local combined = {}
    for _, part in ipairs(inputs) do
      if type(part) == 'function' then part = part() end
      for _, value in ipairs(part) do combined[#combined + 1] = value end
    end
    return combined
  end
end


function item_count.createTwoGroups(kwargs)
  return function()
    local minGoal = kwargs.minGoal or 2
    local minOther = kwargs.minOther or 1
    local totalObjects = kwargs.totalObjects or
        random:uniformInt(minGoal + minOther, kwargs.maxObjects)
    local goalCount = random:uniformInt(minGoal, totalObjects - minOther)
    return {goalCount, totalObjects - goalCount}
  end
end

--[[ Returns a function which generates a list of integers tailored for object
generation group sizes.

Either totalObjects or maxObjects must be specified (see below).

Calling the returned function gives a list of integers which satisfy both the
individual minimum (and maxiumum if specified) value requirements for each
position, and also an overall total sum reqirement, which may itself be
randomised.

Keyword Arguments:

*   `groupMin` (list of integers) groupMin[n] specifies the minimum value of
    result[n].  Size of result will match this.
*   `groupMax` (optional list of integers) groupMax[n] specifies the maximum
    value of result[n].  If present, must be same size as groupMin.  An entry
    in the list of -1 specifies no maximum for that position.
*   `totalObjects` (integer) Specifies the desired total sum of the values in
    the result.  Must be >= 𝝨 groupMin.
*   `maxObjects` (integer) Specifies that a variable number of objects are
    desired.  On each call totalObjects is picked from the range
    [𝝨 groupMin, maxObjects]

Returns a nullary function which returns a list of integers when called.
]]
function item_count.createGroupCounts(kwargs)
  assert(kwargs.totalObjects and not kwargs.maxObjects or
             not kwargs.totalObjects and kwargs.maxObjects,
         'Must specify exactly one of totalObjects or maxObjects, not both.')

  local minObjects = 0
  for _, min in ipairs(kwargs.groupMin) do minObjects = minObjects + min end
  assert(minObjects <= (kwargs.totalObjects or kwargs.maxObjects))

  if kwargs.groupMax then
    assert(#kwargs.groupMax == #kwargs.groupMin)
    -- Ensure relative value of min and max is sane for each group.
    for ii = 1, #kwargs.groupMin do
      local max = kwargs.groupMax[ii]
      if max ~= -1 and max < kwargs.groupMin[ii] then
        error('groupMax of ' .. max .. ' less than groupMin of ' ..
                  kwargs.groupMin[ii] .. ' at index ' .. ii)
      end
    end
    -- Ensure per-group limits don't contradict overall target
    local checkMax = 0
    for _, max in ipairs(kwargs.groupMax) do
      if max == -1 then
        checkMax = -1 break
      else
        checkMax = checkMax + max
      end
    end
    assert(checkMax == -1 or
               checkMax >= (kwargs.totalObjects or kwargs.maxObjects))
  end

  return function()
    local function maxValueFor(current, remaining, maxForGroup)
      local max = current + remaining
      if maxForGroup and maxForGroup ~= -1 then
        max = maxForGroup < max and maxForGroup or max
      end
      return max
    end

    local function distribute(out, remaining, shuffledIndex, groupMax)
      while remaining > 0 do
        for i = 1, #out do
          local index = shuffledIndex[i]
          local current = out[index]
          out[index] = random:uniformInt(current,
                                      maxValueFor(current, remaining,
                                                  groupMax[index]))
          remaining = remaining - (out[index] - current)
        end
      end
      return out
    end

    local totalObjects = kwargs.totalObjects or
        random:uniformInt(minObjects, kwargs.maxObjects)
    local out = {}
    local shuffledIndex = {}
    for index, val in ipairs(kwargs.groupMin) do
      out[#out + 1] = val
      shuffledIndex[#shuffledIndex + 1] = index
    end
    random:shuffleInPlace(shuffledIndex)
    local remaining = totalObjects - minObjects
    return distribute(out, remaining, shuffledIndex, kwargs.groupMax or {})
  end
end

return item_count
