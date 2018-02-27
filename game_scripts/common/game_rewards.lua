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

local game_rewards = {}

game_rewards.REASONS = {
    -- `playerId` touched reward pickup.
    'PICKUP_REWARD',

    -- `playerId` touched goal pickup.
    'PICKUP_GOAL',

    -- Level triggered a reward at `playerId`.
    'TARGET_SCORE',

    -- `playerId` tagged self.
    'TAG_SELF',

    -- `playerId` tagged `otherPlayerId`
    'TAG_PLAYER',

    -- `playerId` picked up enemy flag.
    'CTF_FLAG_BONUS',

    -- `playerId` captured the opposing team's flag.
    'CTF_CAPTURE_BONUS',

    -- `playerId` is part of a team that has captured the opposing team's flag.
    'CTF_TEAM_BONUS',

    -- `playerId` tagged opponent(`otherPlayerId`) flag carrier.
    'CTF_FRAG_CARRIER_BONUS',

    -- `playerId` returned their team flag to the team's base.
    'CTF_RECOVERY_BONUS',

    -- `playerId` is on the same team as the flag carrier and tagged opponent
    -- (`otherPlayerId`) who damaged our flag carrier.
    'CTF_CARRIER_DANGER_PROTECT_BONUS',

    -- `playerId` tagged opponent(`otherPlayerId`) while `playerId` or
    -- `otherPlayerId` is near `playerId`'s flag.
    'CTF_FLAG_DEFENSE_BONUS',

    -- `playerId` tagged opponent(`otherPlayerId`) while `playerId` or
    -- `otherPlayerId` is near `playerId`'s flag carrier.
    'CTF_CARRIER_PROTECT_BONUS',

    -- `playerId` returned the team flag just before payerId's team captured
    -- the opposing team's flag.
    'CTF_RETURN_FLAG_ASSIST_BONUS',

    -- `playerId` tagged opponent team's flag carrier just before a capturing
    -- event occurred.
    'CTF_FRAG_CARRIER_ASSIST_BONUS',
}

function game_rewards:initScoreOveride(kwargs)
  kwargs.reward = {}
  kwargs.reward.red = {}
  kwargs.reward.blue = {}
  for _, reason in ipairs(self.REASONS) do
    kwargs.reward[reason] = false
    kwargs.reward.red[reason] = false
    kwargs.reward.blue[reason] = false
  end
end

function game_rewards:overrideScore(kwargs, rewardInfo)
  local playerId = rewardInfo.playerId
  local reason = rewardInfo.reason
  local team = rewardInfo.team
  local location = rewardInfo.location or {0, 0, 0}
  local otherPlayerId = rewardInfo.otherPlayerId or -1
  local score = kwargs.reward and kwargs.reward[team] and
                kwargs.reward[team][reason] or kwargs.reward[reason] or
                rewardInfo.score
  score = tonumber(score)
  local d = tensor.DoubleTensor
  events:add('reward', reason, team, d{score}, d{playerId}, d(location),
             d{otherPlayerId})
  return score
end


return game_rewards
