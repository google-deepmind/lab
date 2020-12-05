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

local log = require 'common.log'
local property_helpers = require 'common.property_helpers'

--[[ Property decorator

Provides a mechanism for exposing properties to the EnvCApi.

```
local property_decorator = require 'decorators.property_decorator'
local api = {
    _myParams = {
        param0 = 0,
        param1 = '1',
    }
}

function api:init(...)
  property_decorator.addReadWrite('myParams', self._myParams)
end

-- Not necessary if using settings_override decorator.
property_decorator.decorate(api)

return api
```

This will expose properties:

*   'myParams.param0' = '0'
*   'myParams.param1' = '1'
]]

local RESULT = property_helpers.RESULT
local _readWrite = {}
local _readOnly = {}
local _writeOnly = {}

local function addReadWrite(keyList, property)
  property_helpers.addProperty(_readWrite, keyList, property)
end

local function addReadOnly(keyList, property)
  property_helpers.addProperty(_readOnly, keyList, property)
end

local function addWriteOnly(keyList, property)
  property_helpers.addProperty(_writeOnly, keyList, property)
end

local function removeReadWrite(keyList)
  property_helpers.removeProperty(_readWrite, keyList)
end

local function removeReadOnly(keyList)
  property_helpers.removeProperty(_readOnly, keyList)
end

local function removeWriteOnly(keyList)
  property_helpers.removeProperty(_writeOnly, keyList)
end

local function decorate(api)
  assert(api.writeProperty == nil, 'Already has writeProperty')
  assert(api.readProperty == nil, 'Already has readProperty')
  assert(api.listProperty == nil, 'Already has listProperty')

  function api:writeProperty(keyList, value)
    local resRW = property_helpers.writeProperty(_readWrite, keyList, value)
    if resRW == RESULT.SUCCESS then
      return resRW
    end
    local resW0 = property_helpers.writeProperty(_writeOnly, keyList, value)
    if resW0 == RESULT.SUCCESS then
      return resW0
    end

    -- If result is not a success return the most specific error message.
    return math.max(resRW or RESULT.NOT_FOUND,
                    resW0 or RESULT.NOT_FOUND)
  end

  function api:readProperty(keyList)
    local resRW = property_helpers.readProperty(_readWrite, keyList)
    if resRW ~= nil and type(resRW) ~= 'number' then
      return tostring(resRW)
    end
    local resRO = property_helpers.readProperty(_readOnly, keyList)
    if resRO ~= nil and type(resRO) ~= 'number' then
      return tostring(resRO)
    end

    -- If result is not a success return the most specific error message.
    return math.max(resRW or RESULT.NOT_FOUND,
                    resRO or RESULT.NOT_FOUND)
  end

  function api:listProperty(keyList, callback)
    local localReadWrite = _readWrite
    local localReadOnly = _readOnly
    local localWriteOnly = _writeOnly
    local prefix = ''
    if keyList == '' then
      keyList = nil
    end
    -- Avoid calling any functions for listing.
    while keyList do
      local head, tail = property_helpers.headTailSplit(keyList)
      prefix = prefix .. head .. '.'
      keyList = tail
      localReadWrite = localReadWrite and
                       type(localReadWrite) == 'table' and
                       localReadWrite[head]
      localReadOnly = localReadOnly and
                       type(localReadOnly) == 'table' and
                       localReadOnly[head]
      localWriteOnly = localWriteOnly and
                       type(localWriteOnly) == 'table' and
                       localWriteOnly[head]
    end
    if localReadWrite == nil and
       localReadOnly == nil and
       localWriteOnly == nil then
       return nil
    end
    local success = false
    if localReadWrite and type(localReadWrite) == 'table' then
      success = true
      for k, v in pairs(localReadWrite) do
        callback(prefix .. k, type(v) == 'table' and 'l' or 'rw')
      end
    end

    if localReadOnly and type(localReadOnly) == 'table' then
      success = true
      for k, v in pairs(localReadOnly) do
        callback(prefix .. k, type(v) == 'table' and 'l' or 'r')
      end
    end

    if localWriteOnly and type(localWriteOnly) == 'table' then
      success = true
      for k, v in pairs(localWriteOnly) do
        callback(prefix .. k, type(v) == 'table' and 'l' or 'w')
      end
    end
    return success
  end
end

return {
    RESULT = RESULT,
    decorate = decorate,
    addReadWrite = addReadWrite,
    addReadOnly = addReadOnly,
    addWriteOnly = addWriteOnly,
    removeReadWrite = removeReadWrite,
    removeReadOnly = removeReadOnly,
    removeWriteOnly = removeWriteOnly,
}
