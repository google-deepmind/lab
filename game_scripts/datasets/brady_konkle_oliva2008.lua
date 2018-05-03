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

local reader = require 'datasets.reader'
local game = require 'dmlab.system.game'
local setting_overrides = require 'decorators.setting_overrides'
local color_dataset = require 'datasets.color_dataset'

local DATASET_TEMPLATE = '%04d.png'
local DATASET_PATH = ''
local PRELOAD_CONTENT = false
local DATASET_SIZE = 2400


local function brady_konkle_oliva2008()
  local path = setting_overrides:settings().datasetPath
  if path == 'dummy' then
    return color_dataset(256, 256, DATASET_SIZE)
  end
  if path == '' then
    path = DATASET_PATH
  end
  assert(path ~= '',
    '\n Follow instructions to download datasets here: ' ..
    '\n "data/brady_konkle_oliva2008/README.md"' ..
    '\n and update DATASET_PATH to point the data folder.')
  return reader(path, DATASET_TEMPLATE, DATASET_SIZE, PRELOAD_CONTENT)
end


return brady_konkle_oliva2008
