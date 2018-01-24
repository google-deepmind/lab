local make_map = require 'common.make_map'
local pickups = require 'common.pickups'
local api = {}

function api:start(episode, seed)
  make_map.seedRng(seed)
  api._count = 0
end

function api:createPickup(className)
  return pickups.defaults[className]
end

function api:nextMap()
  map = "G I A P"
  api._count = api._count + 1
  for i = 0, api._count do
    map = map.." A"
  end
  return make_map.makeMap{
      mapName = "demo_map_" .. api._count,
      mapEntityLayer = map,
  }
end

return api
