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

local tensor = require 'dmlab.system.tensor'
local game = require 'dmlab.system.game'
local helpers = require 'common.helpers'
local setting_overrides = require 'decorators.setting_overrides'
local color_dataset = require 'datasets.color_dataset'

local DATASET_PATH = ''
local LOAD_CONTENT_FIRST = false


-- Binnary file format.
-- [1 (category byte)][32 x 32 (red)][32 x 32 (green)][32 x 32 blue]
local WIDTH = 32
local HEIGHT = 32
local SIZE = 3 * HEIGHT * WIDTH
local HEADER = 1
local ROW_SIZE = HEADER + SIZE

--[[ Returns image from cifar binary file.

Arguments:

*   'path' Path to folder containing cifar binary files.
*   'batch' Name of binary file to extract image from.
*   'row' Zero-indexed row to extract image.

Returns:

*   ByteTensor 32x32x3 image.
]]
local function cifarImage(path, batch, row)
  local result = tensor.ByteTensor{
      file = {
          name = helpers.pathJoin(path, batch),
          byteOffset = ROW_SIZE * row + HEADER,
          numElements = SIZE
      }
  }
  -- Convert RGB Planar to RGB interlaced.
  return result:reshape{3, HEIGHT, WIDTH}:
         transpose(1, 2):transpose(2, 3):clone()
end

local function cifar10Train(path)
  local dataset = {}
  function dataset:getImage(imageIndex)
    local imageIndex = imageIndex - 1
    local batch = math.floor(imageIndex / 10000)
    local row = imageIndex - batch * 10000
    return cifarImage(path, 'data_batch_' .. batch + 1 .. '.bin', row)
  end

  function dataset:getSize()
    return 50000
  end

  return dataset
end

local function cifar10Test(path)
  local datset = {}
  function datset:getImage(imageIndex)
    local row = imageIndex - 1
    return cifarImage(path, 'test_batch.bin', row)
  end

  function datset:getSize()
    return 10000
  end

  return datset
end

local function cifar(kwargs)
  local path = setting_overrides:settings().datasetPath or ''
  if path == 'dummy' then
    return color_dataset(32, 32, kwargs.test and 10000 or 50000)
  end
  if path == '' then
    path = DATASET_PATH
  end
  assert(path ~= '',
         '\n Follow instructions to download datasets here: ' ..
         '\n "data/cifar/README.md"' ..
         '\n and update DATASET_PATH to point the data folder.')
  if kwargs.test then
    return cifar10Test(path)
  else
    return cifar10Train(path)
  end
end


return cifar

