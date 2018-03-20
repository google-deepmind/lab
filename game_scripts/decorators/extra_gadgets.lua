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

-- Provides replacement models and textures for missing gadgets.

local TEXTURE_REPLACE = {
    ['sprites/plasmaa.tga'] = 'textures/model/gen_impact.tga',
}

local EMPTY_MODEL = 'models/powerups/health/large_sphere.md3'

local MODEL_REPLACE = {
    -- Health --
    ['models/powerups/health/small_cross.md3'] =
      'models/fut_obj_barbell_01.md3',
    ['models/powerups/health/small_sphere.md3'] = EMPTY_MODEL,
    ['models/powerups/health/medium_cross.md3'] =
      'models/fut_obj_barbell_02.md3',
    ['models/powerups/health/medium_sphere.md3'] = EMPTY_MODEL,
    ['models/powerups/health/mega_cross.md3'] = 'models/fut_obj_barbell_03.md3',
    ['models/powerups/health/mega_sphere.md3'] = EMPTY_MODEL,
    -- Armour --
    ['models/powerups/armor/armor_red.md3'] = 'models/fut_obj_cone_01.md3',
    ['models/powerups/armor/shard.md3'] = 'models/fut_obj_cone_02.md3',
    ['models/powerups/armor/shard_sphere.md3'] = EMPTY_MODEL,
    -- Weapons --
    ['models/weapons2/grenadel/grenadel.md3'] = 'models/fut_obj_sphere_01.md3',
    ['models/weapons2/grenadel/grenadel_flash.md3'] = EMPTY_MODEL,
    ['models/weapons2/plasma/plasma.md3'] = 'models/fut_obj_crossbar_01.md3',
    ['models/weapons2/plasma/plasma_flash.md3'] = EMPTY_MODEL,
    ['models/weapons2/shotgun/shotgun.md3'] =
      'models/fut_obj_doubleprism_01.md3',
    ['models/weapons2/shotgun/shotgun_flash.md3'] = EMPTY_MODEL,
    -- Ammo --
    ['models/powerups/ammo/grenadeam.md3'] = 'models/fut_obj_sphere_03.md3',
    ['models/powerups/ammo/plasmaam.md3'] = 'models/fut_obj_crossbar_03.md3',
    ['models/powerups/ammo/shotgunam.md3'] =
      'models/fut_obj_doubleprism_03.md3',
    ['models/ammo/grenade1.md3'] = 'models/fut_obj_sphere_02.md3',
    ['models/weapons2/shells/m_shell.md3'] = EMPTY_MODEL,
    ['models/weapons2/shells/s_shell.md3'] = EMPTY_MODEL,
    -- Powerups --
    ['models/powerups/holdable/medkit.md3'] = 'models/fut_obj_cube_03.md3',
    ['models/powerups/holdable/medkit_sphere.md3'] = EMPTY_MODEL,
    ['models/powerups/instant/quad.md3'] = 'models/fut_obj_glowball_01.md3',
    ['models/powerups/instant/quad_ring.md3'] = EMPTY_MODEL,
    ['models/powerups/instant/regen.md3'] = 'models/fut_obj_prismjack_02.md3',
    ['models/powerups/instant/regen_ring.md3'] = EMPTY_MODEL,
}

local function decorate(api)
  assert(not api._extraGadgetsDecorated, "extra_gadgets already decorated")
  api._extraGadgetsDecorated = true
  local replaceTextureName = api.replaceTextureName
  function api:replaceTextureName(name)
    return replaceTextureName and replaceTextureName(self, name) or
      TEXTURE_REPLACE[name]
  end

  local replaceModelName = api.replaceModelName
  function api:replaceModelName(modelName)
    return replaceModelName and replaceModelName(self, modelName) or
      MODEL_REPLACE[modelName]
  end
end

return {decorate = decorate}
