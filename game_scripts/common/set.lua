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

-- Provides common set-like operations.
local set = {}

-- Insert the elements of list into existingSet.
function set.insert(existingSet, list)
  for _, v in ipairs(list) do
    existingSet[v] = true
  end
  return existingSet
end

-- Turn a list L into a set S. For any V in L, S[V] = true.
function set.Set(list)
  return set.insert({}, list)
end

function set.toList(lhs)
  local list = {}
  for key, _ in pairs(lhs) do
    list[#list + 1] = key
  end
  return list
end

function set.isSame(lhs, rhs)
  for k, _ in pairs(lhs) do
    if not rhs[k] then return false end
  end
  for k, _ in pairs(rhs) do
    if not lhs[k] then return false end
  end
  return true
end

function set.intersect(lhs, rhs)
  local intersection = {}
  for key, _ in pairs(lhs) do
    intersection[key] = rhs[key]
  end
  return intersection
end

-- Returns lhs - rhs
function set.difference(lhs, rhs)
  local out = {}
  for key, _ in pairs(lhs) do
    if not rhs[key] then
      out[key] = true
    end
  end
  return out
end

function set.union(lhs, rhs)
  local out = {}
  for key, _ in pairs(lhs) do
    out[key] = true
  end
  for key, _ in pairs(rhs) do
    out[key] = true
  end
  return out
end

return set
