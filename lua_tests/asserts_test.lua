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

local tests = {}

local function shouldFail(fn)
  local status, out = pcall(fn)
  if status then
    error("Expected assert to fire but it did not.", 2)
  end
end

local function shouldFailWithMessage(fn, message)
  local status, out = pcall(fn)
  if status then
    error("Expected assert to fire but it did not.", 2)
  end
  if type(message) ~= 'table' then message = {message} end
  for _, expected in ipairs(message) do
    if not out:find(expected) then
      error('Expected "' .. expected .. '" to appear in "' .. out .. '"', 2)
    end
  end
end


function tests.EQ_expectSuccess()
  asserts.EQ(7, 7)
  asserts.EQ(7, 7, 'message')
  asserts.EQ(true, true)
  asserts.EQ(false, false)
  asserts.EQ("a string", "a string")
  asserts.EQ(nil, nil)
  local tbl = {2, 4, 6}
  asserts.EQ(tbl, tbl)
end

function tests.EQ_expectFailure()
  shouldFail(function () asserts.EQ(7, 8) end)
  shouldFail(function () asserts.EQ(true, false) end)
  shouldFail(function () asserts.EQ(false, true) end)
  shouldFail(function () asserts.EQ("a string", "a different string") end)
  shouldFail(function () asserts.EQ(nil, {}) end)

  local tbl = {2, 4, 6}
  shouldFail(function () asserts.EQ({2, 4, 6}, {2, 4, 6}) end)

  shouldFail(function () asserts.EQ(7, true) end)
  shouldFail(function () asserts.EQ(true, 7) end)
  shouldFail(function () asserts.EQ(7, false) end)
  shouldFail(function () asserts.EQ(false, 7) end)
  shouldFail(function () asserts.EQ(7, "a string") end)
  shouldFail(function () asserts.EQ("a string", 7) end)
  shouldFail(function () asserts.EQ(7, {2, 4, 6}) end)
  shouldFail(function () asserts.EQ({2, 4, 6}, 7) end)

  shouldFail(function () asserts.EQ(true, "a string") end)
  shouldFail(function () asserts.EQ("a string", true) end)
  shouldFail(function () asserts.EQ(true, {2, 4, 6}) end)
  shouldFail(function () asserts.EQ({2, 4, 6}, true) end)

  shouldFail(function () asserts.EQ(false, "a string") end)
  shouldFail(function () asserts.EQ("a string", false) end)
  shouldFail(function () asserts.EQ(false, {2, 4, 6}) end)
  shouldFail(function () asserts.EQ({2, 4, 6}, false) end)

  shouldFail(function () asserts.EQ("a string", {2, 4, 6}) end)
  shouldFail(function () asserts.EQ({2, 4, 6}, "a string") end)


  shouldFailWithMessage(function () asserts.EQ(7, 8, 'monkey') end,
    {'7', '8', 'monkey'})
end


function tests.NE_expectSuccess()
  asserts.NE(7, 8)
  asserts.NE(true, false)
  asserts.NE(false, true)
  asserts.NE("a string", "a different string")
  asserts.NE(nil, {})
  asserts.NE({2, 4, 6}, {2, 4, 6})

  asserts.NE(7, true)
  asserts.NE(true, 7)
  asserts.NE(7, false)
  asserts.NE(false, 7)
  asserts.NE(7, "a string")
  asserts.NE("a string", 7)
  asserts.NE(7, {2, 4, 6})
  asserts.NE({2, 4, 6}, 7)

  asserts.NE(true, "a string")
  asserts.NE("a string", true)
  asserts.NE(true, {2, 4, 6})
  asserts.NE({2, 4, 6}, true)

  asserts.NE(false, "a string")
  asserts.NE("a string", false)
  asserts.NE(false, {2, 4, 6})
  asserts.NE({2, 4, 6}, false)

  asserts.NE("a string", {2, 4, 6})
  asserts.NE({2, 4, 6}, "a string")
end

function tests.NE_expectFailure()
  shouldFail(function () asserts.NE(7, 7) end)
  shouldFail(function () asserts.NE(7, 7, 'message') end)
  shouldFail(function () asserts.NE(true, true) end)
  shouldFail(function () asserts.NE(false, false) end)
  shouldFail(function () asserts.NE("a string", "a string") end)
  shouldFail(function () asserts.NE(nil, nil) end)
  local tbl = {2, 4, 6}
  shouldFail(function () asserts.NE(tbl, tbl) end)

  shouldFailWithMessage(function () asserts.NE("monkey", "monkey") end,
    'Expected values to differ: monkey')

  shouldFailWithMessage(function () asserts.NE("monkey", "monkey", "magic") end,
    'Expected values to differ: monkey magic')
end


function tests.GT_expectSuccess()
  asserts.GT(1, 0)
  asserts.GT(0, -3)
  asserts.GT(3.14, 3)
end

function tests.GT_expectFailure()
  shouldFail(function () asserts.GT(0, 1) end)
  shouldFail(function () asserts.GT(-3, 0) end)
  shouldFail(function () asserts.GT(3, 3) end)
  shouldFail(function () asserts.GT(3, 3.14) end)

  shouldFailWithMessage(function () asserts.GT(0, 1) end,
    'Expected: 0 > 1')
  shouldFailWithMessage(function () asserts.GT(0, 1, 'monkey') end,
    'Expected: 0 > 1 monkey')
end

function tests.GE_expectSuccess()
  asserts.GE(0, 0)
  asserts.GE(1, 0)
  asserts.GE(0, -3)
  asserts.GE(3, 3)
  asserts.GE(3.14, 3)
end

