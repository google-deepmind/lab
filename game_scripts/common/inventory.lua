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

-- Enums extracted from "engine/code/qcommon/q_shared.h"

local inventory = {}

inventory.GADGETS = {
    IMPULSE = 2,    -- Contact gadget.
    RAPID = 3,      -- Rapid fire gadget.
    SPRAY = 4,      -- Powerful, slow, inaccurate gadget
    BOUNCE_ORB = 5, -- Area damage with delay. Bounces unpredictably.
    ORB = 6,        -- Area damage gadget. (Knocks players)
    BEAM = 7,       -- Accurate and very rapid fire beam.
    DISC = 8,       -- Powerful but long period between firing.
    RIPPLE = 9,     -- Rapid-fire gadget with small area damage.
}

inventory.UNLIMITED = -1

inventory.POWERUPS = {
    QUAD = 2,
    BATTLESUIT = 3,
    HASTE = 4,
    INVIS = 5,
    REGEN = 6,
    FLIGHT = 7,

    RED_FLAG = 8,
    BLUE_FLAG = 9,
}

local function gadgetsToStat(gadgets)
  local result = 0
  for i, gadget in ipairs(gadgets) do
    result = result + 2 ^ (gadget - 1)
  end
  return result
end

local function statToGadgets(stat)
  local result = {}
  local counter = 1
  while stat > 0 do
    local old = stat / 2
    local new = math.floor(old)
    if old ~= new then
      result[#result + 1] = counter
    end
    counter = counter + 1
    stat = new
  end
  return result
end

local STATS = {
    HEALTH = 1,      -- Health of player
    GADGETS = 3,     -- Mask of current gadgets held.
    ARMOR = 4,       -- Quantity of armour.
    MAX_HEALTH = 7,  -- If health is greater than this value it decreases to it
                     -- over time.
}

local PERSISTANT = {
    SCORE = 1,               -- player's personal score
    HITS = 2,                -- total points damage inflicted
    RANK = 3,                -- player rank or team rank
    TEAM = 4,                -- player team
    SPAWN_COUNT = 5,         -- incremented every respawn
    PLAYEREVENTS = 6,        -- 16 bits that can be flipped for events
    ATTACKER = 7,            -- playerId - 1 of last damage inflicter
    ATTACKEE_ARMOR = 8,      -- health/armor of last person we attacked
    TAGGED = 9,              -- count of the number of times you were tagged
    -- player awards tracking
    IMPRESSIVE_COUNT = 10,   -- two disc hits in a row
    EXCELLENT_COUNT = 11,    -- two successive tags in a short amount of time
    DEFEND_COUNT = 12,       -- defend awards
    ASSIST_COUNT = 13,       -- assist awards
    IMPULSE_TAG_COUNT = 14,  -- tags with the impulse
    CAPTURES = 15,           -- flag captures
}

local View = {}
local ViewMT = {__index = View}

function View:gadgetAmount(gadget)
  return self._loadOut.amounts[gadget]
end

function View:setGadgetAmount(gadget, amount)
  self._loadOut.amounts[gadget] = amount
end

-- Returns list of gadgets.
function View:gadgets()
  return statToGadgets(self._loadOut.stats[STATS.GADGETS])
end

-- Sets list of gadgets.
function View:setGadgets(gadgets)
  self._loadOut.stats[STATS.GADGETS] = gadgetsToStat(gadgets)
end

-- Adds gadget with optional amount.
function View:addGadget(gadget, amount)
  if amount then
    self:setGadgetAmount(gadget, amount)
  end
  local gadgets = self:gadgets()
  for i, v in gadgets do
    if v == gadget then
      return
    end
  end
  gadgets[#gadgets + 1] = gadget
  self:setGadgets(gadgets)
end

-- Removes a gadget if present, sets the gadgets amount to 0.
function View:removeGadget(gadget)
  local gadgets = self:gadgets()
  local newGadgets = {}
  for i, v in gadgets do
    if v ~= gadget then
      newGadgets[#newGadgets + 1] = v
    end
  end
  self:setGadgets(newGadgets)
  self:setGadgetAmount(gadget, 0)
end

-- Returns whether powerup is active.
function View:hasPowerUp(powerUp)
  return self._loadOut.powerups[powerUp] ~= 0
end

-- Returns player's armor
function View:armor()
  return self._loadOut.stats[STATS.ARMOR]
end

-- Sets player's armor
function View:setArmor(amount)
  self._loadOut.stats[STATS.ARMOR] = amount
end

-- Gets player's health.
function View:health()
  return self._loadOut.stats[STATS.HEALTH]
end

-- Sets player's health.
function View:setHealth(amount)
  self._loadOut.stats[STATS.HEALTH] = amount
end

-- Gets player's max health. If health() > maxHealth() health will reduce until
-- it matches maxHealth().
function View:maxHealth()
  return self._loadOut.stats[STATS.MAX_HEALTH]
end

-- Sets player's max health.
function View:setMaxHealth(amount)
  self._loadOut.stats[STATS.MAX_HEALTH] = amount
end

-- Returns player's eye position in world units.
function View:eyePos()
  local x, y, z = unpack(self._loadOut.position)
  return {x, y, z + self._loadOut.height}
end

-- Returns players velocity in world units.
function View:velocity()
  return self._loadOut.velocity
end

-- Returns players view direction in Euler angles degrees.
function View:eyeAngles()
  return self._loadOut.angles
end

-- Returns players id.
function View:playerId()
  return self._loadOut.playerId
end

-- Returns players gadget.
function View:gadget()
  return self._loadOut.gadget
end

function View:loadOut()
  return self._loadOut
end

function View:captures()
  return self._loadOut.persistents[PERSISTANT.CAPTURES]
end

function View:rank()
  return self._loadOut.persistents[PERSISTANT.RANK]
end

function View:tagged()
  return self._loadOut.persistents[PERSISTANT.TAGGED]
end

function View:score()
  return self._loadOut.persistents[PERSISTANT.SCORE]
end

function View:team()
  return self._loadOut.persistents[PERSISTANT.TEAM]
end

function View:isBot()
  return self._loadOut.isBot
end

function View:velocity()
  return self._loadOut.velocity
end

function inventory.View(loadOut)
  return setmetatable({_loadOut = loadOut}, ViewMT)
end


return inventory
