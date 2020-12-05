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

--[[ Factory for creating a keysdoors game.

Present the agent with a goal and a set of keys and doors, it must pick up the
correct key to open the corresponding door in a correct sequence to reach the
goal
]]

local custom_observations = require 'decorators.custom_observations'
local game = require 'dmlab.system.game'
local helpers = require 'common.helpers'
local make_map = require 'common.make_map'
local map_maker = require 'dmlab.system.map_maker'
local pickups = require 'common.pickups'
local random = require 'common.random'
local randomMap = random(map_maker:randomGen())
local tensor = require 'dmlab.system.tensor'
local texture_sets = require 'themes.texture_sets'
local timeout = require 'decorators.timeout'

local factory = {}

local ITEMS = {
    key = {
        name = 'Key',
        classname = 'key',
        model = 'models/hr_key_lrg.md3',
        quantity = 1,
        type = pickups.type.REWARD,
    },
    goal = {
        name = 'Goal',
        classname = 'goal',
        model = 'models/hr_ice_lolly_lrg.md3',
        quantity = 50,
        type = pickups.type.REWARD
    },
}

local POSSIBLE_COLORS = {
    red = {1, 0, 0, 1},
    green = {0, 1, 0, 1},
    blue = {0, 0, 1, 1},
    white = {1, 1, 1, 1},
    black = {0, 0, 0, 1}
}

local PICKUPS = {K = 'key', G = 'goal'}

local KEY_PREFIX = 'color_key_'
local DOOR_TEXTURE_PLACEHOLDER = 'door_placeholder'

local GOAL_TEXTURE_NAME = 'textures/model/hr_icelolly_d'
local GOAL_OBJECT_COLOUR = {255, 153, 0}  -- Orange
local GOAL_ID = -1

local PARAMS_WHITELIST = {
    episodeLengthSeconds = true,
    invocationMode = true,
    allowHoldOutLevels = true,
    levelGenerator = true,
    playerId = true,
    players = true,
    randomSeed = true,
    datasetPath = true,
    logLevel = true,
}

