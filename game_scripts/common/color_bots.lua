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

local SINGLE_BOT_SKIN_TEXTURE = 'models/players/crash_color/skin_base.tga'
local SATURATION = 1.0
local VALUE = 1.0

local color_bots = {}

color_bots.BOT_NAMES = {
    'Cygni',
    'Leonis',
    'Epsilon',
    'Cephei',
    'Centauri',
    'Draconis',
}

color_bots.BOT_NAMES_COLOR = {
    'CygniColor',
    'LeonisColor',
    'EpsilonColor',
    'CepheiColor',
    'CentauriColor',
    'DraconisColor',
}

local characterSkinData = nil

local function characterSkins()
  if not characterSkinData then
    local playerDir = game:runFiles() .. '/baselab/game_scripts/player/'
    characterSkinData = {
        image.load(playerDir .. 'dm_character_skin_mask_a.png'),
        image.load(playerDir .. 'dm_character_skin_mask_b.png'),
        image.load(playerDir .. 'dm_character_skin_mask_c.png'),
    }
  end
  return characterSkinData
end

--[[ Returns a list of bots compatible with the addBot API.

Keyword arguments:

*   count - Number [0, #color_bots.BOT_NAMES] - Number of bots to create.
*   color - Boolean (false) - Whether to create the bots with custom colors.
*   skill - Number - Skill level of bot. In range [1, 5]
]]
function color_bots:makeBots(kwargs)
  assert(kwargs.count, 'Missing count')
  local bots = {}
  local botNames = kwargs.color and self.BOT_NAMES_COLOR or self.BOT_NAMES
  for i, name in ipairs(botNames) do
    if i == kwargs.count + 1 then
      break
    end
    bots[#bots + 1] = {name = name, skill = kwargs.skill or 4.0}
  end
  return bots
end

local playerSkinOriginal = nil
local playerSkin = nil
local playerSkinHue = nil

local function createSkin(hue)
  if hue ~= playerSkinHue and playerSkinOriginal then
    playerSkin = playerSkinOriginal:clone()
    local skins = characterSkins()
    local hueAngle = 360 / #skins
    for i, charachterSkin in ipairs(skins) do
      local r, g, b = colors.hsvToRgb(hue, SATURATION, VALUE)
      local skinC = charachterSkin:clone()
      skinC:select(3, 1):mul(r / 255.0)
      skinC:select(3, 2):mul(g / 255.0)
      skinC:select(3, 3):mul(b / 255.0)
      playerSkin:cadd(skinC)
      hue = (hue + hueAngle) % 360
    end
    playerSkinHue = hue
    collectgarbage()
    collectgarbage()
  end
  return playerSkin
end

-- Required to inform bot api of the skin texture used by the bots. Call
-- from modifyTexture API if color bots are desirded.
function color_bots:modifySkin(name, texture, hue)
  if name == SINGLE_BOT_SKIN_TEXTURE then
    playerSkinOriginal = texture:clone()
    texture:copy(createSkin(hue))
    return true
  end
  return false
end

-- Required to set the color of the bots. Call from call mapLoaded API.
function color_bots:colorizeBots(hue)
  if playerSkinHue ~= hue then
    local skin = createSkin(hue)
    if skin then
      game:updateTexture(SINGLE_BOT_SKIN_TEXTURE, skin)
    end
  end
end

return color_bots
