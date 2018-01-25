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

local maze_generation = require 'dmlab.system.maze_generation'

local PositionTrigger = {}
local PositionTriggerMT = {__index = PositionTrigger}

--[[ Create a position trigger class that starts at currentTime, if specified.

You must call `update(currentPosition)` whenever the `currentPosition` changes.
This call will make any callbacks on any triggers that have fired with the new
position.
--]]
function PositionTrigger.new()
  return setmetatable({_triggers = {}}, PositionTriggerMT)
end

--[[ Start trigger `kwargs.name` that will fire when currentPosition overlaps
a square in `kwargs.maze` that matches `kwargs.triggerWhenEnter` or
`kwargs.triggerWhenExit`.

Once the trigger fires, `kwargs.callback` will be called and the trigger will be
removed unless `true` is returned.

Keyword arguments:

*   `name` (string) The name of the trigger.
*   `maze` (string) A text layout of the maze.
*   `triggerWhenEnter` (character) The character in `maze` that sets off the
    trigger, the first time the position is on that character in `maze`.
*   `triggerWhenExit` (character) The character in `maze` that sets off the
    trigger, when position stops being that character in `maze`.
*   `callback` (function) Callback that's called when the trigger goes off.
    If the callback returns `true` then the trigger is re-engaged to go off
    under the same enter/exit conditions. By default, however, the trigger is
    removed.

Example: Print a happy message in 2 seconds.

```Lua
local PositionTrigger = require 'common.position_trigger'

local HAPPINESS_LAYER = [ [
..LL.LL..
.L..L..L.
..L...L..
...L.L...
....L....
] ]

myTriggers = PositionTrigger.new()
myTriggers:start{
    name = 'my happiness trigger',
    maze = HAPPINESS_LAYER,
    triggerWhenEnter = 'L',
    callback = function() print('HAPPINESS!') io.flush() end,
}

function api:myFunctionCalledEveryFrame(currentPosition)
  myPositionTrigger:update(currentPosition)
end

```
--]]
function PositionTrigger:start(kwargs)
  assert(kwargs.name)
  assert(kwargs.maze)
  assert(kwargs.triggerWhenExit or kwargs.triggerWhenEnter)
  assert(kwargs.callback)
  local sizeI, sizeJ = self:mazeSize()
  self._triggers[kwargs.name] = {
      maze = maze_generation:mazeGeneration{entity = kwargs.maze},
      triggerWhenExit = kwargs.triggerWhenExit,
      triggerWhenEnter = kwargs.triggerWhenEnter,
      callback = kwargs.callback,
  }

  -- All mazes must have the same size, since `update(currentPosition)` assumes
  -- the same maze for all position triggers.
  assert(sizeI == nil or
         self._triggers[kwargs.name].maze:size() == sizeI, sizeJ)
end

-- Remove `name` from the active triggers.
function PositionTrigger:remove(name)
  self._triggers[name] = nil
end

-- Remove all triggers, but maintain the current position.
function PositionTrigger:removeAll()
  self._triggers = {}
end

-- Returns true if there exists an active trigger called `name`.
function PositionTrigger:exists(name)
  return self._triggers[name] ~= nil
end

-- All mazes must have the same size. Return the size of the first trigger's
-- maze, or nil if no triggers exist.
function PositionTrigger:mazeSize()
  for _, v in pairs(self._triggers) do
    return v.maze:size()
  end
  return nil, nil
end

-- Return the size of the first text maze, which has to be consistent for all
-- triggers.
function PositionTrigger:mazeCoordinatesFromWorldPosition(worldPosition)
  for _, v in pairs(self._triggers) do
    return v.maze:fromWorldPos(worldPosition[1], worldPosition[2])
  end
  return nil, nil
end

--[[ Provide the new current position in world space.
Check if any triggers have fired.
For triggers that have fired, call their callbacks and remove them unless
the callback returns `true`. Must be called manually by the owner of this class.
--]]
function PositionTrigger:update(currentPosition)
  -- If we have no triggers, or if the maze coordinates haven't changed, then
  -- there is nothing to do.
  local nextI, nextJ = self:mazeCoordinatesFromWorldPosition(currentPosition)
  if nextI == nil then return end
  if self._currentI == nextI and self._currentJ == nextJ then return end

  -- Check if we've transitioned cell types. If so, see if we've triggered
  -- the enter or exit criteria. If so, make callback.
  local triggersToRemove = {}
  for name, v in pairs(self._triggers) do
    local currentCell = self._currentI and
                        v.maze:getEntityCell(self._currentI, self._currentJ)
    local nextCell = v.maze:getEntityCell(nextI, nextJ)
    if currentCell ~= nextCell then
      if (v.triggerWhenExit and v.triggerWhenExit == currentCell) or
         (v.triggerWhenEnter and v.triggerWhenEnter == nextCell) then
        local persist = v.callback()
        if not persist then
          triggersToRemove[#triggersToRemove + 1] = name
        end
      end
    end
  end

  -- Remove any triggers that have fired.
  for _, name in pairs(triggersToRemove) do
    self:remove(name)
  end

  -- Update our current maze coordinates.
  self._currentI = nextI
  self._currentJ = nextJ
end

return PositionTrigger

