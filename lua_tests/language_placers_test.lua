--[[ Copyright (C) 2017-2019 Google Inc.

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

local asserts = require 'testing.asserts'
local test_runner = require 'testing.test_runner'
local placers = require 'language.placers'

local helpers = require 'common.helpers'
local random = require 'common.random'

local function listsMatch(a, b)
  if #a ~= #b then return false end
  for ii, v in ipairs(a) do
    if b[ii] ~= v then return false end
  end
  return true
end

-- See if the list `value` matches any of the validResults and accumulate a
-- count of how many times this value has been seen.
local function validateAndCount(value, validResults, seen)
  local match = false
  for _, valid in ipairs(validResults) do
    if listsMatch(value, valid) then
      match = true break
    end
  end
  local key = helpers.tostring(value)
  seen[key] = (seen[key] or 0) + 1
  if not match then
    print("MATCH FAILED: ")
    print(key)
  end
  return match
end

local function checkEvenDistribution(results, expected, tolerance)
  for _, count in pairs(results) do
    asserts.GE(count, expected - tolerance)
    asserts.LE(count, expected + tolerance)
  end
end


local tests = {}

function tests.static_shouldHaveOneToOneMapping()
  local mapToPickup = {1, 2, 3, 4}
  local EXPECTED_ORDER = helpers.shallowCopy(mapToPickup)
  local placer = placers.createStatic()
  placer{mapIdToPickupId = mapToPickup}
  asserts.tablesEQ(mapToPickup, EXPECTED_ORDER)

  random:shuffleInPlace(mapToPickup)
  placer{mapIdToPickupId = mapToPickup}
  asserts.tablesEQ(mapToPickup, EXPECTED_ORDER)
end


function tests.random_shouldShuffleIds()
  local mapToPickup = {1, 2, 3, 4, 5}
  -- Force a seed for reproducability.
  random:seed(44)

  local placer = placers.createRandom()
  placer{mapIdToPickupId = mapToPickup}
  asserts.tablesNE(mapToPickup, {1, 2, 3, 4, 5})
  table.sort(mapToPickup)
  asserts.tablesEQ(mapToPickup, {1, 2, 3, 4, 5})
end


function tests.room_shouldPutObjectsInRequestedRoom()
  -- Map information about which object poitions are in which rooms.
  local map = {
      source = {
          rooms = {
              A = {[2] = true},
              B = {[1] = true},
          }
      }
  }
  local roomAObject = {region = 'A', _pickupId = 1}
  local roomBObject = {region = 'B', _pickupId = 2}
  local placer = placers.createRoom()
  local mapToPickup = {1, 2}
  placer{mapIdToPickupId = mapToPickup,
         objectDescriptions = {roomAObject, roomBObject},
         map = map}
  asserts.EQ(mapToPickup[1], roomBObject._pickupId)
  asserts.EQ(mapToPickup[2], roomAObject._pickupId)
end

function tests.room_shouldFailIfMoreObjectsThanSlotsInRoom()
  local map = {
      source = {
          rooms = {
              A = {[2] = true},
              B = {[1] = true},
          }
      }
  }
  local roomAObject1 = {region = 'A', _pickupId = 1}
  local roomAObject2 = {region = 'A', _pickupId = 2}
  local placer = placers.createRoom()
  local mapToPickup = {1, 2}
  asserts.shouldFail(function ()
        placer{mapIdToPickupId = mapToPickup,
               objectDescriptions = {roomAObject1, roomAObject2},
               map = map}
  end)
end

function tests.room_shouldNotPlaceIfRegionSaysSo()
  local map = {
      source = {
          rooms = {
              A = {[1] = true, [2] = true},
          }
      }
  }
  local roomAObject = {region = 'DO_NOT_PLACE', _pickupId = 1}
  local placer = placers.createRoom()
  local mapToPickup = {1, 2}
  placer{mapIdToPickupId = mapToPickup,
         objectDescriptions = {roomAObject},
         map = map}
  asserts.tablesEQ(mapToPickup, {0, 0})

end


function tests.room_shouldPickRandomlyFromSlotsInRoomSpecified()
  local map = {
      source = {
          rooms = {
              A = {[1] = true, [2] = true},
              B = {[3] = true},
          }
      }
  }
  local roomAObject = {region = 'A', _pickupId = 1}
  local placer = placers.createRoom()
  -- Unused place on map should get 0.  Should never pick room B's slot
  local validResults = {
      {0, roomAObject._pickupId, 0},
      {roomAObject._pickupId, 0, 0}
  }

  local TRIALS = 200
  local EXPECTED_HITS = TRIALS / #validResults
  local TOLERANCE = EXPECTED_HITS * 0.11 -- 11% tolerance

  local seen = {}
  for _ = 1, TRIALS do
    local mapToPickup = {1, 2, 3}
    placer{mapIdToPickupId = mapToPickup,
           objectDescriptions = {roomAObject},
           map = map}
    assert(validateAndCount(mapToPickup, validResults, seen))
  end
  checkEvenDistribution(seen, EXPECTED_HITS, TOLERANCE)
end

function tests.room_shouldPickRandomlyFromSlotsInRoomSpecified2()
  local map = {
      source = {
          rooms = {
              A = {[1] = true, [2] = true},
              B = {[3] = true, [4] = true},
          }
      }
  }
  local roomAObject = {region = 'A', _pickupId = 1}
  local roomBObject = {region = 'B', _pickupId = 2}
  local placer = placers.createRoom()
  -- Unused place on map should get 0.  Should never pick room B's slot
  local validResults = {
      {0, roomAObject._pickupId, 0, roomBObject._pickupId},
      {0, roomAObject._pickupId, roomBObject._pickupId, 0},
      {roomAObject._pickupId, 0, 0, roomBObject._pickupId},
      {roomAObject._pickupId, 0, roomBObject._pickupId, 0},
  }

  local TRIALS = #validResults * 100
  local EXPECTED_HITS = TRIALS / #validResults
  local TOLERANCE = EXPECTED_HITS * 0.11 -- 11% tolerance

  local seen = {}
  for _ = 1, TRIALS do
    local mapToPickup = {1, 2, 3, 4}
    placer{mapIdToPickupId = mapToPickup,
           objectDescriptions = {roomAObject, roomBObject},
           map = map}
    assert(validateAndCount(mapToPickup, validResults, seen))
  end
  checkEvenDistribution(seen, EXPECTED_HITS, TOLERANCE)
end


function tests.room_shouldPickRandomlyFromAllSlotsIfRegionIsAny()
  local map = {
      source = {
          rooms = {
              A = {[2] = true},
              B = {[1] = true},
          }
      }
  }
  local anywhereObject = {region = 'any', _pickupId = 1}
  -- Unused place on map should get 0
  local validResults = {
      {0, anywhereObject._pickupId},
      {anywhereObject._pickupId, 0}
  }

  local TRIALS = #validResults * 100
  local EXPECTED_HITS = TRIALS / #validResults
  local TOLERANCE = EXPECTED_HITS * 0.11

  local seen = {}
  local placer = placers.createRoom()
  for _ = 1, TRIALS do
    local mapToPickup = {1, 2}
    placer{mapIdToPickupId = mapToPickup,
           objectDescriptions = {anywhereObject},
           map = map}
    assert(validateAndCount(mapToPickup, validResults, seen))
  end
  checkEvenDistribution(seen, EXPECTED_HITS, TOLERANCE)
end

function tests.room_shouldPlaceMixOfAnyAndSpecificRegionObjects()
  local map = {
      source = {
          rooms = {
              A = {[1] = true, [2] = true},
              B = {[3] = true, [4] = true},
          }
      }
  }
  local A = 1
  local B = 2
  local W = 3  -- W for 'wildcard' to make the valid results table aligned
  local roomAObject = {region = 'A', _pickupId = A}
  local roomBObject = {region = 'B', _pickupId = B}
  local anywhereObject = {region = 'any', _pickupId = W}

    -- Unused place on map should get 0.  Should never pick room B's slot
  local validResults = {
      {0, A, W, B},
      {W, A, 0, B},
      {0, A, B, W},
      {W, A, B, 0},
      {A, 0, W, B},
      {A, W, 0, B},
      {A, 0, B, W},
      {A, W, B, 0},
  }

  local TRIALS = #validResults * 100
  local EXPECTED_HITS = TRIALS / #validResults
  local TOLERANCE = EXPECTED_HITS * 0.20 -- 20% tolerance

  local seen = {}
  local placer = placers.createRoom()
  for _ = 1, TRIALS do
    local mapToPickup = {1, 2, 3, 4}
    placer{mapIdToPickupId = mapToPickup,
           objectDescriptions = {roomAObject, roomBObject, anywhereObject},
           map = map}
    assert(validateAndCount(mapToPickup, validResults, seen))
  end
  checkEvenDistribution(seen, EXPECTED_HITS, TOLERANCE)
end

return test_runner.run(tests)
