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

local property_decorator = require 'decorators.property_decorator'


local api = {
    _properties = {
        arg1 = true,
        arg2 = 19,
        arg3 = "hello",
        subArgs = {
            sub1 = 'val1',
            sub2 = 'val2',
            sub3 = 'val3',
        },
        arrArgs = {
            'arr1',
            'arr2',
            'arr3',
        }
    },
    _engineProperties = {
        foo = "bar",
    }
}

function api:nextMap()
  return 'lookat_test'
end

local function times10(valueString)
  local value = tonumber(valueString)
  return value and tostring(value * 10)
end

property_decorator.decorate(api)
property_decorator.addReadWrite('params', api._properties)
property_decorator.addReadOnly('func.times10', times10)
property_decorator.addReadOnly('engine.notReachable', api._engineProperties)

return api
