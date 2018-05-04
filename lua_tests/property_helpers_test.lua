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

local property_helpers = require 'common.property_helpers'
local asserts = require 'testing.asserts'
local test_runner = require 'testing.test_runner'

local tests = {}
local RESULT = property_helpers.RESULT

function tests.headTailSplit()
  local headTailSplit = property_helpers.headTailSplit
  asserts.tablesEQ({headTailSplit('aaa.bbb.ccc')}, {'aaa', 'bbb.ccc'})
  asserts.tablesEQ({headTailSplit('aaa')}, {'aaa', nil})
  asserts.tablesEQ({headTailSplit('10.12')}, {10, '12'})
end

function tests.readProperty()
  local readProperty = property_helpers.readProperty
  asserts.EQ(readProperty({abc = {a = 1, b = 2, c = 3}}, 'abc.b'), '2')
  asserts.EQ(readProperty({abc = {a = 1, b = 2, c = 3}}, 'abc.d'),
             RESULT.NOT_FOUND)
  asserts.EQ(readProperty({abc = {'a', 'b', 'c'}}, 'abc.2'), 'b')
  asserts.EQ(readProperty({abc = {'a', 'b', 'c'}}, 'abc.0'), RESULT.NOT_FOUND)
  local params = {abc = function(listKeys) return listKeys .. listKeys end}
  asserts.EQ(readProperty(params, 'abc.hello'), 'hellohello')
end

function tests.writeProperty()
  local writeProperty = property_helpers.writeProperty
  local params = {abc = {a = '1', b = 2, c = true}}
  asserts.EQ(writeProperty(params, 'abc.a', '11'), RESULT.SUCCESS)
  asserts.tablesEQ(params, {abc = {a = '11', b = 2, c = true}})

  asserts.EQ(writeProperty(params, 'abc.b', '22'), RESULT.SUCCESS)
  asserts.tablesEQ(params, {abc = {a = '11', b = 22, c = true}})

  asserts.EQ(writeProperty(params, 'abc.c', 'false'), RESULT.SUCCESS)
  asserts.tablesEQ(params, {abc = {a = '11', b = 22, c = false}})
  local params2 = {}
  params2.abc = {
      add = function(key, value)
        params2.abc[key] = value
        return RESULT.SUCCESS
      end
  }
  asserts.EQ(writeProperty(params2, 'abc.add.newKey', 'newValue'),
             RESULT.SUCCESS)
  asserts.EQ(params2.abc.newKey, 'newValue')
end

function tests.addProperty()
  local addProperty = property_helpers.addProperty
  local params = {}
  asserts.EQ(addProperty(params, 'abc.b', 2), true)
  asserts.tablesEQ(params, {abc = {b = 2}})
  asserts.EQ(addProperty(params, 'abc.a', '1'), true)
  asserts.tablesEQ(params, {abc = {a = '1', b = 2}})
  asserts.EQ(addProperty(params, 'abc.a', '11'), false)
  asserts.tablesEQ(params, {abc = {a = '1', b = 2}})
  asserts.EQ(addProperty(params, 'abc.c', {'c1', 'c2', 'c3'}), true)
  asserts.tablesEQ(params, {abc = {a = '1', b = 2, c = {'c1', 'c2', 'c3'}}})
end

function tests.removeProperty()
  local removeProperty = property_helpers.removeProperty
  local params = {abc = {a = '1', b = 2, c = {'c1', 'c2', 'c3'}}}
  asserts.EQ(removeProperty(params, 'abc.b'), true)
  asserts.tablesEQ(params, {abc = {a = '1', c = {'c1', 'c2', 'c3'}}})
  asserts.EQ(removeProperty(params, 'abc.a'), true)
  asserts.tablesEQ(params, {abc = {c = {'c1', 'c2', 'c3'}}})
  asserts.EQ(removeProperty(params, 'abc.c.2'), true)
  asserts.tablesEQ(params, {abc = {c = {[1] = 'c1', [3] = 'c3'}}})
  asserts.EQ(removeProperty(params, 'abc'), true)
  asserts.tablesEQ(params, {})
end

return test_runner.run(tests)
