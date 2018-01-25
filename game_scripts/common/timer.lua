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

local Timer = {}
local TimerMT = {__index = Timer}

--[[ Create a timer class that starts at currentTime, if specified, or 0 if not.

The returned `t` must have `t:update(currentTime)` called on it whenever
`currentTime` changes. This call will trigger any callbacks on timers that have
expired.
--]]
function Timer.new(currentTime)
  return setmetatable({_currentTime = currentTime or 0, _timers = {}}, TimerMT)
end

--[[ Start timer `kwargs.name` that will fire in `kwargs.time` seconds.

Once the timer fires, `kwargs.callback` will be called and the timer will be
removed, unless the callback returns `true`.

Keyword arguments:

*   `name` (string) The name of the timer.
*   `time` (number) The number of seconds from now until this timer goes off.
*   `callback` (function) Callback that's called when the timer goes off.
    If the callback returns `true` then the timer is re-engaged to go off again
    after the same amount of time. By default, however, the timer is removed.

Example: Print a lovely message in 2 seconds.

```Lua
local Timer = require 'common.timer'

myTimer = Timer.new()
myTimer:start{
    name = 'my love timer',
    time = 2,
    callback = function() print('LOVE!') end,
}

function api:myFunctionCalledEveryFrame(timeSeconds)
  myTimer:update(timeSeconds)
end

```
--]]
function Timer:start(kwargs)
  assert(kwargs.name)
  assert(kwargs.time)
  assert(kwargs.callback)
  self._timers[kwargs.name] = {
      triggerTime = self._currentTime + kwargs.time,
      time = kwargs.time,
      callback = kwargs.callback,
  }
end

-- Remove `name` from the active timers.
function Timer:remove(name)
  self._timers[name] = nil
end

-- Remove all timers, but maintain the current time.
function Timer:removeAll()
  self._timers = {}
end

-- Returns true if there exists an active timer called `name`.
function Timer:exists(name)
  return self._timers[name] ~= nil
end

-- Returns the time remaining for the timer called `name`.
-- If no such timer exists (perhaps because the timer has already fired and been
-- removed), returns 0.
function Timer:timeRemaining(name)
  if not self:exists(name) then
    return 0
  end
  return self._timers[name].triggerTime - self._currentTime
end

-- Provide the new current time. Check if any timers have fired. For timers that
-- have fired, call their callbacks and remove them unless the callback returns
-- `true`. Must be called manually by the owner of this class.
function Timer:update(currentTime)
  assert(self._currentTime <= currentTime)
  self._currentTime = currentTime

  local timersToRemove = {}
  for name, v in pairs(self._timers) do
    if currentTime >= v.triggerTime then
      local persist = v.callback()
      if persist then
        -- Re-engage the timer. Ensure time is not in the past.
        v.triggerTime = math.max(currentTime, v.triggerTime + v.time)
      else
        timersToRemove[#timersToRemove + 1] = name
      end
    end
  end
  for _, name in pairs(timersToRemove) do
    self:remove(name)
  end
end

return Timer
