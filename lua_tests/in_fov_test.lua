--[[ Copyright (C) 2018-2019 Google Inc.

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
local helpers = require 'common.helpers'
local game = require 'dmlab.system.game'

local tests = {}

local ORIGIN = {0, 0, 0}
local FORWARD = {100, 0, 0}
local BACK = {-100, 0, 0}
local LEFT = {0, 100, 0}
local RIGHT = {0, -100, 0}
local DOWN = {0, 0, -100}
local UP = {0, 0, 100}

local NULL_ROT = {0, 0, 0}
local YAW_180 = {0, 180, 0}
local YAW_LEFT_89 = {0, 89, 0}
local YAW_LEFT_90 = {0, 90, 0}
local YAW_LEFT_91 = {0, 91, 0}
local YAW_RIGHT_90 = {0, -90, 0}
local PITCH_DOWN_90 = {-90, 0, 0}
local PITCH_UP_90 = {90, 0, 0}

function tests:narrowFovForward()
  assert(game:inFov(ORIGIN, FORWARD, NULL_ROT, 1))
end

function tests:narrowFovLeft()
  assert(game:inFov(ORIGIN, LEFT, YAW_LEFT_90, 1))
end

function tests:narrowFovRight()
  assert(game:inFov(ORIGIN, RIGHT, YAW_RIGHT_90, 1))
end

function tests:narrowFovBack()
  assert(game:inFov(ORIGIN, BACK, YAW_180, 1))
end

function tests:narrowFovDown()
  assert(game:inFov(ORIGIN, DOWN, PITCH_DOWN_90, 1))
end

function tests:narrowFovUp()
  assert(game:inFov(ORIGIN, UP, PITCH_UP_90, 1))
end

function tests:nullFov()
  asserts.EQ(false, game:inFov(ORIGIN, FORWARD, NULL_ROT, 0))
end

function tests:edgeIn()
  asserts.EQ(true, game:inFov(ORIGIN, {110, 100, 100}, NULL_ROT, 90))
end

function tests:edgeOut()
  asserts.EQ(false, game:inFov(ORIGIN, {90, 100, 100}, NULL_ROT, 90))
end

function tests:edgeInRot()
  asserts.EQ(true, game:inFov(ORIGIN, {100, 110, 100}, YAW_LEFT_90, 90))
end

function tests:edgeOutRot()
  asserts.EQ(false, game:inFov(ORIGIN, {100, 90, 100}, YAW_LEFT_90, 90))
end

function tests:fovIn()
  asserts.EQ(true, game:inFov(ORIGIN, {100, 100, 100}, NULL_ROT, 91))
end

function tests:fovOut()
  asserts.EQ(false, game:inFov(ORIGIN, {100, 100, 100}, NULL_ROT, 89))
end

function tests:fovInRot()
  asserts.EQ(true, game:inFov(ORIGIN, {100, 100, 100}, YAW_LEFT_90, 91))
end

function tests:fovOutRot()
  asserts.EQ(false, game:inFov(ORIGIN, {100, 100, 100}, YAW_LEFT_90, 89))
end

function tests:angleIn()
  asserts.EQ(true, game:inFov(ORIGIN, {100, 100, 100}, YAW_LEFT_89, 90))
end

function tests:angleOut()
  asserts.EQ(false, game:inFov(ORIGIN, {100, 100, 100}, YAW_LEFT_91, 90))
end

return test_runner.run(tests)
