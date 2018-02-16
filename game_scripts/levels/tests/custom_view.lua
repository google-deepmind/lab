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

local game = require 'dmlab.system.game'
local tensor = require 'dmlab.system.tensor'
local custom_observations = require 'decorators.custom_observations'

local api = {}
local BALLOON_TEXTURE = 'textures/model/hr_balloon_d'

local SCREEN = game:screenShape()
local SHAPE = SCREEN.buffer

-- Make balloon red.
function api:loadTexture(textureName)
  if textureName == BALLOON_TEXTURE then
    return tensor.ByteTensor(4, 4, 4):fill{255, 0, 0, 255}
  end
end

function api:nextMap()
  return 'lookat_test'
end

-- Look from above player with yaw offset.
local function angleLook(yaw)
  local function look()
    local info = game:playerInfo()
    local pos = info.eyePos
    local look = game:playerInfo().angles
    look[2] = look[2] + yaw
    local buffer = game:renderCustomView{
        width = SHAPE.width,
        height = SHAPE.height,
        pos = pos,
        look = look,
        renderPlayer = false,
    }
    return buffer:transpose(3, 2):transpose(2, 1):reverse(2):clone()
  end
  return look
end

custom_observations.decorate(api)
custom_observations.addSpec('RGB.LOOK_BACK', 'Bytes',
                            {3, SHAPE.height, SHAPE.width}, angleLook(180))
custom_observations.addSpec('RGB.LOOK_FORWARD', 'Bytes',
                            {3, SHAPE.height, SHAPE.width}, angleLook(0))

return api
