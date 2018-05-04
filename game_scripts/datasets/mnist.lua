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


-- Binnary image file format.
-- [16] Header + n * [28 x 28 (greyscale)]
local HEADER = 16
local WIDTH = 28
local HEIGHT = 28
-- Binnary label file format.
-- [8] Header + n * 1
local HEADER_LABEL = 8

local function mnistSet(path, imagePath, labelPath, count)
  local mnist = {}

  function mnist:getImage(imageIndex)
    local row = imageIndex - 1
    local result = tensor.ByteTensor(HEIGHT * WIDTH, 3)
    local bw = tensor.ByteTensor{
        file = {
            name = helpers.pathJoin(path, imagePath),
            byteOffset = HEIGHT * WIDTH * row + HEADER,
            numElements = HEIGHT * WIDTH
        }
    }
    result:select(2, 1):copy(bw)
    result:select(2, 2):copy(bw)
    result:select(2, 3):copy(bw)
    return result:reshape{HEIGHT, WIDTH, 3}
  end

  function mnist:getLabel(imageIndex)
    local row = imageIndex - 1
    local label = tensor.ByteTensor{
        file = {
            name = helpers.pathJoin(path, labelPath),
            byteOffset = row + HEADER_LABEL,
            numElements = 1
        }
    }
    return label:val()
  end

  function mnist:getSize()
    return count
  end

  return mnist
end

local function mnist(kwargs)
  local path = setting_overrides:settings().datasetPath
  if path == 'dummy' then
    return color_dataset(HEIGHT, WIDTH, kwargs.test and 10000 or 60000)
  end
  if path == '' then
    path = DATASET_PATH
  end
  assert(path ~= '',
         '\n Follow instructions to download datasets here: ' ..
         '\n "data/mnist/README.md"' ..
         '\n and update DATASET_PATH to point the data folder.')
  if kwargs.test then
    return mnistSet(
        path, 't10k-images-idx3-ubyte', 't10k-labels-idx1-ubyte', 10000)
  else
    return mnistSet(
        path, 'train-images-idx3-ubyte', 'train-labels-idx1-ubyte', 60000)
  end
end

return mnist
