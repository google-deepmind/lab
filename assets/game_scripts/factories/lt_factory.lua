local random = require 'common.random'
local custom_observations = require 'decorators.custom_observations'
local timeout = require 'decorators.timeout'

local BOT_NAMES_COLOR = {
    'CygniColor',
    'LeonisColor',
    'EpsilonColor',
    'CepheiColor',
    'CentauriColor',
    'DraconisColor'
}

local BOT_NAMES = {
    'Cygni',
    'Leonis',
    'Epsilon',
    'Cephei',
    'Centauri',
    'Draconis'
}

--[[ Converts an HSV color value to RGB. Conversion formula adapted from
http://en.wikipedia.org/wiki/HSV_color_space. Assumes h, s and v are contained
in the set [0, 1]. Returns r, g, b each in the set [0, 255].
]]
local function hsvToRgb(h, s, v)
  local r, g, b

  local i = math.floor(h * 6);
  local f = h * 6 - i;
  local p = v * (1 - s);
  local q = v * (1 - f * s);
  local t = v * (1 - (1 - f) * s);

  i = i % 6

  if i == 0 then r, g, b = v, t, p
  elseif i == 1 then r, g, b = q, v, p
  elseif i == 2 then r, g, b = p, v, t
  elseif i == 3 then r, g, b = p, q, v
  elseif i == 4 then r, g, b = t, p, v
  elseif i == 5 then r, g, b = v, p, q
  end

  return r * 255, g * 255, b * 255
end

local SATURATION = 1.0
local VALUE = 1.0

local factory = {}

--[[ Creates a Laser tag API.
Keyword arguments:

*   `mapName` (string) - Name of map to load.
*   `botCount` (number, [-1, 6], default 4) - Number of bots. (-1 for all).
*   `skill` (number, [1.0, 5.0], default 4.0) - Skill level of bot.
*   `episodeLengthSeconds` (number, default 600) - Episode length in seconds.
*   `color` (boolean, default false) - Change color of bots each episode.
]]
function factory.createLevelApi(kwargs)
  assert(kwargs.mapName)
  kwargs.botCount = kwargs.botCount or 4
  kwargs.skill = kwargs.skill or 4.0
  kwargs.episodeLengthSeconds = kwargs.episodeLengthSeconds or 600
  kwargs.color = kwargs.color or false
  assert(kwargs.botCount <= (kwargs.color and #BOT_NAMES_COLOR or #BOT_NAMES))
  local api = {}

  function api:nextMap()
    return kwargs.mapName
  end

  function api:start(episode, seed, params)
    random.seed(seed)
    if kwargs.color then
      -- Pick a random angle.
      api.bot_hue_degrees_ = random.uniformInt(0, 359)
    end
  end

  if kwargs.color then
    function api:modifyTexture(name, tensor)
      if string.match(name, 'players/crash_color/dm_character_skin_mask') then

        local hue =  api.bot_hue_degrees_

        -- Based on the mask name, determine the hue using a triad distribution.
        if string.sub(name, -string.len('mask_a.tga')) == 'mask_a.tga' then
          hue = hue / 360.0
        elseif string.sub(name, -string.len('mask_b.tga')) == 'mask_b.tga' then
          hue = ((hue + 120) % 360) / 360.0
        elseif string.sub(name, -string.len('mask_c.tga')) == 'mask_c.tga' then
          hue = ((hue + 240) % 360) / 360.0
        else
          logging.raiseError('Unrecognised mask: ' .. name)
          return
        end

        local r, g, b = hsvToRgb(hue, SATURATION, VALUE)
        tensor:select(3, 1):fill(r)
        tensor:select(3, 2):fill(g)
        tensor:select(3, 3):fill(b)
      end
    end
  end


  function api:addBots()
    local bots = {}
    for i, name in ipairs(kwargs.color and BOT_NAMES_COLOR or BOT_NAMES) do
      if i == kwargs.botCount + 1 then
        break
      end
      bots[#bots + 1] = {name = name, skill = kwargs.skill}
    end
    return bots
  end

  custom_observations.decorate(api)
  timeout.decorate(api, kwargs.episodeLengthSeconds)
  return api
end

return factory
