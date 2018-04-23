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

-- Carries out transformations specified by common.human_recognisable_pickups.

local hrp = require 'common.human_recognisable_pickups'
local pickups = require 'common.pickups'
local model = require 'dmlab.system.model'
local transform = require 'common.transform'
local decorator = {}

function decorator.decorate(api)
  local replaceModelName = api.replaceModelName
  function api:replaceModelName(modelName)
    local replace = replaceModelName and {replaceModelName(self, modelName)}
      or {nil}
    if replace[1] == nil then
      replace = {hrp.replaceModelName(modelName)}
    end
    return unpack(replace)
  end

  local function buildTransform(transformSuffix)
    -- Matches name{amount}.
    local name, amount = string.match(transformSuffix, '(.*){(.*)}')
    assert(name == 'scale', 'Only scale operation supported')
    amount = tonumber(amount)
    assert(name ~= nil, 'Amount must be a number')
    return transform.scale({amount, amount, amount})
  end

  local createModel = api.createModel
  function api:createModel(modelName)
    -- Matches prefix%transformSuffix.
    local prefix, transformSuffix = string.match(modelName, '(.+)%%(.+)')
    if transformSuffix ~= nil then
      local modelNameActual, prefix = self:replaceModelName(prefix)
      if modelNameActual ~= nil then
        local modelRaw = model:loadMD3(modelNameActual)
        -- Custom loaded models must have their textures updated with the
        -- correct prefix.
        for _, v in pairs(modelRaw.surfaces) do
          v.shaderName = prefix .. v.shaderName
        end
        return model:hierarchy{
            transform = buildTransform(transformSuffix),
            model = modelRaw
        }
      end
    end
    return createModel and createModel(self, modelName)
  end

  local replaceTextureName = api.replaceTextureName
  function api:replaceTextureName(textureName)
    return replaceTextureName and replaceTextureName(self, textureName) or
      hrp.replaceTextureName(textureName)
  end

  local modifyTexture = api.modifyTexture
  function api:modifyTexture(textureName, texture)
    local res = false
    if modifyTexture then
      res = modifyTexture(self, textureName, texture)
    end
    return hrp.modifyTexture(textureName, texture) or res
  end

  local createPickup = api.createPickup
  function api:createPickup(classname)
    return hrp.pickup(classname) or
           (createPickup and createPickup(self, classname)) or
           pickups.defaults[classname]
  end
end

return decorator
