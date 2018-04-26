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

local DATASET_PATH = ''
local LOAD_CONTENT_FIRST = false

-- Binnary file format.
-- [1 (category byte)][32 x 32 (red)][32 x 32 (green)][32 x 32 blue]
local WIDTH = 32
local HEIGHT = 32
local SIZE = 3 * HEIGHT * WIDTH
local HEADER = 1
local ROW_SIZE = HEADER + SIZE

local cifar10Train = {}
function cifar10Train:getImage(imageIndex)
    local imageIndex = imageIndex - 1
    local batch = math.floor(imageIndex / 10000)
    local row = imageIndex - batch * 10000
    local result = tensor.ByteTensor{
        file = {
            name = helpers.pathJoin(
                DATASET_PATH, 'data_batch_' .. batch + 1 .. '.bin'),
            byteOffset = ROW_SIZE * row + HEADER,
            numElements = SIZE
        }
    }
    -- Convert RGB Planar to RGB interlaced.
    return result:reshape{3, HEIGHT, WIDTH}:transpose(1, 2):transpose(2, 3)
end

function cifar10Train:getSize()
  return 50000
end


local cifar10Test = {}
function cifar10Test:getImage(imageIndex)
    local row = imageIndex - 1
    local result = tensor.ByteTensor{
        file = {
            name = helpers.pathJoin(DATASET_PATH, 'test_batch.bin'),
            byteOffset = ROW_SIZE * row + HEADER,
            numElements = SIZE
        }
    }
    return result:reshape{3, HEIGHT, WIDTH}:transpose(1, 2):transpose(2, 3)
end

function cifar10Test:getSize()
  return 10000
end

local function cifar(kwargs)
  assert(DATASET_PATH ~= '',
         '\n Follow instructions to download datasets here: ' ..
         '\n "data/cifar/README.md"' ..
         '\n and update DATASET_PATH to point the data folder.')
  if kwargs.test then
    return cifar10Test
  else
    return cifar10Train
  end
end

return cifar

