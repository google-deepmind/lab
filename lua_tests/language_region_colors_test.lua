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
local region_colors = require 'language.region_colors'

local set = require 'common.set'

local tests = {}

function tests.shuffledFloors_shouldFailIfNoRegionsDefined()
  local colorer = region_colors.createShuffledFloors({})
  asserts.shouldFail(function () colorer() end)
end

function tests.shuffledFloors_shouldFailIfMoreRegionsThanColors()
  local colorer = region_colors.createShuffledFloors({'color1'})
  asserts.shouldFail(function () colorer{regions = {'region1', 'region2'}} end)
end

function tests.shuffledFloors_shouldMapSingleColorToSingleRegion()
  local COLOR = 'color1'
  local REGION = 'region1'
  local EXPECTATION = {floor = COLOR}

  local colorer = region_colors.createShuffledFloors{COLOR}
  local result = colorer{regions = {REGION}}
  asserts.tablesEQ(result[REGION], EXPECTATION)
end

function tests.shuffledFloors_shouldReturnResultsForAllRegions()
  local COLORS = {'color1', 'color2', 'color3'}
  local REGIONS = {'regionA', 'regionB', 'regionC'}

  local colorer = region_colors.createShuffledFloors(COLORS)
  local result = colorer{regions = REGIONS}

  for _, region in ipairs(REGIONS) do
    assert(result[region])
  end
end

function tests.shuffledFloors_shouldReturnValuesFromGivenColors()
  local COLORS = {'color1', 'color2', 'color3', 'color4'}
  local REGIONS = {'regionA', 'regionB', 'regionC'}
  local COLOR_SET = set.Set(COLORS)

  local colorer = region_colors.createShuffledFloors(COLORS)
  local result = colorer{regions = REGIONS}

  for _, region in ipairs(REGIONS) do
    assert(COLOR_SET[result[region].floor])
  end
end

return test_runner.run(tests)
