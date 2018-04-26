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

local image = require 'dmlab.system.image'
local helpers = require 'common.helpers'
local game = require 'dmlab.system.game'

--[[ Given a templated path to a set of images and a count of how many images
to expect, returns a dataset object which can provide images requested by
index.
Assumes that 'template' contains a %d formatting token for the index.
]]

local function reader(basePath, template, size, loadContentFirst)
  local dataset = {}
  local datasetTemplate = helpers.pathJoin(basePath, template)
  function dataset:getImage(imageIndex)
    assert(imageIndex <= size, 'Requested image index ' .. imageIndex ..
           ' exceeds dataset size: ' .. size)
    local filename = string.format(datasetTemplate, imageIndex)
    if loadContentFirst then
      return image.load('content:.png', game:loadFileToString(filename))
    else
      return image.load(filename)
    end
  end

  function dataset:getSize()
    return size
  end

  return dataset
end

return reader
