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
local colors = require 'common.colors'

local tests = {}


function tests.hsl_brightRed()
  asserts.tablesEQ({colors.hslToRgb(0, 1, 0.5)}, {255, 0, 0})
  asserts.tablesEQ({colors.rgbToHsl(255, 0, 0)}, {0, 1, 0.5})
end

function tests.hsl_darkRed()
  asserts.tablesEQ({colors.hslToRgb(0, 1, 0.25)}, {127.5, 0, 0})
  asserts.tablesEQ({colors.rgbToHsl(127.5, 0, 0)}, {0, 1, 0.25})
end

function tests.hsl_lightRed()
  asserts.tablesEQ({colors.hslToRgb(0, 1, 0.75)}, {255, 127.5, 127.5})
  asserts.tablesEQ({colors.rgbToHsl(255, 127.5, 127.5)}, {0, 1, 0.75})
end

function tests.hsl_brightGreen()
  asserts.tablesEQ({colors.hslToRgb(120, 1, 0.5)}, {0, 255, 0})
  asserts.tablesEQ({colors.rgbToHsl(0, 255, 0)}, {120, 1, 0.5})
end

function tests.hsl_brightBlue()
  asserts.tablesEQ({colors.hslToRgb(240, 1, 0.5)}, {0, 0, 255})
  asserts.tablesEQ({colors.rgbToHsl(0, 0, 255)}, {240, 1, 0.5})
end

function tests.hsl_blueGoesblackIfZeroL()
  asserts.tablesEQ({colors.hslToRgb(240, 1, 0)}, {0, 0, 0})
end

function tests.hsl_redGoesBlackIfZeroL()
  asserts.tablesEQ({colors.hslToRgb(0, 1, 0)}, {0, 0, 0})
end

function tests.hsl_redGoesWhiteIfLightness1()
  asserts.tablesEQ({colors.hslToRgb(0, 1, 1)}, {255, 255, 255})
end

function tests.hsl_increasingLightnessMovesTowardsWhite()
  local h, s, l = 30, 0.5, 0
  local rLast, gLast, bLast = colors.hslToRgb(h, s, l)
  local deltaL = 0.1
  for l = l + deltaL, 1, deltaL do
    local r, g, b = colors.hslToRgb(h, s, l)
    asserts.GE(r, rLast)
    asserts.GE(g, gLast)
    asserts.GE(b, bLast)
    rLast, gLast, bLast = r, g, b
  end
end

function tests.hsv_brightRed()
  asserts.tablesEQ({colors.hsvToRgb(0, 1, 1)}, {255, 0, 0})
end

function tests.hsv_brightGreen()
  asserts.tablesEQ({colors.hsvToRgb(120, 1, 1)}, {0, 255, 0})
end

function tests.hsv_darkGreen()
  asserts.tablesEQ({colors.hsvToRgb(120, 1, 0.5)}, {0, 127.5, 0})
end

function tests.hsv_lightGreen()
  asserts.tablesEQ({colors.hsvToRgb(120, 0.5, 1)}, {127.5, 255, 127.5})
end

function tests.hsv_brightBlue()
  asserts.tablesEQ({colors.hsvToRgb(240, 1, 1)}, {0, 0, 255})
end

function tests.hsv_reducingVNeverIncreasesRGB()
  local h, s, v = 30, 0.5, 1
  local rLast, gLast, bLast = colors.hsvToRgb(h, s, v)
  local deltaV = 0.05
  for v = v - deltaV, 0, -deltaV do
    local r, g, b = colors.hsvToRgb(h, s, v)
    asserts.LE(r, rLast)
    asserts.LE(g, gLast)
    asserts.LE(b, bLast)
    rLast, gLast, bLast = r, g, b
  end
end

return test_runner.run(tests)
