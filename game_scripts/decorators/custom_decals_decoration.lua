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

local datasets_selector = require 'datasets.selector'
local texture_sets = require 'themes.texture_sets'
local tensor = require 'dmlab.system.tensor'
local decals = require 'themes.decals'

local decorator = {}

local SUBSTITUTION = {}

function decorator.scale(scale)
  for _, decal in ipairs(decals.decals) do
    decal.scale = scale
  end
end

function decorator.randomize(name, rng)
  local dataset = decorator._dataset
  if name ~= decorator._datasetName then
    dataset = datasets_selector.loadDataset(name)
  end
  assert(dataset)
  decorator._dataset = dataset
  decorator._datasetName = name
  local gen = rng:shuffledIndexGenerator(dataset:getSize())
  for _, k in ipairs(decals.images) do
    SUBSTITUTION[k] = gen()
  end
end

function decorator.decorate(api)
  local loadTexture = api.loadTexture
  function api:loadTexture(textureName)
    local index = SUBSTITUTION[textureName]
    if index then
      local result = decorator._dataset:getImage(index)
      local shape = result:shape()
      local outTensor = tensor.ByteTensor(shape[1], shape[2], 4)
      -- Copying by 3 select calls is faster than via a one narrow.
      outTensor:select(3, 1):copy(result:select(3, 1))
      outTensor:select(3, 2):copy(result:select(3, 2))
      outTensor:select(3, 3):copy(result:select(3, 3))
      outTensor:select(3, 4):fill(255)
      return outTensor
    end
    if loadTexture then
      return loadTexture(self, textureName)
    end
  end
end

return decorator
