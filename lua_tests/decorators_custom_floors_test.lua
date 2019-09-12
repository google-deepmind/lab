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

local asserts = require 'testing.asserts'
local test_runner = require 'testing.test_runner'

local custom_floors = require 'decorators.custom_floors'
local tensor = require 'dmlab.system.tensor'

local tests = {}

function tests.callsOriginalModifyTexture()
  local mock_api = {}
  function mock_api:modifyTexture(textureName, texture)
    -- Record that this was called
    mock_api.modifyTextureCalled = {textureName, texture}
  end

  custom_floors.decorate(mock_api)
  local NAME = 'some_texture_name'
  local TEXTURE = 'pretend_texture'
  mock_api:modifyTexture(NAME, TEXTURE)

  asserts.EQ(mock_api.modifyTextureCalled[1], NAME)
  asserts.EQ(mock_api.modifyTextureCalled[2], TEXTURE)
end

function tests.doesNothingToNonMatchingTexture()
  local fake_api = {}
  custom_floors.decorate(fake_api)

  local NAME = '/some/path/to/lg_floor_placeholder_B_d.tga'
  local EXPECT_WHITE = tensor.ByteTensor(4, 4, 4):fill(255)
  local texture = EXPECT_WHITE:clone()

  custom_floors.setVariationColor('A', {255, 0, 0})
  fake_api:modifyTexture(NAME, texture)
  assert(texture == EXPECT_WHITE)
end

function tests.appliesColorToMatchingTexture()
  local fake_api = {}
  custom_floors.decorate(fake_api)

  local NAME = '/some/path/to/lg_floor_placeholder_A_d.tga'
  local EXPECT_RED = tensor.ByteTensor(4, 4, 4):fill(255)
  EXPECT_RED:select(3, 2):fill(0)
  EXPECT_RED:select(3, 3):fill(0)
  local texture = tensor.ByteTensor(4, 4, 4):fill(255)

  custom_floors.setVariationColor('A', {255, 0, 0})
  fake_api:modifyTexture(NAME, texture)
  assert(texture == EXPECT_RED)
end

return test_runner.run(tests)
