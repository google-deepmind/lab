--[[ Copyright (C) 2017-2019 Google Inc.

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

local asserts = require 'testing.asserts'
local test_runner = require 'testing.test_runner'
local texter = require 'language.texter'

local tests = {}

function tests.decode_canRetrieveTopLevelNamedTableKey()
  local tbl = {topLevelKey = 6, otherTopLevelKey = 7}
  asserts.EQ(texter.decode('topLevelKey', tbl), 6)
  asserts.EQ(texter.decode('otherTopLevelKey', tbl), 7)
end

function tests.decode_canRetrieveTopLevelArrayKey()
  local list = {'one', 'two', 'three'}
  asserts.EQ(texter.decode('1', list), 'one')
  asserts.EQ(texter.decode('2', list), 'two')
  asserts.EQ(texter.decode('3', list), 'three')
end

function tests.decode_canRetrieveNestedLists()
  local list = {
    {'list 1 element 1', 'list 1 element 2'},
    {'list 2 element 1'}
  }
  asserts.EQ(texter.decode('1.2', list), 'list 1 element 2')
  asserts.EQ(texter.decode('2.1', list), 'list 2 element 1')
end

function tests.decode_canRetrieveMixedNestedValues()
  local tbl = {
    pets = {
      {name = {first = 'Ajax', last = 'The Cat'}},
      {name = {first = 'Fido', last = 'The Dog'}}
    }
  }
  asserts.EQ(texter.decode('pets.1.name.first', tbl), 'Ajax')
  asserts.EQ(texter.decode('pets.2.name.last', tbl), 'The Dog')
end

function tests.format_decodesTextInSquareBrackets()
  local tbl = {
    count = 2,
    pets = {
      {name = {first = 'Ajax', last = 'The Cat'}},
      {name = {first = 'Fido', last = 'The Dog'}}
    }
  }
  asserts.EQ(texter.format('[count]', tbl), '2')
  asserts.EQ(texter.format('[pets.2.name.last]', tbl), 'The Dog')

  asserts.EQ(
    texter.format('[pets.1.name.first], meet [pets.2.name.first].', tbl),
    'Ajax, meet Fido.')
end

return test_runner.run(tests)
