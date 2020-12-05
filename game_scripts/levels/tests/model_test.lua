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
