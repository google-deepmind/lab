local screen_message = require 'common.screen_message'

local timeout = {}

-- Function for displaying a timer in the top right of the screen.
-- 'args' is a table which can be passed through from api:screenMessages(args).
--    'width' (number) Screen width.
-- 'time_seconds' (number). Rounded up and if greater than 60, then minutes are
--     displayed, too.
-- Returns
--    A table, suitable for use as an entry in the array returned by
--        api:screenMessages
local function timeDisplay(args, time_seconds)
  local s = math.ceil(time_seconds)
  local time_remaining
  if s < 60 then
    time_remaining = string.format('%.2d', s % 60)
  else
    time_remaining = string.format('%.2d:%.2d', s / 60 % 60, s % 60)
  end
  return {
      message = time_remaining,
      x = args.width - screen_message.kBorderSize,
      y = 0,
      alignment = screen_message.kAlignRight
  }
end

-- Adds a timeout to the level and displays a timer in top right of the screen.
-- 'episodeLength' is the episode length in seconds.
function timeout.decorate(api, episodeLength)
  local timeRemaining

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
  function api:hasEpisodeFinished(time_seconds)
    timeRemaining = episodeLength - time_seconds
    return hasEpisodeFinished and hasEpisodeFinished(api, time_seconds) or
           timeRemaining <= 0
  end
end

return timeout
