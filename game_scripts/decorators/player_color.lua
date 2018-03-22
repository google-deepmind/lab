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
local events = require 'dmlab.system.events'
local image = require 'dmlab.system.image'
local colors = require 'common.colors'

local function decorator(api)
  assert(api.playerColor, 'Must add playerColor api function')
  local players = {}
  local botColors = {}
  local currentMap
  local characterSkinData
  local baseTexture

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

  local function updateSkin(playerSkin, rgbs)
    local skins = characterSkins()
    for i, charachterSkin in ipairs(skins) do
      local rgb = rgbs[i]
      for j = 1, 3 do
        playerSkin:select(3, j):cadd(
          charachterSkin:select(3, j):clone():mul(rgb[j] / 255.0))
      end
    end
  end

  local nextMap = api.nextMap
  function api:nextMap()
    currentMap = nextMap(self)
    if currentMap ~= '' then
      players = {}
      botColors = {}
    end
    return currentMap
  end

  local playerModel = api.playerModel
  function api:playerModel(playerId, playerName)
    if playerId == 1 then
       return 'crash_color'
    elseif playerId < 9 then
      return 'crash_color/skin' .. playerId - 1
    else
      return 'crash'
    end
  end

  local newClientInfo = api.newClientInfo
  function api:newClientInfo(playerId, playerName, playerModel)
    local _, _, id = string.find(playerModel, 'crash_color/skin(%d*)')
    botColors[id or ''] = api.playerColor(self, playerId, playerName)
    players[playerId] = {name = playerName, modelId = id or ''}
    if newClientInfo then
        newClientInfo(self, playerId, playerName, playerModel)
    end
  end

  local modifyTexture = api.modifyTexture
  function api:modifyTexture(name, texture)
    local _, _, id = string.find(name,
      'models/players/crash_color/skin_base(%d*).tga')
    if id then
      if id == '' then
        baseTexture = texture:clone()
      end
      updateSkin(texture, botColors[id])
      return true
    end
    return modifyTexture and modifyTexture(self, name, texture) or false
  end

  -- Required to set the color of the bots. Call from call mapLoaded API.
  local mapLoaded = api.mapLoaded
  function api:mapLoaded()
    if currentMap == '' then
      for playerId, player in pairs(players) do
        botColors[player.modelId] = api.playerColor(self, playerId, player.name)
        local texture = baseTexture:clone()
        updateSkin(texture, botColors[player.modelId])
        game:updateTexture('models/players/crash_color/skin_base' ..
                           player.modelId .. '.tga', texture)
      end
    end
    if mapLoaded then
      mapLoaded(self)
    end
  end

end

return {decorate = decorator}
