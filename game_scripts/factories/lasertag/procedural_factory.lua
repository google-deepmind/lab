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
local make_map = require 'common.make_map'
local random = require 'common.random'
local set = require 'common.set'
local map_maker = require 'dmlab.system.map_maker'
local maze_generation = require 'dmlab.system.maze_generation'
local lt_factory = require 'factories.lasertag.factory'
local map_maker = require 'dmlab.system.map_maker'
local debug_observations = require 'decorators.debug_observations'
local randomMap = random(map_maker:randomGen())

local factory = {}

local function generateLTMaze(mazeGenerationParams, pickupParams)
  local maze = maze_generation.randomMazeGeneration(mazeGenerationParams)

  local emptyCells = {}
  for row = 1, mazeGenerationParams.height do
    for col = 1, mazeGenerationParams.width do
      if maze:getEntityCell(row, col) == " " then
        emptyCells[#emptyCells + 1] = {row = row, col = col}
      end
    end
  end

  local numWeapons = pickupParams.weaponCount
  local numPickups = pickupParams.pickupCount
  if numWeapons + numPickups > #emptyCells then
    error('Not enough empty cells to place pickups')
  end

  random:shuffleInPlace(emptyCells, numWeapons + numPickups)
  for cell = 1, numWeapons do
    maze:setEntityCell(emptyCells[cell].row, emptyCells[cell].col, 'w')
  end
  for cell = numWeapons + 1, numWeapons + numPickups do
    maze:setEntityCell(emptyCells[cell].row, emptyCells[cell].col, 'u')
  end

  return maze
end

--[[ Creates a Procedural Laser tag API.

Keyword arguments:

*   `botCount` (number, [-1, 6]) - Number of bots. (-1 for all).
*   `color` (boolean) - Whether bots change colors between levels.
*   `mazeGenerationParams` - Parameters for
    dmlab.system.maze_generation.randomMazeGeneration.
*   `pickupParams.weaponCount` - The number of weapons to distribute throughout
    the level.
*   `pickupParams.pickupCount` - The number of items (health/armor) to
    distribute throughout the level.
*   `episodeLengthSeconds` (number, default 600) - Episode length in seconds.
]]
function factory.createLevelApi(kwargs)
  kwargs.useSkybox = kwargs.useSkybox == nil or kwargs.useSkybox
  kwargs.color = kwargs.color or false
  kwargs.skill = kwargs.skill or 4.0
  kwargs.episodeLengthSeconds = kwargs.episodeLengthSeconds or false
  kwargs.mazeGenerationParams = kwargs.mazeGenerationParams or false
  kwargs.pickupParams = kwargs.pickupParams or false

  local api = lt_factory.createLevelApi(kwargs)

  local function makeEntities(i, j, c, maker)
    if c == 'w' then
      return maker:makeEntity{
            i = i,
            j = j,
            classname = random:choice({
                'weapon_rocketlauncher',
                'weapon_lightning',
                'weapon_railgun',
            }),
        }
    elseif c == 'u' then
        return maker:makeEntity{
            i = i,
            j = j,
            classname = random:choice({
                'item_health_large',
                'item_armor_combat'
            }),
        }
    end
  end

  function api:nextMap()
    return api._map
  end

  local start = api.start
  function api:start(episode, seed)
    random:seed(seed)
    randomMap:seed(random:mapGenerationSeed())
    start(api, episode, seed)
    local maze
    if kwargs.mazeGenerationParams then
      kwargs.mazeGenerationParams.seed = seed
      maze = generateLTMaze(kwargs.mazeGenerationParams,
                            kwargs.pickupParams)
    else
      error('Must specify kwargs.mazeGenerationParams')
    end

    api._map = make_map.makeMap{
        mapName = 'lt_procedural',
        mapEntityLayer = maze:entityLayer(),
        mapVariationsLayer = maze:variationsLayer(),
        useSkybox = kwargs.useSkybox,
        callback = makeEntities,
        allowBots = true
    }
    debug_observations.setMaze(maze)
  end

  return api
end

return factory
