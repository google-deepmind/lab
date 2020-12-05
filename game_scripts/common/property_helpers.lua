--[[ Copyright (C) 2018-2019 Google Inc.

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

local helpers = require 'common.helpers'
local log = require 'common.log'

-- Must match EnvCApi_PropertyResult from env_c_api.h
local RESULT = {
    SUCCESS = 0,
    NOT_FOUND = 1,
    PERMISSION_DENIED = 2,
    INVALID_ARGUMENT = 3,
}

--[[ Returns the head and tail of keyList.

Examples:

> print(headTailSplit('aaa.bbb.ccc'))
'aaa'    'bbb.ccc'

> print(headTailSplit('aaa'))
'aaa'    nil

> print(headTailSplit('10.12'))
10    '12'

Arguments:

*   keyList - String containing dot (.) separated list of keys.

Returns:

*   Arg1 - First element of keyList (converted to a number if possible).
*   Arg2 - Tail of keyList still dot(.) separated. Returns nil if empty.
]]
local function headTailSplit(keyList)
  local dotIdx = keyList:find('[.]')
  if dotIdx then
    local key = keyList:sub(1, dotIdx - 1)
    key = tonumber(key) or key
    return key, keyList:sub(dotIdx + 1)
  end
  return helpers.fromString(keyList), nil
end

--[[ Returns value of properties in keyList context.

See implmentation for details.

Arguments:

*   properties - table|function|string
*   keyList - String containing dot (.) separated list of keys.

Returns:

*   String if lookup is successful, RESULT.PERMISSION_DENIED if not not
    readable, otherwise RESULT.NOT_FOUND if not findable. If nil
    is returned it is equivalent to returning RESULT.NOT_FOUND.

Examples:

```Lua
-- Look up in tables.
properties = {abc = {a = 1, b = 2, c = 3}}
assert(readProperty(properties, 'abc.b') == '2')
assert(readProperty(properties, 'abc.d') == RESULT.NOT_FOUND)

-- Look up in arrays.
properties = {abc = {'a', 'b', 'c'}}
assert(readProperty(properties, 'abc.2') == 'b')
assert(readProperty(properties, 'abc.0') == RESULT.NOT_FOUND)

-- Look up in functions.
properties = {abc = function(listKeys) return listKeys end}
assert(readProperty(properties, 'abc.hello_world'), 'hello_world')
```
]]
local function readProperty(properties, keyList)
  if properties == nil then
    return RESULT.NOT_FOUND
  end
  if keyList == nil or keyList == '' then
    if type(properties) == 'table' then
      return RESULT.PERMISSION_DENIED
    end
    return tostring(properties)
  end
  local propType = type(properties)
  if propType == 'table' then
    local head, tail = headTailSplit(keyList)
    return readProperty(properties[head], tail)
  elseif propType == 'function' then
    return properties(keyList)
  else
    return RESULT.PERMISSION_DENIED
  end
end

--[[ Write value to writable in keyList context.

See implmentation for details.

Arguments:

*   properties - table|function
*   keyList - String containing dot (.) separated list of keys.

Returns:

*   RESULT.SUCCESS if write is successful, RESULT.INVALID_ARGUMENT if value is
    the wrong type, RESULT.PERMISSION_DENIED if the object is not writable or
    RESULT.NOT_FOUND if the property is not found. If nil is returned it is
    equivalent to returning RESULT.NOT_FOUND.

Examples:
```Lua
-- Write number.
params = {abc = {a = 1, b = 2, c = 3}}
assert(writeProperty(params, 'abc.b', 22), RESULT.SUCCESS)
assert(writeProperty(params, 'abc.b', 'blah'), RESULT.INVALID_ARGUMENT)
assert(writeProperty(params, 'abc.d', 'blah'), RESULT.NOT_FOUND)

-- Write to array.
params = {abc = {'a', 'b', 'c'}}
assert(writeProperty(params, 'abc.2', 'bb'), RESULT.SUCCESS)
assert(writeProperty(params, 'abc.0'), RESULT.NOT_FOUND)
assert(writeProperty(params, 'abc', ''), RESULT.PERMISSION_DENIED)

-- Write to function
local params2 = {}
params2.abc = {
    add = function(key, value)
      params2.abc[key] = value
      return RESULT.SUCCESS
    end
}
assert(writeProperty(params2, 'abc.add.newKey', 'newValue'), RESULT.SUCCESS)
assert(params2.abc.newKey == 'newValue')
```
]]
local function writeProperty(properties, keyList, value)
  if properties == nil then
    return RESULT.NOT_FOUND
  end
  local t = type(properties)
  if t == 'table' then
    if keyList == nil then
      return RESULT.PERMISSION_DENIED
    end
    local head, tail = headTailSplit(keyList)
    if tail == nil then
      local val = properties[head]
      if val == nil then
        return RESULT.NOT_FOUND
      end
      local tval = type(val)
      if tval == 'function' then
        return pcall(val, value) and RESULT.SUCCESS or RESULT.INVALID_ARGUMENT
      elseif tval == 'string' then
        properties[head] = value
        return RESULT.SUCCESS
      elseif tval == 'number' then
        value = tonumber(value)
        if value ~= nil then
          properties[head] = value
          return RESULT.SUCCESS
        else
          return RESULT.INVALID_ARGUMENT
        end
      elseif tval == 'boolean' then
        if value == 'true' then
          properties[head] = true
          return RESULT.SUCCESS
        elseif value == 'false' then
          properties[head] = false
          return RESULT.SUCCESS
        else
          return RESULT.INVALID_ARGUMENT
        end
      else
        return RESULT.PERMISSION_DENIED
      end
    end
    return writeProperty(properties[head], tail, value)
  elseif t == 'function' then
    return properties(keyList, value) or RESULT.INVALID_ARGUMENT
  else
    return RESULT.PERMISSION_DENIED
  end
end

--[[ Add property at location keyList with value value.

If keyList is split into key1, key2 ... keyN then
properties[key1][key2]...[keyN] is assinged to value.

If properties[key1][key2]...[keyN] already exists or any part is not a table
then the operation is not successful and the value is not assigned. Use
writeProperty instead. Returns whether the operation was successful.
]]
local function addPropertyInternal(properties, keyList, value)
  if type(properties) ~= 'table' then
    return false
  end
  local head, tail = headTailSplit(keyList)
  local property = properties[head]
  if tail == nil then
    if property == nil then
      properties[head] = value
      return true
    else
      return false
    end
  else
    if property == nil then
      property = {}
      properties[head] = property
    end
    return addPropertyInternal(property, tail, value)
  end
end

local function addProperty(properties, keyList, value)
  local head = headTailSplit(keyList)
  if head == 'engine' then
    log.warn('"engine" is reserved and will not be accessible via the ' ..
             'environment API. keyList: ' .. keyList)
  end
  return addPropertyInternal(properties, keyList, value)
end

local function removeProperty(properties, keyList)
  if type(properties) ~= 'table' then
    return false
  end
  local head, tail = headTailSplit(keyList)
  local property = properties[head]
  if tail == nil then
    if property ~= nil then
      properties[head] = nil
      return true
    else
      return false
    end
  else
    if property == nil then
      return false
    end
    return removeProperty(property, tail)
  end
end

return {
    RESULT = RESULT,
    addProperty = addProperty,
    removeProperty = removeProperty,
    writeProperty = writeProperty,
    readProperty = readProperty,
    headTailSplit = headTailSplit,
}
