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

local brady_konkle_oliva2008 = require 'datasets.brady_konkle_oliva2008'
local cifar10 = require 'datasets.cifar10'
local mnist = require 'datasets.mnist'
local color_dataset = require 'datasets.color_dataset'

local selector = {}

--[[ Takes a string name, returns a dataset object.

A dataset object will supply two functions, 'getSize', which returns the
number of images in the dataset, and 'getImage', which when supplied with
an index returns an interlaced image tensor.
--]]

function selector.loadDataset(name)
  if name == 'brady_konkle_oliva2008' then
    return brady_konkle_oliva2008{}
  elseif name == 'cifar10' then
    return cifar10{}
  elseif name == 'mnist' then
    return mnist{}
  elseif name == 'color' then
    return color_dataset(8, 8, 1500)
  end
  return nil
end

return selector
