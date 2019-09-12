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
local set = require 'common.set'

local Set = set.Set

local tests = {}

local function elementCount(t)
  local size = 0
  for _, _ in pairs(t) do
    size = size + 1
  end
  return size
end

function tests.Set_shouldCreateTableWithTrueValues()
  local newSet = Set({'foo', 'bar', 'foo'})
  asserts.EQ(elementCount(newSet), 2)
  asserts.EQ(newSet['foo'], true)
  asserts.EQ(newSet['bar'], true)
end

function tests.isSame()
  asserts.EQ(set.isSame(Set{}, Set{}), true)
  asserts.EQ(set.isSame(Set{}, Set{'foo'}), false)
  asserts.EQ(set.isSame(Set{'bar'}, Set{}), false)
  asserts.EQ(set.isSame(Set{'bar'}, Set{'foo'}), false)
  asserts.EQ(set.isSame(Set{'foo', 'bar'}, Set{'foo', 'bar'}), true)
  asserts.EQ(set.isSame(Set{'foo', 'bar'}, Set{'bar', 'foo'}), true)
end

function tests.toList_shouldReturnDistinctKeys()
  local list = set.toList(Set({'foo', 'bar', 'foo'}))
  asserts.EQ(#list, 2)
  table.sort(list)
  asserts.tablesEQ(list, {'bar', 'foo'})
end

function tests.intersect_shouldReturnNewSetOfCommonItems()
  local lhsElements = {'foo', 'bar', 'baz'}
  local rhsElements = {'bar', 'baz', 'fub'}
  local lhs = Set(lhsElements)
  local rhs = Set(rhsElements)

  local intersection = set.intersect(lhs, rhs)
  asserts.EQ(elementCount(intersection), 2)
  asserts.EQ(intersection['bar'], true)
  asserts.EQ(intersection['baz'], true)

  -- Check inputs were not changed
  assert(set.isSame(lhs, Set(lhsElements)))
  assert(set.isSame(rhs, Set(rhsElements)))
end

function tests.difference_shouldReturnNewSetWithFirstMinusSecond()
  local lhsElements = {'foo', 'bar', 'baz'}
  local rhsElements = {'bar', 'baz', 'fub'}
  local lhs = Set(lhsElements)
  local rhs = Set(rhsElements)

  local difference = set.difference(lhs, rhs)
  asserts.EQ(elementCount(difference), 1)
  asserts.EQ(difference['foo'], true)

  -- Check inputs were not changed
  assert(set.isSame(lhs, Set(lhsElements)))
  assert(set.isSame(rhs, Set(rhsElements)))
end

function tests.union_shouldReturnFirstPlusSecond()
  local lhsElements = {'foo', 'bar', 'baz'}
  local rhsElements = {'bar', 'baz', 'fub'}
  local lhs = Set(lhsElements)
  local rhs = Set(rhsElements)

  local union = set.union(lhs, rhs)
  asserts.EQ(elementCount(union), 4)
  asserts.EQ(union['foo'], true)
  asserts.EQ(union['bar'], true)
  asserts.EQ(union['baz'], true)
  asserts.EQ(union['fub'], true)

  -- Check inputs were not changed
  assert(set.isSame(lhs, Set(lhsElements)))
  assert(set.isSame(rhs, Set(rhsElements)))
end

function tests.insert_shouldAddNewValuesFromList()
  local newSet = Set({'foo', 'bar', 'foo'})
  asserts.EQ(elementCount(newSet), 2)
  asserts.EQ(newSet['foo'], true)
  asserts.EQ(newSet['bar'], true)

  set.insert(newSet, {'bar', 'baz'})
  asserts.EQ(elementCount(newSet), 3)
  asserts.EQ(newSet['foo'], true)
  asserts.EQ(newSet['bar'], true)
  asserts.EQ(newSet['baz'], true)
end

return test_runner.run(tests)
