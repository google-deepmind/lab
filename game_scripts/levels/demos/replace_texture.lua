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

-- Demonstration of modifying a texture in multiple ways.
-- In the level there should be a blue picture on the wall.

local game = require 'dmlab.system.game'
local make_map = require 'common.make_map'
local tensor = require 'dmlab.system.tensor'
local api = {}

function api:start(episode, seed)
  make_map.seedRng(1)
  api._count = 0
end

function api:nextMap()
  if api._map then
    return api._map
  end
  api._map = make_map.makeMap{
      mapName = 'demo_rectangle',
      mapEntityLayer = 'P ',
      useSkybox = true,
  }
  return api._map
end

local TEXTURE_NAME = 'textures/decal/lab_games/dec_img_style02_013'
function api:replaceTextureName(textureName)
  if textureName == TEXTURE_NAME then
    textureName = textureName .. '%01'
  end
  return textureName
end

function api:loadTexture(textureName)
  if textureName == TEXTURE_NAME .. '%01' then
    return tensor.ByteTensor(8, 8, 4):fill(255)
  end
end

local colourIndex = 0
local columnIndex = 0

local function changeTexture(textureData)
  for i = 0, 2 do
    local c = (math.floor(colourIndex) == i) and 255 or 0
    textureData:select(3, i + 1):fill(c)
  end
  textureData:select(2, math.floor(columnIndex) + 1):fill(255)
  colourIndex = (colourIndex + 0.01) % 3
  columnIndex = (columnIndex + 0.01) % 8
end

function api:modifyTexture(textureName, tensorData)
  if textureName == TEXTURE_NAME then
    assert(tensorData == tensor.ByteTensor(8, 8, 4):fill(255))
    -- Make texture red.
    changeTexture(tensorData)
    return true
  end
  return false
end

local textureData = tensor.ByteTensor(8, 8, 4):fill(255)

function api:modifyControl(actions)
  if actions.crouchJump > 0 then
    actions.crouchJump = 0
    changeTexture(textureData)
    game:updateTexture(TEXTURE_NAME, textureData)
  end
  return actions
end

return api
