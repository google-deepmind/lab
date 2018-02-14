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
local screen_message = require 'common.screen_message'

local timeout = {}

--[[ Function for displaying a timer in the top right of the screen.

Arguments:

*   'args' (table) Passed through from api:screenMessages(args).
*   'timeSeconds' (number or boolean). Rounded up and if greater than 60, then
    minutes are displayed. Set to false to disable.

Returns:

*   A table, suitable for use as an entry in the array returned by
    api:screenMessages.
]]
local function timeDisplay(args, timeSeconds)
  return {
      message = helpers.secondsToTimeString(timeSeconds),
      x = args.width - screen_message.BORDER_SIZE,
      y = 0,
      alignment = screen_message.ALIGN_RIGHT
  }
end

local timeRemaining = 0.0

function timeout.timeRemainingSeconds()
  return timeRemaining
end

-- Adds a timeout to the level and displays a timer in top right of the screen.
-- 'episodeLength' is the episode length in seconds.
function timeout.decorate(api, episodeLength)
  if episodeLength then
    local start = api.start
    function api:start(...)
      timeRemaining = episodeLength
      return start and start(api, ...)
    end

    local screenMessages = api.screenMessages
    function api:screenMessages(args)
      local messages = screenMessages and screenMessages(api, args) or {}
      messages[#messages + 1] = timeDisplay(args, timeRemaining)
      return messages
    end

    local hasEpisodeFinished = api.hasEpisodeFinished
    function api:hasEpisodeFinished(timeSeconds)
      timeRemaining = episodeLength - timeSeconds
      return hasEpisodeFinished and hasEpisodeFinished(api, timeSeconds) or
             timeRemaining <= 0
    end
  else
    local hasEpisodeFinished = api.hasEpisodeFinished
    function api:hasEpisodeFinished(timeSeconds)
      return hasEpisodeFinished and hasEpisodeFinished(api, timeSeconds) or
             false
    end
  end
end

return timeout
