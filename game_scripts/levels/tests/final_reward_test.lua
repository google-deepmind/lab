local game = require 'dmlab.system.game'

local api = {}

function api:nextMap()
  api._tick = 0
  return 'lookat_test'
end

function api:hasEpisodeFinished(timeSeconds)
  api._tick = api._tick + 1
  game:addScore(1, api._tick)
  return api._tick >= 10
end

return api
