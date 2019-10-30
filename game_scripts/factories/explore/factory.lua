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

local custom_decals = require 'decorators.custom_decals_decoration'
local custom_observations = require 'decorators.custom_observations'
local property_decorator = require 'decorators.property_decorator'
local log = require 'common.log'
local helpers = require 'common.helpers'
local make_map = require 'common.make_map'
local map_maker = require 'dmlab.system.map_maker'
local maze_generation = require 'dmlab.system.maze_generation'
local pickup_decorator = require 'decorators.human_recognisable_pickups'
local random = require 'common.random'
local setting_overrides = require 'decorators.setting_overrides'
local debug_observations = require 'decorators.debug_observations'

local randomMap = random(map_maker:randomGen())

local factory = {}

--[[ Creates a level API for Exploration levels.

Keyword arguments:

*   `opts.episodeLengthSeconds` (number, default 90)
*   `opts.extraConnectionProbability` (number, default 0.0)
*   `opts.hasDoors` (boolean, default false)
*   `opts.maxVariations` (number, default 1)
*   `opts.mazeHeight` (number, default 11)
*   `opts.mazeWidth` (number, default 11)
*   `opts.objectCount` (number, default 4)
*   `opts.objectEntity (character, default 'O')
*   `opts.roomCount` (number, default 4)
*   `opts.roomMaxSize` (number, default 5)
*   `opts.roomMinSize` (number, default 3)
*   `opts.spawnCount` (number, default 5)
*   `opts.spawnEntity` (character, default 'P')
*   `opts.decalFrequency` (number, default 0.1)
*   `opts.decalScale` (number, default 1.0)
*   `opts.wallDecoration` (string, default '')
*   `level` (table) - Level table.
]]
function factory.createLevelApi(kwargs)
  assert(kwargs.level)
  kwargs.opts.useSkybox = kwargs.opts.useSkybox == nil or kwargs.opts.useSkybox
  kwargs.opts.episodeLengthSeconds = kwargs.opts.episodeLengthSeconds or 90
  kwargs.opts.extraConnectionProbability =
      kwargs.opts.extraConnectionProbability or 0.0
  kwargs.opts.hasDoors = kwargs.opts.hasDoors or false
  kwargs.opts.maxVariations = kwargs.opts.maxVariations or 1
  kwargs.opts.mazeHeight = kwargs.opts.mazeHeight or 11
  kwargs.opts.mazeWidth = kwargs.opts.mazeWidth or 11
  kwargs.opts.objectCount = kwargs.opts.objectCount or 4
  kwargs.opts.objectEntity = kwargs.opts.objectEntity or 'O'
  if kwargs.opts.quickRestart == nil then
    kwargs.opts.quickRestart = true
  end
  kwargs.opts.randomSeed = false
  kwargs.opts.roomCount = kwargs.opts.roomCount or 4
  kwargs.opts.roomMaxSize = kwargs.opts.roomMaxSize or 5
  kwargs.opts.roomMinSize = kwargs.opts.roomMinSize or 3
  kwargs.opts.spawnCount = kwargs.opts.spawnCount or 5
  kwargs.opts.spawnEntity = kwargs.opts.spawnEntity or 'P'
  kwargs.opts.decalFrequency = kwargs.opts.decalFrequency or 0.1
  kwargs.opts.decalScale = kwargs.opts.decalScale or 1.0
  kwargs.opts.wallDecoration = kwargs.opts.wallDecoration or ''
  kwargs.opts.overrideEntityLayer = kwargs.opts.overrideEntityLayer or ''
  kwargs.opts.overrideVariationsLayer = kwargs.opts.overrideVariationsLayer or
                                        ''

  local api = {}

  function api:nextMap()
    if kwargs.level.restart then
      kwargs.level:restart(api._maze)
    end
    local map = api._map
    if kwargs.opts.quickRestart then
      api._map = ''
    end
    return map
  end

  function api:pickup(spawnId)
    if kwargs.level.pickup then
      return kwargs.level:pickup(spawnId)
    end
  end

  local function mazeGen(seed)
    local maze = maze_generation.randomMazeGeneration{
        seed = seed,
        width = kwargs.opts.mazeWidth,
        height = kwargs.opts.mazeHeight,
        maxRooms = kwargs.opts.roomCount,
        maxVariations = kwargs.opts.maxVariations,
        hasDoors = kwargs.opts.hasDoors,
        spawn = kwargs.opts.spawnEntity,
        roomSpawnCount = kwargs.opts.spawnCount,
        object = kwargs.opts.objectEntity,
        roomObjectCount = kwargs.opts.objectCount,
        roomMinSize = kwargs.opts.roomMinSize,
        roomMaxSize = kwargs.opts.roomMaxSize,
        extraConnectionProbability = kwargs.opts.extraConnectionProbability,
    }
    return maze
  end

  local function entityLayer(stringSeed)
    local seed = tonumber(stringSeed)
    return seed and mazeGen(seed):entityLayer()
  end

  local function variationsLayer(stringSeed)
    local seed = tonumber(stringSeed)
    return seed and mazeGen(seed):variationsLayer()
  end

  property_decorator.addReadOnly('func.entityLayer', entityLayer)
  property_decorator.addReadOnly('func.variationsLayer', variationsLayer)

  function api:start(episode, seed)
    local mapName = 'explore_maze'
    random:seed(seed)
    randomMap:seed(seed)
    local maze = mazeGen(seed)
    kwargs.opts.generatedSeed = seed
    kwargs.opts.generatedEntityLayer = maze:entityLayer()
    kwargs.opts.generatedVariationsLayer = maze:variationsLayer()
    local override = kwargs.opts.overrideEntityLayer ~= ''
    if override then
      maze = maze_generation.mazeGeneration{
          entity = kwargs.opts.overrideEntityLayer,
          variations = kwargs.opts.overrideVariationsLayer
      }
    end
    kwargs.opts.currentEntityLayer = maze:entityLayer()
    kwargs.opts.currentVariationsLayer = maze:variationsLayer()

    if kwargs.opts.decalScale and kwargs.opts.decalScale ~= 1 then
      custom_decals.scale(kwargs.opts.decalScale)
    end
    if kwargs.opts.wallDecoration ~= '' then
      custom_decals.randomize(kwargs.opts.wallDecoration, randomMap)
      custom_decals.decorate(self)
    end

    if override then
      log.info('Maze overriden via property:\n' ..
               kwargs.opts.currentEntityLayer)
    else
      log.info('Maze Generated (seed ' .. seed .. '):\n' ..
               kwargs.opts.currentEntityLayer)
    end
    api._map = make_map.makeMap{
        mapName = mapName,
        mapEntityLayer = maze:entityLayer(),
        mapVariationsLayer = maze:variationsLayer(),
        useSkybox = kwargs.opts.useSkybox,
        decalFrequency = kwargs.opts.decalFrequency,
    }
    api._maze = maze
    if kwargs.level.start then
      kwargs.level:start(maze, episode, seed)
    end
    debug_observations.setMaze(maze)
  end

  function api:updateSpawnVars(spawnVars)
    if kwargs.level.spawnVarsUpdater and
       kwargs.level.spawnVarsUpdater[spawnVars.classname] then
      return kwargs.level.spawnVarsUpdater[spawnVars.classname](spawnVars)
    else
      return spawnVars
    end
  end

  custom_observations.decorate(api)
  pickup_decorator.decorate(api)
  setting_overrides.decorate{
      api = api,
      apiParams = kwargs.opts,
      decorateWithTimeout = true
  }
  return api
end

return factory
