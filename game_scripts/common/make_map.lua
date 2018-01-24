local map_maker = require 'dmlab.system.map_maker'
local random = require 'common.random'
local themes = require 'themes.themes'
local texture_sets = require 'themes.texture_sets'
local make_map = {}

local PICKUPS = {
    A = 'apple_reward',
    G = 'goal',
}

local SKYBOX_TEXTURE_NAME = 'map/lab_games/sky/lg_sky_03'

function make_map.makeMap(kwargs)
  assert(kwargs.mapName)
  local skyboxTextureName = nil
  if kwargs.useSkybox then
    skyboxTextureName = SKYBOX_TEXTURE_NAME
  end
  map_maker:mapFromTextLevel{
      entityLayer = kwargs.mapEntityLayer,
      variationsLayer = kwargs.mapVariationsLayer,
      mapName = kwargs.mapName,
      allowBots = kwargs.allowBots,
      skyboxTextureName = skyboxTextureName,
      theme = kwargs.theme or themes.fromTextureSet{
          textureSet = kwargs.textureSet or texture_sets.MISHMASH,
          decalFrequency = kwargs.decalFrequency,
          floorModelFrequency = kwargs.floorModelFrequency,
      },
      callback = kwargs.callback or function(i, j, c, maker)
        local pickup = kwargs.pickups and kwargs.pickups[c] or PICKUPS[c]
        if pickup then
          return maker:makeEntity{
              i = i,
              j = j,
              classname = pickup,
          }
        end
      end
  }
  return kwargs.mapName
end

function make_map.seedRng(value)
  map_maker:randomGen():seed(value)
end

function make_map.random()
  return random(map_maker:randomGen())
end

return make_map
