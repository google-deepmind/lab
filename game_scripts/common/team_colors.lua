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
local colors = require 'common.colors'
local game = require 'dmlab.system.game'
local image = require 'dmlab.system.image'

local RED_TEXTURES = {
    'models/flags/r_flag.tga',
    'models/players/crash/redskin.tga',
    'icons/iconh_red',
    'icons/iconf_red1',
    'icons/iconf_red2',
    'icons/iconf_red3',
}

local BLUE_TEXTURES = {
    'models/flags/b_flag.tga',
    'models/players/crash/blueskin.tga',
    'icons/iconh_blue',
    'icons/iconf_blu1',
    'icons/iconf_blu2',
    'icons/iconf_blu3',
}

local RECOLOR_MAP = {}
for _, r in ipairs(RED_TEXTURES) do
  RECOLOR_MAP[r] = 'r'
end

for _, b in ipairs(BLUE_TEXTURES) do
  RECOLOR_MAP[b] = 'b'
end

local team_colors = {
    _teamHues = {r = 0, b = 240},
    _dirty = {r = true, b = true},
    _updateTextures = {}
}

function team_colors:setTeamColors(redHue, blueHue)
  if self._teamHues.r ~= redHue then
    self._dirty.r = true
    self._teamHues.r = redHue
    local rR, rG, rB = colors.hslToRgb(redHue, 1, 0.5)
    game:console("set cg_redteam_r " .. rR)
    game:console("set cg_redteam_g " .. rG)
    game:console("set cg_redteam_b " .. rB)
  end
  if self._teamHues.b ~= blueHue then
    self._dirty.b = true
    self._teamHues.b = blueHue
    local bR, bG, bB = colors.hslToRgb(blueHue, 1, 0.5)
    game:console("set cg_blueteam_r " .. bR)
    game:console("set cg_blueteam_g " .. bG)
    game:console("set cg_blueteam_b " .. bB)
  end
end

function team_colors:modifyTexture(textureName, img)
  local team = RECOLOR_MAP[textureName]
  if team ~= nil then
    self._updateTextures[textureName] = img:clone()
    image.setHue(img, self._teamHues[team])
    self._dirty[team] = false
    return true
  end
  return false
end

function team_colors:updateTextures()
  for textureName, texture in pairs(self._updateTextures) do
    local v = RECOLOR_MAP[textureName]
    if v and self._dirty[v] then
      local texture = self._updateTextures[textureName]:clone()
      image.setHue(texture, self._teamHues[v])
      game:updateTexture(textureName, texture)
    end
  end
  self._dirty.r = false
  self._dirty.b = false
end

return team_colors;
