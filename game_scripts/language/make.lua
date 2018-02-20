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

--[[ Functors to build language level object constraint sets.

This module provides methods for building specifications for use with the
lang_generator module.

It provides functions which return functors that conform to the calling
signature expected by context.applyConstraints().
]]

local constraints = require 'language.constraints'

local make = {}

function make.isNot(itemsToExclude)
  return function(src, unusedContext, unusedAttr, unusedGroupNum)
    return constraints.remove(src, itemsToExclude)
  end
end

--[[ Candidates are replaced with items.

Note that this narrows the available candidates to those in the list, a
subsequent operation (such as random or unique) can be used to select one.
Alternatively use oneOf() to automatically pick one at random.
]]
function make.choices(items)
  return function(unusedSrc, unusedContext, unusedAttr, unusedGroupNum)
    return constraints.copy(items)
  end
end

-- Define an explicit list of choices, and return one of them at random.
function make.oneOf(items)
  return function(unusedSrc, unusedContext, unusedAttr, unusedGroupNum)
    return constraints.random(items)
  end
end

-- Candidates are intersected with items.
function make.keep(itemsToKeep)
  return function(src, unusedContext, unusedAttr, unusedGroupNum)
    return constraints.intersect(src, itemsToKeep)
  end
end

function make.sameAs(groupNumber)
  return function(unused_src, context, attr, groupNum)
    local groupNumber = groupNumber or groupNum
    return constraints.sameAs(context, attr, groupNumber)
  end
end

-- Removes used values and returns list for further filtering.
function make.notSameAs(groupNumber)
  return function(src, context, attr, groupNum)
    local groupNumber = groupNumber or groupNum
    return constraints.notSameAs(src, context, attr, groupNumber)
  end
end

-- Removes used values and returns random value from remainder.
function make.differentTo(groupNumber)
  return function(src, context, attr, groupNum)
    local groupNumber = groupNumber or groupNum
    return constraints.differentTo(src, context, attr, groupNumber)
  end
end


function make.unique(groupNumber)
  return function(src, context, attr, groupNum)
    local groupNumber = groupNumber or groupNum
    return constraints.unique(src, context, attr, groupNumber)
  end
end

function make.random()
  return constraints.random
end

function make.commonRandom()
  return constraints.commonRandom
end

-- With chance probabilty, range 0-1 (default 0.5), pick same value as
-- groupNumber. Otherwise pick randomly excluding that value.
function make.maybeSameAs(groupNumber, chance)
  return function(src, context, attr, groupNum)
    local groupNumber = groupNumber or groupNum
    local chance = chance or 0.5
    return constraints.maybeSameAs(src, context, attr, groupNumber, chance)
  end
end

return make
