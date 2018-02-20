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

--[[ Defines filters and constraints to operate on lists of candidate choices.

The initial use case for these is to help with generating levels for the
Language in Labyrinth OKR which requires us to construct levels that have
e.g. only one red balloon in them.

They are typically not used directly, but wrapped via the functor generation
methods in lang_make, and processed in groups alongside a context from
lang_generator.

To be compatible with the functions here, the context needs only to support one
method: getGroup(groupNum) which should return an array of tables.  These
tables represent the attribute-value pairs already chosen for members of this
group.
]]

local random = require 'common.random'
local set = require 'common.set'

local constraints = {}

--[[ Returns a list containing subtable[attribute] for each subtable in groups.

Arguments:

*   `group` is a list of tables.
*   `attribute` is a string.
]]
local function getGroupAttributeValues(group, attribute)
  local usedAttributes = {}
  for _, values in ipairs(group) do
    if values[attribute] ~= nil then
      usedAttributes[#usedAttributes + 1] = values[attribute]
    end
  end
  return usedAttributes
end

-- Returns a random element from input sequence or nil if seq is empty.
function constraints.random(seq)
  return random:choice(seq)
end

--[[ Returns the first element of seq or nil if seq is empty.

Arguments:

*   `seq` is a list.
]]
function constraints.first(seq)
  if #seq > 0 then
    return seq[1]
  else
    return nil
  end
end

--[[ Removes itemsToExclude from seq.

Arguments:

*   `seq` is a list.
*   `itemsToExclude` may be a list or a single value.
]]
function constraints.remove(seq, itemsToExclude)
  if type(itemsToExclude) ~= 'table' then
    itemsToExclude = {itemsToExclude}
  end
  local exclusionSet = set.Set(itemsToExclude)

  local filteredSequence = {}
  for i = 1, #seq do
    if not exclusionSet[seq[i]] then
      filteredSequence[#filteredSequence + 1] = seq[i]
    end
  end
  return filteredSequence
end

--[[ Returns the intersection of itemsToKeep and seq, preserving the order of
seq.

Arguments:

*   `seq` is a list.
*   `itemsToKeep` may be a list or a single value.
]]
function constraints.intersect(seq, itemsToKeep)
  if type(itemsToKeep) ~= 'table' then
    itemsToKeep = {itemsToKeep}
  end
  local inclusionSet = set.Set(itemsToKeep)

  local filteredSequence = {}
  for i = 1, #seq do
    if inclusionSet[seq[i]] then
      filteredSequence[#filteredSequence + 1] = seq[i]
    end
  end
  return filteredSequence
end

--[[ Returns copy of the given sequence.

Arguments:

*   `seq` is a list.
]]
function constraints.copy(seq)
  if type(seq) ~= 'table' then
    seq = {seq}
  end

  local newSequence = {}
  for i = 1, #seq do
    newSequence[i] = seq[i]
  end
  return newSequence
end

--[[ Returns the value of attribute held by group in context.

Arguments:

*   `context` is an object implementing the Context API.
*   `attribute` is a string.
*   `group` is a number.
]]
function constraints.sameAs(context, attribute, group)
  -- TODO: Since a group can have multiple objects, which one do we take the
  -- same value as?  For now assume first object
  local requiredValue = context:getGroup(group)[1][attribute]
  return requiredValue
end

--[[ Returns list of values in seq which are not already used by group or groups
in context.

Arguments:

*   `seq` is a list.
*   `context` is an object implementing the Context API.
*   `attribute` is a string.
*   `group` is a number or list of numbers.
]]
function constraints.notSameAs(seq, context, attribute, groups)
  if type(groups) ~= 'table' then groups = {groups} end
  for _, group in ipairs(groups) do
    -- Remove all values for attribute in group:
    for _, used in ipairs(context:getGroup(group)) do
      seq = constraints.remove(seq, used[attribute])
    end
  end
  return seq
end

--[[ Returns a random value from seq which excludes those already used by group
or groups in context.

Arguments:

*   `seq` is a list.
*   `context` is an object implementing the Context API.
*   `attribute` is a string.
*   `group` is a number or list of numbers.
]]
function constraints.differentTo(seq, context, attribute, group)
  return constraints.random(
    constraints.notSameAs(seq, context, attribute, group))
end

--[[ Pick same value for attribute as group with chance probability. Otherwise
pick randomly from the choices in seq excluding the target value.

Arguments:

*   `seq` is a list.
*   `context` is an object implementing the Context API.
*   `attribute` is a string.
*   `group` is a number.
*   `chance` is a probability range 0-1.
]]
function constraints.maybeSameAs(seq, context, attribute, group, chance)
  if random:uniformReal(0, 1) < chance then
    return constraints.sameAs(context, attribute, group)
  else
    seq = constraints.notSameAs(seq, context, attribute, group)
    return constraints.random(seq)
  end
end

--[[ Selects a random-but-unique-within-this-group value from seq for attribute.

The existing attribute values from the group are removed from seq to determine
the subset of available unique options.

Arguments:

*   `seq` is a list.
*   `context` is an object implementing the Context API.
*   `attribute` is a string.
*   `group` is a number.
]]
function constraints.unique(seq, context, attribute, group)
  local usedValues = getGroupAttributeValues(context:getGroup(group), attribute)
  return constraints.random(constraints.remove(seq, usedValues))
end

--[[ Selects a common value for the group.

If there are no values in the group yet, pick at random from seq.
Otherwise return the value of the first element.

Arguments:

*   `seq` is a list of possible values.
*   `context` is an object implementing the Context API.
*   `attribute` is a string.
*   `group` is a number.
]]
function constraints.commonRandom(seq, context, attribute, group)
  local usedValues = getGroupAttributeValues(context:getGroup(group), attribute)
  if #usedValues > 0 then
    return usedValues[1]
  else
    return constraints.random(seq)
  end
end

return constraints