function tests.GE_expectFailure()
  shouldFail(function () asserts.GE(0, 1) end)
  shouldFail(function () asserts.GE(-3, 0) end)
  shouldFail(function () asserts.GE(3, 3.14) end)

  shouldFailWithMessage(function () asserts.GE(0, 1) end,
    'Expected: 0 >= 1')
  shouldFailWithMessage(function () asserts.GE(0, 1, 'monkey') end,
    'Expected: 0 >= 1 monkey')
end

function tests.LT_expectSuccess()
  asserts.LT(0, 1)
  asserts.LT(-3, 0)
  asserts.LT(3, 3.14)
end

function tests.LT_expectFailure()
  shouldFail(function () asserts.LT(1, 0) end)
  shouldFail(function () asserts.LT(0, -3) end)
  shouldFail(function () asserts.LT(3, 3) end)
  shouldFail(function () asserts.LT(3.14, 3) end)

  shouldFailWithMessage(function () asserts.LT(1, 0) end,
    'Expected: 1 < 0')
  shouldFailWithMessage(function () asserts.LT(1, 0, 'monkey') end,
    'Expected: 1 < 0 monkey')
end

function tests.LE_expectSuccess()
  asserts.LE(0, 0)
  asserts.LE(0, 1)
  asserts.LE(-3, 0)
  asserts.LE(-3, -3)
  asserts.LE(3, 3.14)
end

function tests.LE_expectFailure()
  shouldFail(function () asserts.LE(1, 0) end)
  shouldFail(function () asserts.LE(0, -3) end)
  shouldFail(function () asserts.LE(3.14, 3) end)

  shouldFailWithMessage(function () asserts.LE(1, 0) end,
    'Expected: 1 <= 0')
  shouldFailWithMessage(function () asserts.LE(1, 0, 'monkey') end,
    'Expected: 1 <= 0 monkey')
end


function tests.TablesEQ_expectSuccess()
  asserts.tablesEQ({}, {})
  asserts.tablesEQ({2, 4, 6}, {2, 4, 6})
  local tbl = {'foo', 'bar'}
  asserts.tablesEQ(tbl, tbl)
  asserts.tablesEQ({pet = 'monkey', power = 'magic'},
    {pet = 'monkey', power = 'magic'})

  asserts.tablesEQ({2, 4, {nested = 6}}, {2, 4, {nested = 6}})
end

function tests.TablesEQ_expectFailure()
  shouldFail(function () asserts.tablesEQ({}, {1}) end)
  shouldFail(function () asserts.tablesEQ({1}, {}) end)
  shouldFail(function () asserts.tablesEQ({2, 4, 6}, {2, 4}) end)
  shouldFail(function () asserts.tablesEQ({2, 4}, {2, 4, 6}) end)
  shouldFail(function () asserts.tablesEQ({2, 4, 6}, {3, 4, 6}) end)
  shouldFail(function () asserts.tablesEQ({2, 4, 6}, {3, 4, 6}) end)
  shouldFail(function () asserts.tablesEQ({2, 4, 6}, {2, 5, 6}) end)
  shouldFail(function () asserts.tablesEQ({2, 4, 6}, {2, 4, 7}) end)
  shouldFail(function () asserts.tablesEQ({2, 4, 6}, {4, 2, 6}) end)

  shouldFail(function () asserts.tablesEQ(
        {2, 4, {nested = 6}},
        {2, 4, {nested = 7}})
  end)

  shouldFail(function () asserts.tablesEQ(
        {pet = 'monkey', power = 'magic'},
        {pet = 'dog', power = 'slobber'})
  end)

  shouldFailWithMessage(function () asserts.tablesEQ({}, {1}) end,
    'Expected equal table values.')

  shouldFailWithMessage(function () asserts.tablesEQ({}, {1}, 'OOPS') end,
    {'Expected equal table values', 'OOPS'})
end



function tests.TablesNE_expectSuccess()
  asserts.tablesNE({}, {1})
  asserts.tablesNE({1}, {})
  asserts.tablesNE({2, 4, 6}, {2, 4})
  asserts.tablesNE({2, 4}, {2, 4, 6})
  asserts.tablesNE({2, 4, 6}, {3, 4, 6})
  asserts.tablesNE({2, 4, 6}, {3, 4, 6})
  asserts.tablesNE({2, 4, 6}, {2, 5, 6})
  asserts.tablesNE({2, 4, 6}, {2, 4, 7})
  asserts.tablesNE({2, 4, 6}, {4, 2, 6})

  asserts.tablesNE({2, 4, {nested = 6}}, {2, 4, {nested = 7}})
  asserts.tablesNE({pet = 'monkey', power = 'magic'},
    {pet = 'dog', power = 'slobber'})
end

function tests.TablesNE_expectFailure()
  shouldFail(function () asserts.tablesNE({}, {}) end)
  shouldFail(function () asserts.tablesNE({2, 4, 6}, {2, 4, 6}) end)

  local tbl = {'foo', 'bar'}
  shouldFail(function () asserts.tablesNE(tbl, tbl) end)

  shouldFail(function () asserts.tablesNE(
        {pet = 'monkey', power = 'magic'},
        {pet = 'monkey', power = 'magic'})
  end)

  shouldFail(function () asserts.tablesNE(
        {2, 4, {nested = 6}},
        {2, 4, {nested = 6}})
  end)

  shouldFailWithMessage(function () asserts.tablesNE({1}, {1}) end,
    'Expected tables with different values.')

  shouldFailWithMessage(function () asserts.tablesNE({1}, {1}, 'OOPS') end,
    {'Expected tables with different values.', 'OOPS'})
end

return test_runner.run(tests)
