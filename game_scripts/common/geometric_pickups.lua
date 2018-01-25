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

local random = require 'common.random'
local pickups = require 'common.pickups'

local geometric_pickups = {}

-- Matches the 'fut_obj_*' models available in assets/models.
local OBJECTS = {
    'fut_obj_barbell_',
    'fut_obj_coil_',
    'fut_obj_cone_',
    'fut_obj_crossbar_',
    'fut_obj_cube_',
    'fut_obj_cylinder_',
    'fut_obj_doubleprism_',
    'fut_obj_glowball_',
    'fut_obj_potcone_',
    'fut_obj_prismjack_',
    'fut_obj_sphere_',
    'fut_obj_toroid_',
}

local OBJECT_MODS = 3
local EAT_REWARD_MIN = -1
local EAT_REWARD_MAX = 1

-- Returns a table with the names of all of the available pickups.
function geometric_pickups.createPickupNames()
  local pickupNames = {}

  for i = 1, #OBJECTS do
    for j = 1, OBJECT_MODS do
      pickupNames[#pickupNames + 1] = OBJECTS[i] .. string.format('%02d', j)
    end
  end

  return pickupNames
end

-- Returns a table with all available objects as pickups (see common.pickups).
function geometric_pickups.createPickups()
  local geoPickups = {}

  local pickupNames = geometric_pickups.createPickupNames()
  for k, v in ipairs(pickupNames) do
    geoPickups[v] = {
        name = v,
        classname = v,
        model = 'models/' .. v .. '.md3',
        quantity = 0,
        type = pickups.type.REWARD
    }
  end

  return geoPickups
end

-- Assigns random rewards (within supplied inclusive range) to pickups table.
function geometric_pickups.randomisePickupRewards(
    pickupTable, minReward, maxReward)
  minReward = minReward or EAT_REWARD_MIN
  maxReward = maxReward or EAT_REWARD_MAX
  assert(minReward <= maxReward)

  for k, v in pairs(pickupTable) do
    v.quantity = random:uniformInt(0, maxReward - minReward) + minReward
  end
end

--[[ Returns a table with the requested number of pickup chosen from the list of
possible pickups. Returned table contains unique elements unless the number of
elements requested is greater than the size of the available elements, then
elements will be repeated.
]]
function geometric_pickups.randomUniquePickupNames(numPickups)
  local pickupNames = geometric_pickups.createPickupNames()
  assert(0 <= numPickups, "Invalid numPickups!")
  random:shuffleInPlace(pickupNames, numPickups)
  local randElements = {}
  for i = 1, numPickups do
    randElements[i] = pickupNames[(i - 1) % #pickupNames + 1]
  end
  return randElements
end

--[[ Returns a shuffled list of pickups. List contains 'setSize' copies of
'numTypes' types of object, selected at random from 'allEdibles'.
]]
function geometric_pickups.randomPickups(allEdibles, numTypes, setSize)
  local pickupKeys = geometric_pickups.randomUniquePickupNames(numTypes)
  local pickupsList = {}

  for j = 1, #pickupKeys do
    local obj_type = pickupKeys[j]
    for i = 1, setSize do
      pickupsList[#pickupsList + 1] = obj_type
    end
  end
  random:shuffleInPlace(pickupsList)

  return pickupsList
end

return geometric_pickups
