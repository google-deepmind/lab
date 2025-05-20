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

-- Demonstration of creating a fixed level described using text.

-- local maze_generation = require 'dmlab.system.maze_generation'
-- local tensor = require 'dmlab.system.tensor'
-- local log = require 'common.log'
-- local random = require 'common.random'

local game = require 'dmlab.system.game'
local make_map = require 'common.make_map'
local pickups = require 'common.pickups'
local texture_sets = require 'themes.texture_sets'
local custom_observations = require 'decorators.custom_observations'
local setting_overrides = require 'decorators.setting_overrides'
local random = require 'common.random'

local api = {}

--[[ Text map contents:

'P' - Player spawn point. Player is spawned with random orientation.
'A' - Apple pickup. 1 reward point when picked up.
'G' - Goal object. 10 reward points and the level restarts.
'I' - Door. Open and closes West-East corridors.
'H' - Door. Open and closes North-South corridors.
'*' - Walls.

Lights are placed randomly through out and decals are randomly placed on the
walls according to the theme.
]]

local TEXT_MAP = {

[[
*********************
*G     PPPPPPPPPPPPP*
*     *PPPPPPP*PPP*P*
*    PPPPPPPPPPP*PPP*
*   *PPPPPPPPPP*PPPP*
* *PPPPPPPPPPPPPPPPP*
*PPPPPPPPPPP**PPPPP**
*PPPPP*P*PPP*PPPPPPP*
*PPPPP*PPPPPPP*PPP*P*
*PPP*PPPPP*PPPPPPPPP*
*PP*PPPPPPPPP*PPPP*P*
*PPPPPPPPP*PPPPPPPPP*
*PPP*PPPPP*PPP*PPP*P*
*P*PPPPPPPPP*PPPPPPP*
*PP**PPP*P**PPP**PPP*
*PPPPPPPPPPPPPPPPPPP*
**PPPPPPPPPP*PPPP*PP*
*PPPPPPPPPPPPPPPPPPP*
*P*PPPPPPPP*PPP*P*PP*
*PPPPPPP*PPPPPPPPPPP*
*********************
]]
,
[[
*********************
*PPPPPPPPPPPPPPPPPPP*
*PPPPP*PPPPPPP*PPP*P*
*PPPPPPPPPPPPPPP*PPP*
*PPP*PPPPPPPPPP*PPPP*
*P*PPPPPPPPPPPPPPPPP*
*PPPPPPPPPPP**PPPPP**
*PPPPP*P*PPP*PPPPPPP*
*PPPPP*PPPPPPP*PPP*P*
*PPP*PPPPP*PPPPPPPPP*
*PP*PPPPPPPPP*PPPP*P*
*PPPPPPPPP*PPPPPPPPP*
*PPP*PPPPP*PPP*PPP*P*
*P*PPPPPPPPP*PPPPPPP*
* P**PPP*P**PPP**PPP*
*  PPPPPPPPPPPPPPPPP*
**  PPPPPPPP*PPPP*PP*
*    PPPPPPPPPPPPPPP*
* *   PPPPP*PPP*P*PP*
*G     P*PPPPPPPPPPP*
*********************
]]
,
[[
*********************
*PPPPPPPPPPPPPPPPPPP*
*PPPPP*PPPPPPP*PPP*P*
*PPPPPPPPPPPPPPP*PPP*
*PPP*PPPPPPPPPP*PPPP*
*P*PPPPPPPPPPPPPPPPP*
*PPPPPPPPPPP**PPPPP**
*PPPPP*P*PPP*PPPPPPP*
*PPPPP*PPPPPPP*PPP*P*
*PPP*PPPPP*PPPPPPPPP*
*PP*PPPPPPPPP*PPPP*P*
*PPPPPPPPP*PPPPPPPPP*
*PPP*PPPPP*PP * P *P*
*P*PPPPPPPPP*      P*
*PP**PPP*P**   **   *
*PPPPPPPPP     G    *
**PPPPPPPPP *    *  *
*PPPPPPPPPPP       P*
*P*PPPPPPPP*P  * *PP*
*PPPPPPP*PPPPP P PPP*
*********************
]]
}


local GOAL = {
  name = 'Goal',
  classname = 'goal',
  model = 'models/goal_transparent.md3', -- use large model
  quantity = 10,
  type = pickups.type.GOAL
}