--[[ Creates a Keysdoors API.

Keyword arguments:

*   `episodeLengthSeconds` (number) Episode length in seconds.
*   `levelGenerator` (function) Generates the level map and information about
    keys and doors
]]
function factory.createLevelApi(kwargs)
  assert(kwargs.episodeLengthSeconds)
  local api = {}

  -- Any unknown params are put into the level generator config. If the level
  -- generator doesn't know about them either, it will raise an error.
  function api:init(params)
    api._genArgs = {}

    for k, v in pairs(kwargs) do
      if not PARAMS_WHITELIST[k] then
        api._genArgs[k] = v
      end
    end

    for k, v in pairs(params) do
      if not PARAMS_WHITELIST[k] then
        api._genArgs[k] = helpers.fromString(v)
      end
    end
  end

  function api:start(episode, seed)
    random:seed(seed)
    randomMap:seed(random:mapGenerationSeed())
    api._level = kwargs.levelGenerator(api._genArgs, seed)
    assert(api._level.map)
    assert(api._level.keys)
    assert(api._level.doors)
    api._keys = api._level.keys
    api._doors = {}
    api._doorsOpened = {}
    api._doorsCount = 0
    api._keysCount = 0
    api._carriedKeyColor = nil
    api._timeOut = nil
    api._map = make_map.makeMap{
        mapName = "keysdoors_demo_map",
        mapEntityLayer = api._level.map,
        textureSet = texture_sets.INVISIBLE_WALLS,
        useSkybox = true,
        callback = function (i, j, c, maker)
          if PICKUPS[c] then
            return maker:makeEntity{
                i = i,
                j = j,
                classname = PICKUPS[c]
            }
          end
          if c == 'I' then
            return maker:makeFenceDoor{
                i = i,
                j = j,
                isEastWest = true,
            }
          end
          if c == 'H' then
            return maker:makeFenceDoor{
                i = i,
                j = j,
                isEastWest = false,
            }
          end
        end
    }
  end

  function api:nextMap()
    return api._map
  end

  function api:createPickup(className)
    if className:sub(1, #KEY_PREFIX) == KEY_PREFIX then
      local obj = {}
      for k, v in pairs(ITEMS['key']) do
        obj[k] = v
      end
      local color = className:match(KEY_PREFIX .. '(.*)')
      obj.classname = obj.classname .. '_' .. color
      obj.model = KEY_PREFIX .. color .. ':' .. obj.model
      return obj
    end
    return ITEMS[className]
  end

  function api:pickup(entityId)
    if entityId == GOAL_ID then
      api._timeOut = api._time + 0.2
    end
    if api._keys[entityId] then
      api._carriedKeyColor = api._keys[entityId]
    end
  end

  function api:canTrigger(entityId, targetName)
    return not api._doorsOpened[targetName] and
        api._carriedKeyColor == api._doors[targetName]
  end

  function api:trigger(entityId, targetName)
    if api._doors[targetName] and not api._doorsOpened[targetName] then
      api._doorsOpened[targetName] = true
      api._carriedKeyColor = nil
      game:addScore(1)
    end
  end

  function api:updateSpawnVars(spawnVars)
    local classname = spawnVars.classname
    if classname == 'key' then
      api._keysCount = api._keysCount + 1
      spawnVars.classname = KEY_PREFIX .. api._keys[api._keysCount]
      spawnVars.id = tostring(api._keysCount)
      spawnVars.wait = "-1" -- set to -1 so that key will not respawn
    end
    if classname == 'func_door' then
      api._doorsCount = api._doorsCount + 1
      spawnVars.id = tostring(api._doorsCount + 100)
      spawnVars.wait = "1000000" --[[ sufficient large number to make the door
                                      open forever once it has been opened.]]
      api._doors[spawnVars.targetname] = api._level.doors[api._doorsCount]
    end
    if classname == 'goal' then
      spawnVars.id = tostring(GOAL_ID)
      spawnVars.wait = "-1"
    end

    return spawnVars
  end

  function api:filledRectangles(args)
    local rgba = api._carriedKeyColor and POSSIBLE_COLORS[api._carriedKeyColor]
    if rgba then
      return {{
          x = 12,
          y = 12,
          width = 48,
          height = 48,
          rgba = rgba
      }}
    end
    return {}
  end

  function api:replaceModelName(modelName)
    if modelName:sub(1, #KEY_PREFIX) == KEY_PREFIX then
      local prefixTexture, newModelName = modelName:match('(.*:)(.*)')
      return newModelName, prefixTexture
    end
  end

  function api:modifyTexture(textureName, tensorData)
    if textureName == GOAL_TEXTURE_NAME then
      local r, g, b = unpack(GOAL_OBJECT_COLOUR)
      local function addClamped(amount)
        return function(val)
          local result = val + amount
          return result < 255 and result or 255
        end
      end
      tensorData:select(3, 1):apply(addClamped(r))
      tensorData:select(3, 2):apply(addClamped(g))
      tensorData:select(3, 3):apply(addClamped(b))
      return true
    end
    return false
  end

  local function createSingleColorTexture(color)
    local texture = tensor.ByteTensor(8, 8, 4)
    local r, g, b = unpack(POSSIBLE_COLORS[color])
    texture:select(3, 1):fill(r * 255)
    texture:select(3, 2):fill(g * 255)
    texture:select(3, 3):fill(b * 255)
    return texture
  end

  function api:loadTexture(textureName)
    local target = textureName:match(DOOR_TEXTURE_PLACEHOLDER .. ':(.*)')
    if target and api._doors[target] then
      return createSingleColorTexture(api._doors[target])
    end

    local keyColor = textureName:match(KEY_PREFIX .. '(.*):.*')
    if keyColor then
      return createSingleColorTexture(keyColor)
    end
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

