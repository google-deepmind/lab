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

--[[ Methods to assist with generating levels for Language in Labyrinth.

Defines a 'context' object, which processes groups of attribute specifications
to generate output tables with values instantiated for a defined set of
attributes.  (Note: attribute == table key).

Specifications may be literals (number, string, boolean), functions (typically
from the language.make module) or lists which are processed elementwise.
These are typically lists of functions, each one receiving the filtered
output of the previous step as its input.

A context may contain some default starting values for each attribute, a list
of attributes to process into an output table, and optional default
specification methods to apply for each attribute when no explicit values are
passed.

Language level specific contexts can be created via createObjectContext() and
createRoomContext() which pre-populate expected attributes etc.
]]

local constraints = require 'language.constraints'
local set = require 'common.set'

local generator = {}

--[[ Returns a new context which will process the given attributes.

Keyword arguments:

*   `attributes` (table) Keys are attribute names, values are lists of possible
    choices for the value of each attribute.
*   `process` (list of strings) Names of attributes to generate values for.
    Default behaviour will process each key present in attributes.
*   `defaults` (table) Optional default spec for each attribute. See
    processSpec.
]]
function generator.createContext(kwargs)
  kwargs = kwargs or {}
  local attributes = kwargs.attributes or {}
  local process = kwargs.process or {}
  local defaults = kwargs.defaults or {}
  if next(process) == nil and next(attributes) ~= nil then
    -- default to processing all specified attributes.  May need to specify
    -- this explicitly if you care about the order of processing
    for attr, _ in pairs(attributes) do
      table.insert(process, attr)
    end
  end

  local context = {
      outputs = {},
      attributes = attributes,
      attributesToProcess = process,
      attributeDefaults = defaults,
  }

  -- Returns the possible values for attribute.
  function context.getChoicesFor(self, attribute)
    return self.attributes[attribute]
  end

  -- Sets the possible values for attribute.
  function context.setChoicesFor(self, attribute, values)
    self.attributes[attribute] = values
    return self
  end

  -- Returns a list of all known possible values for all attributes, uniquified.
  function context.vocabulary(self, kwargs)
    local attributes = kwargs and kwargs.attributes or self.attributesToProcess
    local userDict = kwargs and kwargs.userDict or {}

    -- Build a set to de-duplicate entries.
    local words = set.Set({})
    for _, attr in ipairs(attributes) do
      local values = self:getChoicesFor(attr) or {}
      set.insert(words, values)
    end
    set.insert(words, userDict)
    return set.toList(words)
  end

  -- Returns the generated output groups.
  function context.getGroups(self)
    return self.outputs
  end

  -- Return a single generated output group.
  function context.getGroup(self, groupIndex)
    return self.outputs[groupIndex]
  end

  -- Returns the number of groups which were generated.
  function context.groupCount(self)
    return #self.outputs
  end

  -- Returns a list of all values generated for groupIndex, in generation order.
  function context.getGroupAttributeValues(self, groupIndex, attr)
    local group = self:getGroup(groupIndex)
    local usedAttributes = {}
    for _, values in ipairs(group) do
      if values[attr] ~= nil then
        table.insert(usedAttributes, values[attr])
      end
    end
    return usedAttributes
  end

  -- Reset generated data, ready for another generation pass.
  function context.clearOutput(self)
    self.outputs = {}
  end

  -- Returns a list of all generated tables with the group hierarchy removed.
  function context.getFlattenedOutput(self)
    local out = {}
    for _, group in ipairs(self.outputs) do
      for _, values in ipairs(group) do
        table.insert(out, values)
      end
    end
    return out
  end

  --[[ Determine a values for attribute in groupNum using spec and candidates.

  Arguments:

  *   `spec` (various types) Used to determine a value for `attribute`.
      Used literally when number, string or boolean.
  *   If function, call with (candidates, self, attribute, groupNum) and use
      result as attribute value.
  *   If table, sequentially call this function again, passing each element in
      the list as the spec argument, using the return value as the candidates
      argument to the next call in the list..
  *   `candidates` table of possible values for the attribute, which may be used
      by the spec to select a value.
  *   `attribute` The name of the attribute to generate a value for.
  *   `groupNum` The group which this attribute belongs to.
  ]]
  function context.applyConstraints(self, spec, candidates, attribute, groupNum)
    local kind = type(spec)
    if kind == 'number' or kind == 'string' or kind == 'boolean' then
      return spec
    elseif kind == 'function' then
      return spec(candidates, self, attribute, groupNum)
    elseif kind == 'table' then
      for i = 1, #spec do
        -- Candidates must be a list, not a single item.
        -- Did you use `unique`, `differentTo`, or some such **selector** in the
        -- middle of a sequence of filters? These selectors return a single
        -- value, not a list, so they must come last.
        -- For example, use `notSameAs` instead of `differentTo` in a sequence
        -- of filters.
        assert(type(candidates) == 'table')
        candidates = self:applyConstraints(spec[i], candidates, attribute,
                                           groupNum)
      end
    else
      error('Unexpected spec of type ' .. kind)
    end
    return candidates
  end

  function context.defaultSpecForAttribute(self, attribute)
    -- If no default is specified for an attribute, use random.
    local default = self.attributeDefaults[attribute]
    return default == nil and constraints.random or default
  end

  -- Generates a value for attribute in groupNum using spec, or a suitable
  -- default if no spec is supplied for the attribute.
  function context.processSpec(self, spec, attribute, groupNum)
    -- Get an attribute specific default if the spec isn't given, being careful
    -- to preserve things that are explicitly set to false.
    if spec ~= false then
      spec = spec or self:defaultSpecForAttribute(attribute)
    end

    local initialCandidates = self:getChoicesFor(attribute)
    local candidates = self:applyConstraints(spec, initialCandidates, attribute,
                                       groupNum)
    return candidates
  end

  --[[ Given a list of tables of attribute specs, generate output tables.

  Arguments:

  *   `specs` (list of table) Each element may contain attribute/spec pairs
      which will be used to generate a value for attribute via
      applyConstraints(). This is processed sequentially so e.g. spec functions
      in the second list element (group 2) may reference attribute values
      already selected during the processing of group 1, to ensure similarity or
      difference.
  *   `groupSizes` (list of numbers, same length as specs) The number of tables
      of attribute/value pairs to generate for each spec in the corresponding
      position of the specs arguments.
  ]]
  function context.processSpecs(self, specs, groupSizes)
    assert(#specs >= 1, "Expected at least one spec")
    assert(#specs == #groupSizes, "Number of sizes should match specs")

    self:clearOutput()
    for groupNumber, groupSpec in ipairs(specs) do
      -- Create a list to hold the generated descriptions.
      local groupOutput = {}
      table.insert(self.outputs, groupOutput)
      for _ = 1, groupSizes[groupNumber] do
        local object = {}
        for _, attr in ipairs(self.attributesToProcess) do
          object[attr] = self:processSpec(groupSpec[attr], attr, groupNumber)
        end
        table.insert(groupOutput, object)
      end
    end
    return self
  end

  return context
end

return generator
