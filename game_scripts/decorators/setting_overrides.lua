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

local game = require 'dmlab.system.game'
local log = require 'common.log'
local helpers = require 'common.helpers'
local timeout = require 'decorators.timeout'
local debug_observations = require 'decorators.debug_observations'
local gadget_selection = require 'decorators.gadget_selection'
local property_decorator = require 'decorators.property_decorator'

-- These parameters may or may not be specified in the init `params`, depending
-- on the execution environment. We should ignore them without raising an
-- 'Invalid setting' error.
local PARAMS_WHITELIST = {
    episodeLengthSeconds = true,
    invocationMode = true,
    allowHoldOutLevels = true,
    levelGenerator = true,
    playerId = true,
    players = true,
    randomSeed = true,
    datasetPath = true,
    enableCameraMovement = true,
    logLevel = true,
}

-- Some scripts still pass these in, though they are not used.
-- Output a warning but not an error.
local PARAMS_DEPRECATED = {
    __platform__ = true,
    textureRandomization = true,
}

--[[ Splits key by '.' separator and returns the last sub-key and sub-table.

If a sub-key is not found it returns the sub-key and sub-table where the find
failed.

Sub-keys are converted to numbers if possible.

Success Example:
```
nestedTable = {levelA = {levelB = {levelC = {10, 11, 12}}}}
key = 'levelA.levelB.levelC.2'
subTable, subKey = findLastSubTableAndSubKey(nestedTable, key)
assert(subTable == nestedTable.levelA.levelB.levelC)
assert(subKey == 2)
assert(subTable[subKey] == 11)
```

Failure Example
```
nestedTable = {levelA = {levelB = {levelC = {10, 11, 12}}}}
key = 'levelA.levelB.levelN.2'
subTable, subKey = findLastSubTableAndSubKey(nestedTable, key)
assert(subTable == nestedTable.levelA.levelB)
assert(subKey == 'levelN')
assert(subTable[subKey] == nil)
```
]]
local function findLastSubTableAndSubKey(nestedTable, key)
  local subTable = nestedTable
  local prevSubTable = subTable
  local lastKey = key
  -- Split string with dots.
  for subKey in key:gmatch('([^.]+)') do
    prevSubTable = subTable
    -- Convert key to integer if possible.
    lastKey = helpers.fromString(subKey)
    subTable = prevSubTable[lastKey]
    if subTable == nil then
      break
    end
  end
  return prevSubTable, lastKey
end

local settings = {}

local setting_overrides = {}

--[[ Decorate the api with an init(params) function that overrides values in
apiParams with the corresponding values in params. This lets command-line and
Python environments override the apiParams settings.

Keyword arguments:

*   `api` the class to decorate. Gets an `init(params)` function added to it.
*   `apiParams` the parameters that hold values overridden by the `params`
    passed into `init(params)`
*   `decorateWithTimeout` (boolean, default nil) if true, decorate `api` with
    the timeout decorator, and use the (potentially updated) version of
    `apiParams.episodeLengthSeconds`.
]]
function setting_overrides.decorate(kwargs)
  local api = kwargs.api
  local apiParams = kwargs.apiParams
  local decorateWithTimeout = kwargs.decorateWithTimeout
  local allowMissingSettings = kwargs.allowMissingSettings or false
  assert(api ~= nil and apiParams ~= nil)
  local debugCameraPos = debug_observations.getCameraPos()
  apiParams.camera = apiParams.camera or debugCameraPos.pos
  apiParams.cameraLook = apiParams.cameraLook or debugCameraPos.look
  apiParams.gadgetSelect = apiParams.gadgetSelect or false
  apiParams.gadgetSwitch = apiParams.gadgetSwitch or false
  apiParams.datasetPath = apiParams.datasetPath or ''
  settings = apiParams
  -- Preserve the existing init call.
  local apiInit = api.init

  -- Override the init() function to parse known settings.
  function api:init(params)
    if params.logLevel then
      log.setLogLevel(params.logLevel)
    end
    -- Override known settings.
    for k, v in pairs(params) do
      local subTable, subKey = findLastSubTableAndSubKey(apiParams, k)
      if subTable[subKey] ~= nil then
        subTable[subKey] = helpers.fromString(v)
      elseif PARAMS_DEPRECATED[k] then
        log.warn('Setting ' .. k .. '" (here set to "' .. v .. '")' ..
                  ' has been deprecated.\n')
      elseif not allowMissingSettings and not PARAMS_WHITELIST[k] then
        error('Invalid setting: "' .. k .. '" = "' .. v .. '"')
      end
    end

    debug_observations.setCameraPos(apiParams.camera, apiParams.cameraLook)

    -- Decorate with timeout. We do this at the end because episodeLengthSeconds
    -- may have been overridden.
    if decorateWithTimeout then
      assert(apiParams.episodeLengthSeconds ~= nil,
             'Time out must have episodeLengthSeconds set. ' ..
             '(Set to false to disable)')
      timeout.decorate(api, apiParams.episodeLengthSeconds)
    end

    gadget_selection(api, apiParams)
    property_decorator.decorate(api)

    property_decorator.addReadWrite('params', apiParams)
    property_decorator.addReadOnly('console',
      function(command)
        game:console(command)
        return property_decorator.RESULT.SUCCESS
      end)

    -- Call version of `init` that existed before decoration.
    if apiInit then
      return apiInit(api, params)
    end
  end
end

function setting_overrides:settings()
  return settings
end

return setting_overrides

