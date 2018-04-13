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

local tensor = require 'dmlab.system.tensor'
local random = require 'common.random'
local pickups = require 'common.pickups'
local custom_observations = require 'decorators.custom_observations'
local timeout = require 'decorators.timeout'
local map_maker = require 'dmlab.system.map_maker'
local randomMap = random(map_maker:randomGen())

local OBJECT_NUM = 10

local PICKUPS = {
    hat = {
        name = 'Hat',
        classname = 'hat',
        model = 'models/hr_hat.md3',
        quantity = -1,
        type = pickups.type.REWARD,
        moveType = pickups.moveType.STATIC,
    },
    can = {
        name = 'Can',
        classname = 'can',
        model = 'models/hr_can.md3',
        quantity = 1,
        type = pickups.type.REWARD,
        moveType = pickups.moveType.STATIC,
    },
    cake = {
        name = 'Cake',
        classname = 'cake',
        model = 'models/hr_cake.md3',
        quantity = -1,
        type = pickups.type.REWARD,
        moveType = pickups.moveType.STATIC,
    },
    balloon = {
        name = 'Balloon',
        classname = 'balloon',
        model = 'models/hr_balloon.md3',
        quantity = 1,
        type = pickups.type.REWARD,
        moveType = pickups.moveType.STATIC,
    }
}

local ITEMS = {'hat', 'can', 'cake', 'balloon'}

local ITEMS_ID = {
    hat = 1,
    can = 2,
    cake = 3,
    balloon = 4,
}

local MAZE_HEIGHT = 10
local MAZE_WIDTH = 9

local GRID_SIZE = 150
local MIN_X = -450
local MIN_Y = -700
local ITEMS_OFFSET_Z = 24

local REPLACE_TEXTURE_MAP = {
    ['textures/map/lab_games/lg_style_01_floor_orange_d.tga'] =
        'textures/map/lab_games/lg_style_02_floor_blue_bright_d.tga',
    ['textures/map/lab_games/lg_style_01_wall_green_d.tga'] =
        'textures/map/lab_games/lg_style_05_wall_red_bright_d.tga',
    ['textures/map/lab_games/lg_style_01_floor_light_m.tga'] =
        'textures/map/lab_games/lg_style_02_floor_light_m.tga',
}

local factory = {}

--[[ Creates an API for collect good objects tasks.
There are 10 good objects and 10 bad objects in the maze. The map finishes when
the player has picked up 10 objects or the time limit is reached. Each epsisode
contains exactly 1 map.

Keyword arguments:

*   `episodeLengthSeconds` (number) Episode length in seconds.
*   `configs` (table) Arguments used to decide the objects model and maze
    appearance.
]]
function factory.createLevelApi(kwargs)
  assert(kwargs.episodeLengthSeconds)
  assert(kwargs.configs)

  local api = {}

  local function shuffledPositions(height, width, count)
    if height * width < count then
      error('There are not enough posistions to pick from.')
    end
    local positions = {}
    for i = 1, height do
      for j = 1, width do
        positions[#positions + 1] = {i, j}
      end
    end
    return random:shuffleInPlace(positions, count)
  end

  function api:start(episode, seed)
    random:seed(seed)
    randomMap:seed(random:mapGenerationSeed())
    api._timeOut = nil
  end

  function api:nextMap()
    api._shuffledPositions =
        shuffledPositions(MAZE_HEIGHT, MAZE_WIDTH, OBJECT_NUM * 2 + 1)
    api._pickupCount = 0
    local config = random:choice(kwargs.configs)
    api._pickups = config.pickups
    api._replaceWallAndFloor = config.replaceWallAndFloor
    return 'rooms_collect_good_objects'
  end

  function api:createPickup(classname)
    return PICKUPS[classname]
  end

  function api:pickup(entityId)
    local object = ITEMS[entityId]
    assert(object)
    api._pickupCount = api._pickupCount + 1
    if api._pickupCount >= OBJECT_NUM then
      api._timeOut = api._time + 0.2
    end
  end

  function api:_getOrigin(inx)
    local pos = api._shuffledPositions[inx]
    local x = (pos[1] - 1) * GRID_SIZE + MIN_X
    local y = (pos[2] - 1) * GRID_SIZE + MIN_Y
    return x .. ' ' .. y .. ' ' .. ITEMS_OFFSET_Z
  end

  function api:updateSpawnVars(spawnVars)
    if spawnVars.classname == 'reward_object' then
      local inx = tonumber(spawnVars.script_id)
      if inx <= OBJECT_NUM then
        spawnVars.classname = api._pickups.A
      else
        spawnVars.classname = api._pickups.B
      end
      spawnVars.id = tostring(ITEMS_ID[spawnVars.classname])
      spawnVars.origin = api:_getOrigin(inx)
      spawnVars.wait = '-1' -- set to -1 so that it will not respawn
    elseif spawnVars.classname == 'info_player_start' then
      spawnVars.origin = api:_getOrigin(OBJECT_NUM * 2 + 1)
    end
    return spawnVars
  end

  function api:replaceTextureName(textureName)
    if api._replaceWallAndFloor then
      return REPLACE_TEXTURE_MAP[textureName]
    end
  end

  local function fillSingleColor(tensorData, color)
    local r, g, b = unpack(color)

    local function overrideDark(amount)
      return function(val)
        return val < 128 and amount or val
      end
    end
    tensorData:select(3, 1):apply(overrideDark(r))
    tensorData:select(3, 2):apply(overrideDark(g))
    tensorData:select(3, 3):apply(overrideDark(b))
  end

  function api:modifyTexture(textureName, tensorData)
    if textureName == 'textures/model/hr_hat_d' then
      fillSingleColor(tensorData, {58, 31, 255})
      return true
    elseif textureName == 'textures/model/hr_can_d' then
      fillSingleColor(tensorData, {241, 255, 30})
      return true
    elseif textureName == 'textures/model/hr_cake_d' then
      fillSingleColor(tensorData, {36, 255, 174})
      return true
    elseif textureName == 'textures/model/hr_balloon_d' then
      fillSingleColor(tensorData, {255, 12, 154})
      return true
    end
    return false
  end

  function api:hasEpisodeFinished(timeSeconds)
    api._time = timeSeconds
    return api._timeOut and timeSeconds > api._timeOut
  end

  custom_observations.decorate(api)
  timeout.decorate(api, kwargs.episodeLengthSeconds)
  return api
end

return factory
