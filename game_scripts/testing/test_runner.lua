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

--[[ Useful for running Lua tests in DM Lab.

In the build file:

```
cc_test(
    name = "test_script",
    args = ["lua_tests/test_script.lua"],
    data = [":test_script.lua"],
    deps = ["/labyrinth/testing:lua_unit_test_lib"],
)
```

In test_script.lua:

```
local tests = {}

function tests.testAssert()
  assert(true)
end

return test_runner.run(tests)
```
]]
local setting_overrides = require 'decorators.setting_overrides'

local test_runner = {}

function test_runner.run(tests)
  local function runAllTests()
    local statusAll = true
    local errors = ''
    for name, test in pairs(tests) do
      print('[ LuaTest  ] - ' .. name)
      local status, msg = xpcall(test, debug.traceback)
      if not status then
        errors = errors .. 'function tests.' .. name .. '\n' .. msg .. '\n'
        print('[ Failed   ] - ' .. name)
        statusAll = false
      else
        print('[ Success  ] - ' .. name)
      end
    end
    return (statusAll and 0 or 1), errors
  end
  local api = {init = runAllTests}
  setting_overrides.decorate{
      api = api,
      apiParams = {},
      decorateWithTimeout = false,
      allowMissingSettings = true,
  }
  return api
end

return test_runner
