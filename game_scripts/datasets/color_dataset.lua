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

-- Generates as many colors as needed and images are the size requested.
-- Maybe used in place of missing datasets.

local tensor = require 'dmlab.system.tensor'
local colors = require 'common.colors'

--[[ Extracts an integer from another integer and returns the residuals.

Arguments:

*   'count' The number of rows.
*   'amount' The range to extract.
*   'row' is the integer to extract from. (zero-indexed).

Returns:

*   'val' The number extracted (zero-indexed).
*   'row' The residual row.
*   'count' The residual count.
]]
local function extract(row, count, amount)
  local val = row % amount
  count = math.ceil(count / amount)
  row = math.floor(row / amount)
  return val, row, count
end

local COLOURS = 60
local function randomCol(imageIndex, count)
  local h, s, l
  local row = imageIndex - 1
  h, row, count = extract(row, count, COLOURS)
  local amount = math.ceil(math.sqrt(count))
  s, row, count = extract(row, count, amount)
  l, row, count = extract(row, count, amount)
  return colors.hslToRgb(h * 360 / COLOURS,
                         1.0 - 0.7 * s / amount,  -- Saturation from [0.3, 1.0]
                         0.6 * l / amount + 0.2)  -- Light from [0.2, 0.8]
end


--[[ Returns a solid colored image dataset.

Arguments:

*   'height' Height of each image.
*   'width' Width of each image.
*   'count' Number of rows in the dataset.
]]
local function dataset(height, width, count)
  assert(count > 0, 'count must be > 0')
  assert(height > 0, 'height must be > 0')
  assert(width > 0, 'width must be > 0')
  local colorDataset = {}

  function colorDataset:getImage(imageIndex)
    local result = tensor.ByteTensor(height, width, 3)
    result:fill{randomCol(imageIndex, count)}
    return result
  end

  function colorDataset:getLabel(imageIndex)
    return extract(imageIndex - 1, count, COLOURS)
  end

  function colorDataset:getSize()
    return count
  end
  return colorDataset
end


return dataset
