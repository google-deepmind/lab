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

local helpers = require 'common.helpers'
local make_map = require 'common.make_map'
local custom_observations = require 'decorators.custom_observations'
local events = require 'dmlab.system.events'
local game = require 'dmlab.system.game'
local timeout = require 'decorators.timeout'
local api = {}

local MAP_NO_WEAPON = [[
*********
*      P*
* ***** *
* ***** *
* ***** *
* ***** *
* ***** *
*P      *
*********
]]

function api:init(params)
  make_map.seedRng(1)
  api._map = make_map.makeMap{
      mapName = 'empty_room',
      mapEntityLayer = MAP_NO_WEAPON,
      useSkybox = true,
      allowBots = true
  }
  self._spawnWeapons = params.spawnWeapons
end

function api:nextMap()
  return self._map
end

function api:addBots()
  local bots = {
      {name = 'Cygni', skill = 5.0}
  }
  return bots
end

function api:extraEntities()
  if helpers.fromString(self._spawnWeapons) then
    -- List of entities to create.
    local vars = {
        {
            classname = 'weapon_rocketlauncher',
            origin = '750 150 30',
            spawnflags = "1"
        },
        {
            classname = 'weapon_lightning',
            origin = '150 750 30',
            spawnflags = "1"
        }
    }
    return vars
  else
    return nil
  end
end

function api:rewardOverride(args)
  if args.reason == "TAG_PLAYER" and args.playerId == 2 and
    args.otherPlayerId == 1 then
    events:add('PLAYER_TAGGED', 'Player tagged by bot')
  end
end

timeout.decorate(api, 60 * 60)
custom_observations.decorate(api)

return api
