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

-- Common utilities.
local io = require 'io'

local SECONDS_IN_MINUTE = 60
local SECONDS_IN_HOUR = SECONDS_IN_MINUTE * 60

local helpers = {}

-- Returns an array of strings split according to single character separator.
-- Skips empty fields.
function helpers.split(str, sep)
  local words = {}
  for word in string.gmatch(str, '([^' .. sep .. ']+)') do
    words[#words + 1] = word
  end
  return words
end

-- Converts a space-delimited string of numerical values to a table of numbers.
-- e.g. "1.000 2.00 3.00" becomes {1, 2, 3}
function helpers.spawnVarToNumberTable(spaceSeperatedString)
  local result = {}
  for v in string.gmatch(spaceSeperatedString, "%S+") do
    local index = #result + 1
    result[index] = tonumber(v)
  end
  return result
end

--[[ Joins a path with a base directory.

*   pathJoin('base', 'path') => 'base/path'
*   pathJoin('base/', 'path') => 'base/path'
*   pathJoin('base', '/path') => '/path'
*   pathJoin('base/', '/path') => '/path'
]]
function helpers.pathJoin(base, path)
  if path:sub(1, 1) == '/' then
    return path
  elseif base:sub(#base) == '/' then
    return base .. path
  else
    return base .. '/' .. path
  end
end

function helpers.dirname(str)
  return str:match(".-/.-") and str:gsub("(.*/)(.*)", "%1") or ''
end

-- Returns whether a file exists by opening it for read.
function helpers.fileExists(file)
  local f = io.open(file, "r")
  if f ~= nil then
    io.close(f)
    return true
  else
    return false
  end
end

-- Given the name of a Lua module from game_scripts, e.g. 'common.helpers',
-- returns the full path to the source Lua file on disk, or nil.
function helpers.findFileInLuaPath(moduleName)
  local moduleName = moduleName:gsub('[.]', '/')
  for element in package.path:gmatch('[^;]+') do
    if element:find('/game_scripts/') then
      local path = element:gsub('[?]', moduleName)
      if helpers.fileExists(path) then
        return path
      end
    end
  end
  return nil
end

--[[ Returns a shallow copy of a table.

That is, copies the first level of key value pairs, but does not recurse down
into the table.
]]
function helpers.shallowCopy(input)
  if input == nil then return nil end
  assert(type(input) == 'table')
  local output = {}
  for k, v in pairs(input) do
    output[k] = v
  end
  setmetatable(output, getmetatable(input))
  return output
end

--[[ Returns a deep copy of a table.

Performs a deep copy of the key-value pairs of a table, recursing for each
value, depth-first, up to the specified depth, or indefinitely if that parameter
is nil.
]]
function helpers.deepCopy(input, depth)
  if type(input) == 'table' then
    if depth == nil or depth > 0 then
      local output = {}
      local below = depth ~= nil and depth - 1 or nil
      for k, v in pairs(input) do
        output[k] = helpers.deepCopy(v, below)
      end
      setmetatable(output, helpers.deepCopy(getmetatable(input), below))
      return output
    end
  end
  return input
end

-- Naive recursive pretty-printer.
-- Prints the table hierarchically. Assumes all the keys are simple values.
function helpers.tostring(input, spacing, limit)
  limit = limit or 5
  if limit < 0 then
    return ''
  end
  spacing = spacing or ''
  if type(input) == 'table' then
    local res = '{\n'
    for k, v in pairs(input) do
      if type(k) ~= 'string' or string.sub(k, 1, 2) ~= '__' then
        res = res .. spacing .. '  [\'' .. tostring(k) .. '\'] = ' ..
          helpers.tostring(v, spacing .. '  ', limit - 1)
      end
    end
    return res .. spacing .. '}\n'
  else
    return tostring(input) .. '\n'
  end
end

--[[ Convert a number of seconds into a string of the format hhh:mm:ss, where

*   'hhh' represents the hours,
*   'mm' the minutes, and
*   'ss' the seconds.

Omits 'hhh:' if hours is 0, and also 'mm:' if hours and minutes are zero.
Rounds-up timeSeconds to the closest second.
--]]
function helpers.secondsToTimeString(timeSeconds)
  local s = math.ceil(timeSeconds)
  if s < SECONDS_IN_MINUTE then
    return string.format('%d', s)
  elseif s < SECONDS_IN_HOUR then
    return string.format('%d:%.2d',
                         s / SECONDS_IN_MINUTE, s % SECONDS_IN_MINUTE)
  else
    return string.format('%d:%.2d:%.2d',
                         s / SECONDS_IN_HOUR,
                         s / SECONDS_IN_MINUTE % SECONDS_IN_MINUTE,
                         s % SECONDS_IN_MINUTE)
  end
end

--[[ Converts from a string to its most likely type.

Numbers are returned as numbers.
"true" and "false" are returned as booleans.
Everything else is unchanged.
]]
function helpers.fromString(input)
  local number = tonumber(input)
  if number ~= nil then
    return number
  end
  if input == "true" then
    return true
  end
  if input == "false" then
    return false
  end
  return input
end

--[[ Iterates dictionary in sorted key order.

Arguments:

*   'dict' (table) Table to be iterated.
*   'sorter' (function) Optional sorting function.

Raises an error if keys are not comparable.
]]
function helpers.pairsByKeys(dict, sorter)
  local a = {}
  for n in pairs(dict) do a[#a + 1] = n end
  table.sort(a, sorter)
  local i = 0
  return function()
    i = i + 1
    local k = a[i]
    if k then
      return k, dict[k]
    else
      return nil
    end
  end
end

return helpers
