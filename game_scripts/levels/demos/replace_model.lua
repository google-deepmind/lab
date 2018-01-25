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

-- Demonstration of modifying a texture of a model multiple times.
-- In the level there should apples and lemons of varying colors.
local tensor = require 'dmlab.system.tensor'
local pickups = require 'common.pickups'
local api = {}

function api:start(episode, seed)
  api._count = 0
end

function api:nextMap()
  return 'seekavoid_arena_01'
end

local COLORS = {
    {255, 168, 0},
    {0, 255, 28},
    {255, 98, 0},
    {250, 0, 255},
    {255, 34, 0},
    {0, 255, 160},
    {0, 223, 255},
}

local PREFIX = 'PICKUP_'

function api:createPickup(classname)
  local obj_orig = pickups.defaults[classname]
  if obj_orig ~= nil then
    local obj = {}
    for k, v in pairs(obj_orig) do
      obj[k] = v
    end
    local color = api._count % #COLORS + 1
    api._count = api._count + 1
    obj.classname = obj.classname .. '_' .. color
    obj.model = PREFIX .. color .. ':' .. obj.model
    return obj
  end
  return obj_orig
end

function api:replaceModelName(modelName)
  if modelName:sub(1, #PREFIX) == PREFIX then
    local prefixTexture, newModelName = modelName:match('(.*:)(.*)')
    return newModelName, prefixTexture
  end
end

function api:replaceTextureName(textureName)
  -- Remove the texture's prefix. This will load a new copy of the same texture
  -- ready for modifyTexture.
  if textureName:sub(1, #PREFIX) == PREFIX then
    -- Strip prefix and color id.
    return textureName:match('.*:(.*)')
  end
end

function api:modifyTexture(textureName, texture)
  if textureName:sub(1, #PREFIX) ~= PREFIX then
    return false
  end
  local color = tonumber(textureName:match('(%d+):'))
  local r, g, b = unpack(COLORS[color])
  -- Make texture black and white.
  texture:mul(1 / 3)
  local red = texture:select(3, 1)
  local green = texture:select(3, 2)
  local blue = texture:select(3, 3)
  red:cadd(green):cadd(blue)
  green:copy(red)
  blue:copy(red)

  -- Set texture color.
  red:mul(r / 255)
  green:mul(g / 255)
  blue:mul(b / 255)
  return true
end

return api