-- local TEXT_MAP = [[
-- *********************
-- *GPPPPPPPPPPPPPPPPPP*
-- *PPPPP*PPPPPPP*PPP*P*
-- *PPPPPPPPPPPPPPP*PPP*
-- *PPP*PPPPPPPPPP*PPPP*
-- *P*PPPPPPPPPPPPPPPPP*
-- *PPPPPPPPPPP**PPPPP**
-- *PPPPP*P*PPP*PPPPPPP*
-- *PPPPP*PPPPPPP*PPP*P*
-- *PPP*PPPPP*PPPPPPPPP*
-- *PP*PPPPPPPPP*PPPP*P*
-- *PPPPPPPPP*PPPPPPPPP*
-- *PPP*PPPPP*PP * P *P*
-- *P*PPPPPPPPP*      P*
-- *PP**PPP*P**   **   *
-- *PPPPPPPPP     G    *
-- **PPPPPPPPP *    *  *
-- *PPPPPPPPPPP       P*
-- *P*PPPPPPPP*P  * *PP*
-- *PPPPPPP*PPPPP P PPP*
-- *********************
-- ]]

-- Called only once at start up. Settings not recognised by DM Lab internal
-- are forwarded through the params dictionary.
-- function api:init(params)
--   -- Seed the map so only one map is created with lights and decals placed in
--   -- the same place each run.
  
--   api._has_goal = false


-- end
function api:init(params)
  -- Seed the map so only one map is created with lights and decals placed in
  -- the same place each run.
  make_map.random():seed(1)


  api._map = {
  make_map.makeMap{
      mapName = "openfield_map1",
      mapEntityLayer = TEXT_MAP[1],
      useSkybox = true,
      textureSet = texture_sets.TETRIS
  },
  make_map.makeMap{
    mapName = "openfield_map2",
    mapEntityLayer = TEXT_MAP[2],
    useSkybox = true,
    textureSet = texture_sets.TETRIS
  },
  make_map.makeMap{
    mapName = "openfield_map3",
    mapEntityLayer = TEXT_MAP[3],
    useSkybox = true,
    textureSet = texture_sets.TETRIS
  }
  }
end

function api:createPickup(classname)
  if classname=='goal' then
    return  GOAL
  end
  return pickups.defaults[classname]
end


function api:start(episode, seed, params)
  -- print("Start",episode, seed, params)
  -- random:seed(seed)
  -- randomMap:seed(random:mapGenerationSeed())


  api._has_goal = false
  api._count = 0
  api._finish_count = 0
end

function api:pickup(spawn_id)
  print('api:pickup:    ',spawn_id,'; Time: ',game:episodeTimeSeconds())
  api._count = api._count + 1
  if not api._has_goal and api._count == api._finish_count then
    game:finishMap()
    print('should not be')
  end
end
function api:hasEpisodeFinished(timeSeconds)
  return api._count>0
end
function api:updateSpawnVars(spawnVars)
  local classname = spawnVars.classname
  -- print('updateSpawnVars ',classname)
  -- if spawnVars.random_items then
  --   local possibleClassNames = helpers.split(spawnVars.random_items, ',')
  --   if #possibleClassNames > 0 then
  --     classname = possibleClassNames[
  --       random:uniformInt(1, #possibleClassNames)]
  --   end
  -- end
  local pickup = pickups.defaults[spawnVars.classname]
  if pickup then
    -- print('updateSpawnVars: pickup!')
    -- if pickup.type == pickups.type.REWARD and pickup.quantity > 0 then
    --   api._finish_count = api._finish_count + 1
    --   spawnVars.id = tostring(api._finish_count)
    -- end
    if pickup.type == pickups.type.GOAL then
      spawnVars.id = tostring(10)
      api._has_goal = true
    end
  end
  if classname == 'goal' then

    -- print('updateSpawnVars ',classname)
    -- api._has_goal = true
  end
  return spawnVars
end


-- On first call we return the name of the map. On subsequent calls we return
-- an empty string. This informs the engine to only perform a quik map restart
-- instead.
function api:nextMap()
  rand_num=2 -- random:uniformInt(1, 3)
  print("Map number:", rand_num)
  -- print(TEXT_MAP[rand_num])
  api.setInstruction(tostring(rand_num))
  local mapName = api._map[rand_num]
  -- api._map[rand_num] = ''
  return mapName
end

custom_observations.decorate(api)
setting_overrides.decorate{
    api = api,
    apiParams = {episodeLengthSeconds = 2 * 60, camera = {1050, 1050, 1000}},
    decorateWithTimeout = true
}
return api
