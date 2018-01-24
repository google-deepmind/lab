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

local map_maker = require 'dmlab.system.map_maker'
local random = require 'common.random'
local randomMap = random(map_maker:randomGen())

local themes = {}

function themes.fromTextureSet(opts)
  assert(opts.textureSet, "Must supply a textureSet")
  local ts = opts.textureSet
  local decalFrequency = opts.decalFrequency or 0.1
  local floorModelFrequency = opts.floorModelFrequency or 0.05
  local theme = {}
  local themeVariation = {}
  local riser = ((ts.riser and ts.riser[1])
                 or {tex = 'map/lab_games/lg_style_02_wall_blue'})
  local tread = ((ts.tread and ts.tread[1])
                 or {tex = 'map/black_d', width = 64, height = 64})
  themeVariation.default = {
      floor = ts.floor[1],
      ceiling = ts.ceiling[1],
      wallN = ts.wall[1],
      wallE = ts.wall[1],
      wallS = ts.wall[1],
      wallW = ts.wall[1],
      riser = riser,
      tread = tread,
  }

  local wallDecorations = {}
  if ts.wallDecals then
    for i = 1, #ts.wallDecals do
      wallDecorations[#wallDecorations + 1] = ts.wallDecals[i]
    end
  end

  local function variationSet(ts, variation, property)
    return ts.variations and ts.variations[variation] and
        ts.variations[variation][property] or ts[property]
  end

  function theme:mazeVariation(variation)
    if not themeVariation[variation] then
      local wall = randomMap:choice(variationSet(ts, variation, 'wall'))
      themeVariation[variation] = {
          floor = randomMap:choice(variationSet(ts, variation, 'floor')),
          ceiling = randomMap:choice(variationSet(ts, variation, 'ceiling')),
          wallN = wall,
          wallE = wall,
          wallS = wall,
          wallW = wall,
      }
    end
    return themeVariation[variation]
  end

  -- Only create function if it will return anything.
  if decalFrequency > 0 and ts.wallDecals and #ts.wallDecals > 0 then
    function theme:placeWallDecals(allWallLocations)
      local decorationCount = math.floor(math.min(0.5 +
          decalFrequency * #allWallLocations, #ts.wallDecals))
      local wallDecals = {}
      if decorationCount > 0 then
        randomMap:shuffleInPlace(allWallLocations)
        local wallDecalsShuffled = randomMap:shuffle(ts.wallDecals)
        for i = 1, decorationCount do
          wallDecals[i] = {
              index = allWallLocations[i].index,
              decal = wallDecalsShuffled[i],
          }
        end
      end
      return wallDecals
    end
  end

  -- Only create function if it will return anything.
  if floorModelFrequency > 0 and ts.floorModels and #ts.floorModels > 0 then
    function theme:placeFloorModels(allFloorLocations)
      local decorationCount = math.floor(floorModelFrequency *
                                         #allFloorLocations)
      local floorModels = {}
      if decorationCount > 0 and #ts.floorModels > 0 then
        randomMap:shuffleInPlace(allFloorLocations)
        for i = 1, decorationCount do
          floorModels[i] = {
              index = allFloorLocations[i].index,
              model = randomMap:choice(ts.floorModels)
          }
        end
      end
      return floorModels
    end
  end

  return theme
end

return themes
