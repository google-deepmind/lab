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


local custom_entities = {}


local function makePlane(p1, p2, p3)
  return '( ' .. table.concat(p1, ' ') .. ' ) ' ..
         '( ' .. table.concat(p2, ' ') .. ' ) ' ..
         '( ' .. table.concat(p3, ' ') .. ' ) ' ..
         'common/caulk 0 0 0 0 0 0 0 0\n'
end


local function brushBottom(x, y, d)
  return makePlane({x + d, y + d, 8},
                   {x - d, y + d, 8},
                   {x - d, y - d, 8})
end


local function brushTop(x, y, d)
  return makePlane({x - d, y - d, 48},
                   {x - d, y + d, 48},
                   {x + d, y + d, 48})
end


local function brushFace1(x, y, d)
  return makePlane({x - d, y - d, 24},
                   {x + d, y - d, 24},
                   {x + d, y - d, 16})
end


local function brushFace2(x, y, d)
  return makePlane({x + d, y - d, 24},
                   {x + d, y + d, 24},
                   {x + d, y + d, 16})
end


local function brushFace3(x, y, d)
  return makePlane({x + d, y + d, 24},
                   {x - d, y + d, 24},
                   {x - d, y + d, 16})
end


local function brushFace4(x, y, d)
  return makePlane({x - d, y + d, 24},
                   {x - d, y - d, 24},
                   {x - d, y - d, 16})
end


local function makeBox(location, size)
  local x, y = unpack(location)
  return '  {\n' ..
              brushBottom(x, y, size) ..
              brushTop(x, y, size) ..
              brushFace1(x, y, size) ..
              brushFace2(x, y, size) ..
              brushFace3(x, y, size) ..
              brushFace4(x, y, size) ..
         '  }\n'
end


-- Creates a teleporter source, linked to destination `name`.
-- Pos are world space coordinates {x, y}.
function custom_entities.makeTeleporter(pos, name)
  local x, y = pos[1], pos[2]

  return '{\n' ..
       '  "model" "models/teleport_platform.md3"\n' ..
       '  "origin" "' .. x .. ' ' .. y .. ' 0"\n' ..
       '  "classname" "misc_model"\n' ..
       '}\n' ..
       -- Trigger doesn't pay attention to `origin`; we have to specify
       -- world space coordinates for the brush.
       '{\n' ..
       '  "classname" "trigger_teleport"\n' ..
       '  "target" "' .. name .. '"\n' ..
           makeBox({x, y}, 24) ..
       '}\n'
end


-- Creates a teleporter destination, facing south on emergence.
-- Pos are world space coordinates {x, y}.
function custom_entities.makeTeleporterTarget(pos, name)
  local x, y = pos[1], pos[2]

  -- Must be above-ground or agent gets stuck after teleport.
  return '{\n' ..
       '  "classname" "misc_teleporter_dest"\n' ..
       '  "origin" "' .. x .. ' ' .. y .. ' 48"\n' ..
       '  "angle" "-90"\n' ..
       '  "targetname" "' .. name .. '"\n' ..
       '}\n'
end

return custom_entities
