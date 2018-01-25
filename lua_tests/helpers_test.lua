local asserts = require 'testing.asserts'
local test_runner = require 'testing.test_runner'

local helpers = require 'common.helpers'

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

local function toKeyValueArrays(table)
  if table == nil then return {} end
  local arr = {}
  for k, v in pairs(table) do
    arr[#arr + 1] = {k = k, v = v}
  end
  return arr
end

local function shallowEqual(tableA, tableB)
  if tableA == nil and tableB == nil then return true end

  -- Ensure tables don't reference the same object.
  if tableA == tableB then return false end

  -- Convert to arrays
  local a = toKeyValueArrays(tableA)
  local b = toKeyValueArrays(tableB)
  if #a ~= #b then return false end

  -- Check for simple equality between key values.
  for i = 1, #a do
    if a[i].k ~= b[i].k then return false end
    if a[i].v ~= b[i].v then return false end
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
  local a = toKeyValueArrays(tableA)
  local b = toKeyValueArrays(tableB)
  if #a ~= #b then return false end

  for i = 1, #a do
    -- Check for simple equality between keys
    if a[i].k ~= b[i].k then return false end
    if type(a[i].v) ~= type(b[i].v) then return false end
    if type(a[i].v) == 'table' then
      -- Check for deep equality between table values
      if not deepEqual(a[i].v, b[i].v) then return false end
    else
      -- Check for simple equality between simple values
      if a[i].v ~= b[i].v then return false end
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

return test_runner.run(tests)
