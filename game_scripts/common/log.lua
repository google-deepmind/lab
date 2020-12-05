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

--[[ A library for logging.

This library provides facilities to emit log messages, each of which is
configured with a log level that measures its verbosity. There are three
built-in levels ERROR, WARN, INFO. The log level may also be any positive
integer.

The program's verbosity can be controlled with the setting 'logLevel': only
messages whose level is less than or equal to the value of the setting are
actually printed. A higher setting means "more verbose". For this purpose,
the built-in levels ERROR, WARN, and INFO have numeric values -2, -1, and 0,
respectively. The setting value 'NEVER' (value -3) has the effect of
suppressing all log output. Specifically, if the 'logLevel' is...

*   INFO (default), then ERROR, WARN, and INFO messages are printed;
*   WARN, then ERROR and WARN messages are printed;
*   ERROR, then only ERROR messages are printed;
*   NEVER, then no messages are printed at all; and
*   any positive integer: ERROR, WARN, INFO, and all messages with custom log
    level less than or equal to this value are printed.

Printed log messages are prefixed with either a letter ('E', 'W', 'I') or a
number corresponding to the message's level, as well as with the file name and
line number of the source location of the log message.

There are functions to log at each one of the three built-in log levels:

```lua
log.info('My script info!')
I game_scripts/my_script.lua:10] My script info!

log.warn('My script warning!')
W game_scripts/my_script.lua:11] My script warning!

log.error('My script error!')
E game_scripts/my_script.lua:12] My script error!
```

Furthermore, there is a function to log at a custom (positive!) verbosity level:

```lua
log.v(5, 'a very verbose trace message')
5 game_scripts/my_script.lua:12] a very verbose trace message
```

Note that all log messages are purely informational and do not affect control
flow. In particular, no log message raises a Lua error or causes execution to
branch or terminate. (There is no "fatal" log level.) Note further that logging
is a regular function call and all arguments are evaluated regardless of whether
the message will actually be printed.
]]

local log = {
    INFO = 0,
    WARN = -1,
    ERROR = -2,
    NEVER = -3,
}

local ESCAPE = string.char(27)
local GREEN = ESCAPE .. '[0;32m'
local ORANGE = ESCAPE .. '[0;33m'
local RED = ESCAPE .. '[0;31m'
local CLEAR = ESCAPE .. '[0;0m'

local _verbosity
local _output

local function _shorten(path)
  return string.match(path, "baselab/(.*)") or
         string.match(path, "runfiles/(.*)") or
         path
end

local function _logInternal(messageType, ...)
  local func = debug.getinfo(3)
  local loc = _shorten(func.source) .. ':' .. func.currentline .. '] '
  _output:write(messageType, ' ')
  _output:write(loc)
  for i, arg in ipairs{...} do
    if i ~= 1 then _output:write('\t') end
    _output:write(tostring(arg))
  end
  _output:write('\n')
  _output:flush()
end

local function _info(...)
  _logInternal(GREEN .. 'I' .. CLEAR, ...)
end

local function _warn(...)
  _logInternal(ORANGE .. 'W' .. CLEAR, ...)
end

local function _error(...)
  _logInternal(RED .. 'E' .. CLEAR, ...)
end

local function _v(verbosity, ...)
  assert(verbosity > log.INFO)
  if verbosity <= _verbosity then
    _logInternal(tostring(verbosity), ...)
  end
end

local function _noop()
end

function log.setLogLevel(threshold)
  _verbosity = log[string.upper(threshold)] or
              tonumber(threshold) or
              error("Invalid logLevel: " .. threshold)
  log.error = _verbosity >= log.ERROR and _error or _noop
  log.warn = _verbosity >= log.WARN and _warn or _noop
  log.info = _verbosity >= log.INFO and _info or _noop
  log.v = _verbosity > log.INFO and _v or _noop
end

function log.getLogLevel()
  return _verbosity
end

function log.setOutput(output)
  _output = output
end

log.setLogLevel(log.INFO)
log.setOutput(io.stdout)

return log
