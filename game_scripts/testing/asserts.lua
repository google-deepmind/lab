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

local asserts = {}

local function fail(message, optional)
  optional = optional and (' ' .. optional) or ''
  error(message .. optional, 3)
end

local function areTablesEqual(target, expect)
  local targetSize = 0
  for k, v in pairs(target) do
    targetSize = targetSize + 1
  end
  for k, v in pairs(expect) do
    if type(v) == 'table' then
      if not areTablesEqual(target[k], v) then
        return false
      end
    elseif target[k] ~= v then
      return false
    end
    targetSize = targetSize - 1
  end
  if targetSize ~= 0 then
    -- There were values in target not present in expect
    return false
  end
  return true
end

function asserts.EQ(target, expect, msg)
  if target ~= expect then
    fail('Expected: ' .. tostring(expect) .. '. ' ..
         'Actual: ' .. tostring(target) .. '.', msg)
  end
end

function asserts.NE(target, expect, msg)
  if target == expect then
    fail('Expected values to differ: ' .. tostring(expect), msg)
  end
end

-- Assert target > expect
function asserts.GT(target, expect, msg)
  if target <= expect then
    fail('Expected: ' .. tostring(target) .. ' > ' .. tostring(expect), msg)
  end
end

-- Assert target >= expect
function asserts.GE(target, expect, msg)
  if target < expect then
    fail('Expected: ' .. tostring(target) .. ' >= ' .. tostring(expect), msg)
  end
end

-- Assert target < expect
function asserts.LT(target, expect, msg)
  if target >= expect then
    fail('Expected: ' .. tostring(target) .. ' < ' .. tostring(expect), msg)
  end
end

-- Assert target <= expect
function asserts.LE(target, expect, msg)
  if target > expect then
    fail('Expected: ' .. tostring(target) .. ' <= ' .. tostring(expect), msg)
  end
end


function asserts.tablesEQ(target, expect, msg)
  if type(target) ~= 'table' then fail('1st argument should be a table') end
  if type(expect) ~= 'table' then fail('2nd argument should be a table') end
  if not areTablesEqual(target, expect) then
    fail("Expected equal table values.", msg)
  end
end

function asserts.tablesNE(target, expect, msg)
  if type(target) ~= 'table' then fail('1st argument should be a table') end
  if type(expect) ~= 'table' then fail('2nd argument should be a table') end
  if areTablesEqual(target, expect) then
    fail("Expected tables with different values.", msg)
  end
end

function asserts.shouldFail(fn)
  local status, out = pcall(fn)
  if status then
    error("Expected an error but call succeeded.", 2)
  end
end

return asserts
