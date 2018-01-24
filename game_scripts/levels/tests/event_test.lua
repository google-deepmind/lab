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

-- Tested in python/dmlab_module_test.py.
local events = require 'dmlab.system.events'
local tensor = require 'dmlab.system.tensor'

local api = {}

local map = 'seekavoid_arena_01'

function api:nextMap()
  local result = map
  map = ''
  return result
end

function api:start(episode, seed)
  events:add('TEXT', 'EPISODE ' .. episode)
  events:add('DOUBLE', tensor.DoubleTensor{{1, 0}, {0, 1}})
  events:add('BYTE', tensor.ByteTensor{2, 2})
  events:add('ALL', 'Text', tensor.ByteTensor{3}, tensor.DoubleTensor{7})
end


function api:hasEpisodeFinished(time)
  if time >= 1.0 then
    events:add('LOG', 'Episode ended')
    return true
  else
    return false
  end
end

return api
