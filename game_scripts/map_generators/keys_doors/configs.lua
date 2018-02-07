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

local random = require 'common.random'

local SAMPLEABLE_KEYS = {
    'coreSequenceLength',
    'numTrapRooms',
    'numEasyExtraRooms',
    'numRedHerringRooms',
    'numUnopenableDoors',
    'numUselessKeys',
    'extraUsefulKeys',
    'numOpenableDoors',
    'numRedHerringRoomUselessKeys',
}

local UNDEFINED = 'undefined_flag'

local api = {}

-- Returns the default config of keys_doors levels.
local function defaultConfig()
  return {
      -- key/door pairs, excludes the goal. 0 means single room, no doors.
      coreSequenceLength = {1, 3},
      coreNoEarlyKeys = false,
      numTrapRooms = 1,
      probTrapRoomDuringCore = 0.5,
      numEasyExtraRooms = {0, 1},
      numRedHerringRooms = 1,
      -- Unlike the other rooms, empty rooms are not filled in after the core
      -- by default, to reduce clutter.
      numMaxEmptyRooms = 1,
      redHerringRespectCoreOrder = false,
      probRedHerringRoomDuringCore = 0.5,
      -- Number of useless keys specifically placed in red herring rooms.
      numRedHerringRoomUselessKeys = 0,
      -- These compete with useless keys for colors.
      numUnopenableDoors = 2,
      numUselessKeys = {0, 1},
      extraUsefulKeys = 0,
      numOpenableDoors = {0, 1},
      compact = true,
      -- Set to false for less clutter.
      fillWithEmptyRooms = false,
      -- By default, only fill in for short sequences.
      probEmptyRoomDuringCore = UNDEFINED,
      -- A list of possible starting positions, e.g. {{1, 1}, {1, 2}}.
      -- nil means it can start from anywhere.
      startRoomGridPositions = UNDEFINED,
      roomGridHeight = 3,
      roomGridWidth = 3,
      addToGoalRoom = true,
      possibleColors = {'red', 'green', 'blue', 'black', 'white'},
      -- Whether to output logs during level generation.
      verbose = false,
  }
end

--[[ Some config values may be given as an interval, from which the actual value
to be used should be randomly sampled.

Arguments:

*   config - original config, some of the keys are to be sampled.

Returns:

*   The config after sampling.
]]
local function sampleConfig(config)
  for _, key in pairs(SAMPLEABLE_KEYS) do
    if type(config[key]) == 'table' then
      config[key] = random:uniformInt(config[key][1], config[key][2])
    end
  end
  return config
end

--[[ Generates the configs used by keys_doors level generator.

Arguments:

*   overrideConfig - used to override the values in default config.

Returns:

*   The config after overriding, sampling and sanitizing.
]]
function api.getConfig(overrideConfig)
  -- Get the default config.
  local config = defaultConfig()

  -- Override the default config with the input config.
  overrideConfig = overrideConfig or {}
  assert(type(overrideConfig) == 'table')
  for key, value in pairs(overrideConfig) do
    if config[key] == nil then
      error('Unrecognized config key: ' .. key)
    end
    config[key] = value
  end

  -- Sample the config for sampleable keys.
  config = sampleConfig(config)

  for key, value in pairs(config) do
    if value == UNDEFINED then
      config[key] = nil
    end
  end

  assert(#config.possibleColors > config.coreSequenceLength,
      'Not enough possibleColors for coreSequenceLength: ' ..
      config.coreSequenceLength)

  if not config.probEmptyRoomDuringCore then
    -- To reduce clutter, we by default place empty rooms with low probability
    -- for longer sequences
    if config.coreSequenceLength == 1 then
      config.probEmptyRoomDuringCore = 0.8
    elseif config.coreSequenceLength == 2 then
      config.probEmptyRoomDuringCore = 0.5
    else
      config.probEmptyRoomDuringCore = 0.
    end
  end

  return config
end

return api
