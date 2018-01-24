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

-- Utility functions to construct transformation matrices as tensors.

local sys_transform = require 'dmlab.system.transform'

local transform = {}

-- Returns a 4x4 tensor with the coefficients of a column-major transformation
-- matrix which applies a translation by vector 'ofs'.
function transform.translate(ofs)
  return sys_transform.translate(ofs)
end

-- Returns a 4x4 tensor with the coefficients of a column-major transformation
-- matrix which applies a rotation of 'angle' degrees around vector 'axis'.
function transform.rotate(angle, axis)
  return sys_transform.rotate(angle, axis)
end

-- Returns a 4x4 tensor with the coefficients of a column-major transformation
-- matrix which applies a rotation of 'angle' degrees around the X axis.
function transform.rotateX(angle)
  return sys_transform.rotate(angle, {1, 0, 0})
end

-- Returns a 4x4 tensor with the coefficients of a column-major transformation
-- matrix which applies a rotation of 'angle' degrees around the Y axis.
function transform.rotateY(angle)
  return sys_transform.rotate(angle, {0, 1, 0})
end

-- Returns a 4x4 tensor with the coefficients of a column-major transformation
-- matrix which applies a rotation of 'angle' degrees around the Z axis.
function transform.rotateZ(angle)
  return sys_transform.rotate(angle, {0, 0, 1})
end

-- Returns a 4x4 tensor with the coefficients of a column-major transformation
-- matrix which applies scale factors in vector 'scl' to their corresponding
-- coordinates.
function transform.scale(scl)
  return sys_transform.scale(scl)
end

return transform
