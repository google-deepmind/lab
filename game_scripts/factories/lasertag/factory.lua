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

local color_bots = require 'common.color_bots'
local colors = require 'common.colors'
local custom_observations = require 'decorators.custom_observations'
local random = require 'common.random'
local setting_overrides = require 'decorators.setting_overrides'

local factory = {}

--[[ Creates a Laser tag API.

Keyword arguments:

*   `mapName` (string) - Name of map to load.
*   `botCount` (number, [-1, 6]) - Number of bots. (-1 for all).
*   `skill` (number, [1.0, 5.0], default 4.0) - Skill level of bot.
*   `episodeLengthSeconds` (number, default 600) - Episode length in seconds.
*   `color` (boolean, default false) - Change color of bots each episode.
]]
function factory.createLevelApi(kwargs)
  assert(kwargs.botCount, "must supply botCount")
  kwargs.skill = kwargs.skill or 4.0
  kwargs.episodeLengthSeconds = kwargs.episodeLengthSeconds or 600
  kwargs.color = kwargs.color or false
  assert(kwargs.botCount <= #color_bots.BOT_NAMES)
  local api = {}

  function api:nextMap()
    local map = kwargs.mapName
    kwargs.mapName = ''
    return map
  end

  function api:start(episode, seed, params)
    random:seed(seed)
    if kwargs.color then
      -- Pick a random angle.
      self._botHueDegrees = random:uniformInt(0, 359)
    end
  end

  if kwargs.color then
    function api:modifyTexture(name, skin)
      return color_bots:modifySkin(name, skin, self._botHueDegrees)
    end

    function api:mapLoaded()
      color_bots:colorizeBots(self._botHueDegrees)
    end
  end

  function api:addBots()
    return color_bots:makeBots{
        count = kwargs.botCount,
        color = kwargs.color,
        skill = kwargs.skill,
    }
  end

  custom_observations.decorate(api)
  setting_overrides.decorate{
      api = api,
      apiParams = kwargs,
      decorateWithTimeout = true
  }
  return api
end

return factory
