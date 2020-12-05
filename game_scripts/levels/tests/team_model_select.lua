--[[ Copyright (C) 2017-2018 Google Inc.

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

local debug_observations = require 'decorators.debug_observations'
local factory = require 'factories.lasertag.factory'
local make_map = require 'common.make_map'
local maze_generation = require 'dmlab.system.maze_generation'
local game = require 'dmlab.system.game'
local events = require 'dmlab.system.events'
local image = require 'dmlab.system.image'
local colors = require 'common.colors'

local MAP = [[
*************************
*P *P *P *P *P *P *P *P *
*                       *
*************************
]]

local api = factory.createLevelApi{
    episodeLengthSeconds = 60 * 5,
    botCount = 6,
}

local spawnCount = 1
function api:updateSpawnVars(spawnVars)
  if spawnVars.classname == 'info_player_start' then
    -- Spawn facing East.
    spawnVars.angle = '0'
    spawnVars.randomAngleRange = '0'
    -- Make Bot spawn on first 'P' and player on second 'P'.
    spawnVars.nohumans = spawnCount > 1 and '1' or '0'
    spawnVars.nobots = spawnCount == 1 and '1' or '0'
    spawnCount = spawnCount + 1
  end
  return spawnVars
end

function api:nextMap()
  api._botColors = {}
  local maze = maze_generation:mazeGeneration{entity = MAP}
  debug_observations.setMaze(maze)
  return make_map.makeMap{
      mapName = 'two_players',
      mapEntityLayer = MAP,
      allowBots = true,
  }
end

local characterSkinData
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
    local r, g, b = unpack(rgbs[i])
    local skinC = charachterSkin:clone()
    skinC:select(3, 1):mul(r / 255.0)
    skinC:select(3, 2):mul(g / 255.0)
    skinC:select(3, 3):mul(b / 255.0)
    playerSkin:cadd(skinC)
  end
end

function api:playerModel(playerId, playerName)
  if playerId == 1 then
    return "crash_color"
  elseif playerId < 9 then
    return 'crash_color/skin' .. playerId - 1
  end
  return "crash"
end

function api:newClientInfo(playerId, playerName, playerModel)
  local rgb = {colors.hsvToRgb(playerId * 40, 1.0, 1.0)}
  local _, _, id = string.find(playerModel, 'crash_color/skin(%d*)')
  id = id and id + 1 or 1
  api._botColors[id] = rgb
  events:add('newClientInfo' .. id)
end

local modifyTexture = api.modifyTexture
function api:modifyTexture(name, texture)
  local _, _, id = string.find(name,
    'models/players/crash_color/skin_base(%d*).tga')
  if id then
    if id == '' then
      id = 1
    else
      id = tonumber(id) + 1
    end
    local rgb = api._botColors[id]
    updateSkin(texture, {rgb, rgb, rgb})
    events:add('skinModified' .. id)
    return true
  end
  return modifyTexture and modifyTexture(self, texture) or false
end

return api
