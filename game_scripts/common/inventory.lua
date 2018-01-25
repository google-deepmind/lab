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
    IMPULSE = 2,  -- Contact gadget.
    RAPID = 3,    -- Rapid fire gadget.
    ORB = 6,      -- Area damage gadget. (Knocks players)
    BEAM = 7,     -- Accurate and very rapid fire beam.
    DISC = 8,     -- Powerful but long period between firing.
}

inventory.UNLIMITED = -1

inventory.POWERUPS = {
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

function inventory.View(loadOut)
  return setmetatable({_loadOut = loadOut}, ViewMT)
end


return inventory
