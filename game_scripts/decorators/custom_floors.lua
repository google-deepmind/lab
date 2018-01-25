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

--[[ Provide simple re-coloring of floor textures.

Use along with a theme using the CUSTOMIZABLE_FLOORS texture set:

```
local theme = themes.fromTextureSet{
    textureSet = texture_sets.CUSTOMIZABLE_FLOORS,
    randomizeFloorTextures = false,
}
```

RGB values floor variations can be specified via setVariationColor() and will
be automatically applied.  Variation 0 represents the default corridor.
]]
local decorator = {}

local colorMappings = {}

function decorator.setVariationColor(variation, color)
  assert(#color == 3, 'Expected RGB color')
  colorMappings[variation] = color
end

function decorator.setVariationColors(variationToColor)
  colorMappings = {}
  for variation, color in pairs(variationToColor) do
    decorator.setVariationColor(variation, color)
  end
end

function decorator.decorate(api)
  local modifyTexture = api.modifyTexture
  function api:modifyTexture(textureName, texture)
    local res = false
    if modifyTexture then
      res = modifyTexture(self, textureName, texture)
    end

    local variation = textureName:match('lg_floor_placeholder_(.)_d%.tga')
    local color = variation and colorMappings[variation]
    if color then
      texture:select(3, 1):mul(color[1] / 255)
      texture:select(3, 2):mul(color[2] / 255)
      texture:select(3, 3):mul(color[3] / 255)
      return true
    end
    return res
  end
end

return decorator
