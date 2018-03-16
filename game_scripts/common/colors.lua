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

-- Utilities for color conversion.

local colors = {}

--[[ Converts an HSV color value to RGB.

Conversion formula adapted from
http://en.wikipedia.org/wiki/HSV_color_space.

Arguments:

*   'h' Hue must be in range [0, 360)
*   's' Saturation must be in range [0, 1] (default 1)
*   'v' Value must be in range [0, 1] (default 1)

Returns r, g, b each in the range [0, 255].
]]
function colors.hsvToRgb(h, s, v)
  s = s or 1
  v = v or 1
  local i = math.floor(h / 60)
  local f = h / 60 - i
  local p = v * (1 - s)
  local q = v * (1 - f * s)
  local t = v * (1 - (1 - f) * s)

  i = i % 6
  local r, g, b
  if i == 0 then r, g, b = v, t, p
  elseif i == 1 then r, g, b = q, v, p
  elseif i == 2 then r, g, b = p, v, t
  elseif i == 3 then r, g, b = p, q, v
  elseif i == 4 then r, g, b = t, p, v
  elseif i == 5 then r, g, b = v, p, q
  end

  return r * 255, g * 255, b * 255
end


--[[ Converts an HSL color value to RGB.

Based on formula at https://en.wikipedia.org/wiki/HSL_and_HSV#From_HSL.
Assumes h ∈ [0°, 360°), s ∈ [0, 1] and l ∈ [0, 1].

Returns r, g, b each in the set [0, 255].
]]
function colors.hslToRgb(h, s, l)
  local c = (1 - math.abs(2 * l - 1)) * s
  local hprime = h / 60
  local x = c * (1 - math.abs(hprime % 2 - 1))

  local m = l - 0.5 * c
  c = c + m
  x = x + m
  local r, g, b
  if hprime <= 1 then r, g, b = c, x, m
  elseif hprime <= 2 then r, g, b = x, c, m
  elseif hprime <= 3 then r, g, b = m, c, x
  elseif hprime <= 4 then r, g, b = m, x, c
  elseif hprime <= 5 then r, g, b = x, m, c
  elseif hprime < 6 then r, g, b = c, m, x
  end

  return r * 255, g * 255, b * 255
end

--[[ Converts an RGB color value to HSL.

Based on formula at https://en.wikipedia.org/wiki/HSL_and_HSV#Hue_and_chroma.
Assumes r, g, b each in the set [0, 255].

Returns h ∈ [0°, 360°), s ∈ [0, 1] and l ∈ [0, 1].
]]
function colors.rgbToHsl(r, g, b)
  r, g, b = r / 255, g / 255, b / 255
  local max, min = math.max(r, g, b), math.min(r, g, b)
  local h, s, l
  l = (max + min) * 0.5
  if max == min then
    h, s = 0, 0
  else
    local d = (max - min) * 0.5
    s = l > 0.5 and d / (1 - l) or d / l
    h = max == r and ((g - b) / d + (g < b and 6 or 0)) or
        max == g and ((b - r) / d + 2) or
        max == b and ((r - g) / d + 4)
    h = 360 * h / 6
  end
  return h, s, l
end

return colors
