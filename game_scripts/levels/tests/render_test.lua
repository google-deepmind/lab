-- Tested in python/lookat_test.py.
local game = require 'dmlab.system.game'
local tensor = require 'dmlab.system.tensor'
local random = require 'common.random'
local pickups = require 'common.pickups'
local custom_observations = require 'decorators.custom_observations'

local api = {}

function api:init(params)
  self._vertFlipBuffer = tonumber(params.vertFlipBuffer) or 0
end

function api:createPickup(classname)
  return pickups.defaults[classname]
end

function api:commandLine(cmdLine)
  return cmdLine .. ' +set r_vertFlipBuffer "' .. self._vertFlipBuffer .. '"'
end

function api:start(episode, seed)
  random:seed(seed)
end

function api:nextMap()
  return 'lookat_test'
end

return api
