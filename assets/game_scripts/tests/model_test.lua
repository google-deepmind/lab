-- Tested in deepmind/model_generation/lua_model_test.cc.
local model = require 'dmlab.system.model'
local transform = require 'common.transform'

local api = {}

function api:createModel(modelName)
  local models = {
      cone = model:cone{
        phiSegments = 4,
        radiusSegments = 4,
        heightSegments = 4,
        shaderName = 'textures/model/beam'
      },
      cube = model:cube{
        segments = 4,
        shaderName = 'textures/model/beam'
      },
      cylinder = model:cylinder{
        phiSegments = 4,
        radiusSegments = 4,
        heightSegments = 4,
        shaderName = 'textures/model/beam'
      },
      sphere = model:sphere{
        phiSegments = 4,
        thetaSegments = 4,
        shaderName = 'textures/model/beam'
      },
      hierarchy = model:hierarchy{
          transform = transform.rotateX(180),
          model = model:cone{
              radius = 10,
              height = 20,
              shaderName = 'textures/model/apple_d'
          },
          children = {
              centre_bottom_centre_p = {
                  model = model:sphere{
                      radius = 8,
                      shaderName = 'textures/model/pig_d'
                  },
                  children = {
                      centre_top_left_p = {
                          locator = 'centre_bottom_centre_s',
                          model = model:cylinder{
                              radius = 0.5,
                              height = 5,
                              shaderName = 'textures/model/cherry_d'
                          },
                          children = {
                              centre_top_centre_p = {
                                  locator = 'centre_bottom_centre_s',
                                  model = model:cone{
                                      radius = 3,
                                      height = 2,
                                      shaderName = 'textures/model/apple_d'
                                  }
                              }
                          }
                      }
                  }
              }
          }
      }
  }
  return models[modelName]
end

return api
