--[[ Copyright (C) 2017-2019 Google Inc.

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

local test_runner = require 'testing.test_runner'
local asserts = require 'testing.asserts'
local themes = require 'themes.themes'
local texture_sets = require 'themes.texture_sets'
local map_maker = require 'dmlab.system.map_maker'
local random = require 'common.random'
local mapRandomGen = map_maker:randomGen()

local tests = {}

function tests.TextureSetMishMash()
  for otherSeed = 1, 3 do
    random:seed(otherSeed) -- Make sure other seed does not affect results.
    mapRandomGen:seed(2)
    local mishmash = texture_sets.MISHMASH
    local theme = themes.fromTextureSet{
        textureSet = mishmash,
        decalFrequency = 0.1
    }

    local vDefault = theme:mazeVariation('default')
    asserts.EQ(mishmash.floor[1].tex, vDefault.floor.tex)
    asserts.EQ(mishmash.ceiling[1].tex,
               vDefault.ceiling.tex)
    asserts.EQ(mishmash.wall[1].tex, vDefault.wallN.tex)
    asserts.EQ(mishmash.wall[1].tex, vDefault.wallE.tex)
    asserts.EQ(mishmash.wall[1].tex, vDefault.wallS.tex)
    asserts.EQ(mishmash.wall[1].tex, vDefault.wallW.tex)

    local locs = {}
    for i = 1, 100 do
      locs[i] = {index = i}
    end
    assert(theme.placeWallDecals)
    local wallDecals = theme:placeWallDecals(locs)
    asserts.GE(#mishmash.wallDecals, 10)
    -- 'decalFrequency' is 0.1 and there are 100 locations.
    asserts.EQ(#wallDecals, 10)
    assert(theme.placeFloorModels == nil)
  end
end

function tests.TextureSetCustom()
  for otherSeed = 1, 3 do
    random:seed(otherSeed) -- Make sure other seed does not affect results.
    mapRandomGen:seed(2)
    local textureSet = {
        floor = {{tex = 'floor'}},
        ceiling = {{tex = 'ceiling'}},
        wall = {{tex = 'wall'}},
        wallDecals = {{tex = 'wallDecals'}},
        floorModels = {{mod = 'floorModels'}},
    }
    local theme = themes.fromTextureSet{textureSet = textureSet}

    local vDefault = theme:mazeVariation('default')
    asserts.EQ('floor', vDefault.floor.tex)
    asserts.EQ('ceiling', vDefault.ceiling.tex)
    asserts.EQ('wall', vDefault.wallN.tex)
    asserts.EQ('wall', vDefault.wallE.tex)
    asserts.EQ('wall', vDefault.wallS.tex)
    asserts.EQ('wall', vDefault.wallW.tex)

    local vA = theme:mazeVariation('A')
    asserts.EQ('floor', vA.floor.tex)
    asserts.EQ('ceiling', vA.ceiling.tex)
    asserts.EQ('wall', vA.wallN.tex)
    asserts.EQ('wall', vA.wallE.tex)
    asserts.EQ('wall', vA.wallS.tex)
    asserts.EQ('wall', vA.wallW.tex)

    local locs = {}
    for i = 1, 100 do
      locs[i] = {index = i}
    end
    assert(theme.placeWallDecals)
    local wallDecals = theme:placeWallDecals(locs)
    asserts.EQ(#wallDecals, 1)
    assert(theme.placeFloorModels)
    local floorModels = theme:placeFloorModels(locs)
    asserts.EQ(#floorModels, 5)
  end
end

return test_runner.run(tests)
