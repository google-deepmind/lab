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

local events = require 'dmlab.system.events'
local tensor = require 'dmlab.system.tensor'

local api = {}

local map = 'seekavoid_arena_01'

function api:nextMap()
  local result = map
  map = ''
  return result
end

function api:customDiscreteActionSpec()
  return {
      {name = 'SWITCH_GADGET', min = -1, max = 1},
      {name = 'SELECT_GADGET', min = 0, max = 10},
  }
end

local current_actions
function api:customDiscreteActions(actions)
  current_actions = actions
end

function api:hasEpisodeFinished(time)
  if current_actions[1] ~= 0 then
    events:add('SWITCH_GADGET', tostring(current_actions[1]))
  end
  if current_actions[2] ~= 0 then
    events:add('SELECT_GADGET', tostring(current_actions[2]))
  end
  io.flush()
  return false
end


return api
