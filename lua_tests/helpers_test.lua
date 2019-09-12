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

local helpers = require 'common.helpers'
local random = require 'common.random'

local tests = {}

function tests.findFileInLuaPath_shouldFindCommonHelpers()
  -- This must be present, because we require it above:
  local path = helpers.findFileInLuaPath('common.helpers')
  assert(path)
  assert(path:find('/game_scripts/common/helpers.lua'))
end

function tests.findFileInLuaPath_shouldNotFindMadeUpFile()
  local path = helpers.findFileInLuaPath('wiggly.biggly.boo')
  asserts.EQ(path, nil)
end

local function toKeyArrays(table)
  if table == nil then return {} end
  local arr = {}
  for k, v in pairs(table) do
    arr[#arr + 1] = k
  end
  return arr
end

local function shallowEqual(tableA, tableB)
  if tableA == nil and tableB == nil then return true end

  -- Ensure tables don't reference the same object.
  if tableA == tableB then return false end

  -- Convert to arrays
  local a = toKeyArrays(tableA)
  local b = toKeyArrays(tableB)
  if #a ~= #b then return false end

  -- Check for simple equality between key values.
  for _, k in ipairs(a) do
    if tableA[k] ~= tableB[k] then return false end
  end

  return true
end

local function assertShallowCopyIsEqual(orig)
  local copy = helpers.shallowCopy(orig)
  assert(shallowEqual(orig, copy))
end

function tests.shallowCopy_nil()
  assertShallowCopyIsEqual(nil)
end

function tests.shallowCopy_emptyTable()
  assertShallowCopyIsEqual({})
end

function tests.shallowCopy_oneItemArray()
  assertShallowCopyIsEqual({'hello'})
end

function tests.shallowCopy_flatArray()
  assertShallowCopyIsEqual({'hello', 7, 'swha!?'})
end

function tests.shallowCopy_oneItemTable()
  assertShallowCopyIsEqual({myKey = 'hello'})
end

function tests.shallowCopy_flatTable()
  assertShallowCopyIsEqual({
    ['myKey'] = 'hello',
    [7] = 'there',
    ['thinking'] = 1
  })
end

function tests.shallowCopy_parentChildTable()
  local leaf = {myKey = 'hello'}
  assertShallowCopyIsEqual({root = leaf})
end

function tests.shallowCopy_complexKeyTable()
  local key = {myKey = 'hello'}
  assertShallowCopyIsEqual({[key] = 'yo'})
end

function tests.shallowCopy_bigTable()
  local key = {myKey = 'hello'}
  local leaf = {fancy = 'string'}
  local mid = {midKey = leaf}

  assertShallowCopyIsEqual({
    'try',
    'something',
    'here',
    [key] = leaf,
    anotherKey = mid
  })
end

local function deepEqual(tableA, tableB)
  if tableA == nil and tableB == nil then return true end

  -- Ensure tables don't reference the same object.
  if tableA == tableB then return false end

  -- Convert to arrays
  local a = toKeyArrays(tableA)
  local b = toKeyArrays(tableB)
  if #a ~= #b then return false end

  for _, k in ipairs(a) do
    -- Check for simple equality between keys
    local va = tableA[k]
    local vb = tableB[k]
    if type(va) ~= type(vb) then return false end
    if type(va) == 'table' then
      -- Check for deep equality between table values
      if not deepEqual(va, vb) then return false end
    else
      -- Check for simple equality between simple values
      if va ~= vb then return false end
    end
  end

  return true
end

local function assertDeepCopyIsEqual(orig)
  local copy = helpers.deepCopy(orig)
  assert(deepEqual(orig, copy))
end

function tests.deepCopy_nil()
  assertDeepCopyIsEqual(nil)
end

function tests.deepCopy_emptyTable()
  assertDeepCopyIsEqual({})
end

function tests.deepCopy_oneItemArray()
  assertDeepCopyIsEqual({'hello'})
end

function tests.deepCopy_flatArray()
  assertDeepCopyIsEqual({'hello', 7, 'swha!?'})
end

function tests.deepCopy_oneItemTable()
  assertDeepCopyIsEqual({myKey = 'hello'})
end

function tests.deepCopy_flatTable()
  assertDeepCopyIsEqual({
    ['myKey'] = 'hello',
    [7] = 'there',
    ['thinking'] = 1
  })
end

function tests.deepCopy_parentChildTable()
  local leaf = {myKey = 'hello'}
  assertDeepCopyIsEqual({root = leaf})
end

function tests.deepCopy_complexKeyTable()
  local key = {myKey = 'hello'}
  assertDeepCopyIsEqual({[key] = 'yo'})
end

function tests.deepCopy_bigTable()
  local key = {myKey = 'hello'}
  local leaf = {fancy = 'string'}
  local mid = {midKey = leaf}

  assertDeepCopyIsEqual({
    'try',
    'something',
    'here',
    [key] = leaf,
    anotherKey = mid
  })
end

function tests.fromString_boolean()
  asserts.EQ(helpers.fromString("true"), true)
  asserts.EQ(helpers.fromString("false"), false)
end

function tests.fromString_nil()
  asserts.EQ(helpers.fromString(nil), nil)
end

function tests.fromString_number()
  asserts.EQ(helpers.fromString("0"), 0)
  asserts.EQ(helpers.fromString("1"), 1)
  asserts.EQ(helpers.fromString("-1"), -1)
  asserts.EQ(helpers.fromString("2"), 2)
  asserts.EQ(helpers.fromString("0.0000000001"), 0.0000000001)
end

function tests.fromString_string()
  asserts.EQ(helpers.fromString(""), "")
  asserts.EQ(helpers.fromString("random string"), "random string")
end

function tests.fromString_table()
  local random_table = {random_name = 1}
  asserts.EQ(helpers.fromString(random_table), random_table)
end

function tests.pathJoin()
  asserts.EQ(helpers.pathJoin('base', 'path'), 'base/path')
  asserts.EQ(helpers.pathJoin('base/', 'path'), 'base/path')
  asserts.EQ(helpers.pathJoin('base', '/path'), '/path')
  asserts.EQ(helpers.pathJoin('base/', '/path'), '/path')
end

function tests.pairsByKeys()
  local keys = {'a', 'b', 'c', 'd'}
  local vals = {'10', '20', '80', '40'}
  local gen = random:shuffledIndexGenerator(#keys)
  local dict = {}
  for i = 1, #keys do
    local id = gen()
    dict[keys[id]] = vals[id]
  end
  local i = 0
  for k, v in helpers.pairsByKeys(dict) do
    i = i + 1
    asserts.EQ(k, keys[i])
    asserts.EQ(v, vals[i])
  end

  asserts.EQ(i, #keys)
end

function tests.pairsByKeysReversed()
  local keys = {'a', 'b', 'c', 'd'}
  local vals = {'10', '20', '80', '40'}
  local gen = random:shuffledIndexGenerator(#keys)
  local dict = {}
  for i = 1, #keys do
    local id = gen()
    dict[keys[id]] = vals[id]
  end
  local i = #keys
  for k, v in helpers.pairsByKeys(dict, function(a, b) return a > b end) do
    asserts.EQ(k, keys[i])
    asserts.EQ(v, vals[i])
    i = i - 1
  end
  asserts.EQ(i, 0)
end

return test_runner.run(tests)
