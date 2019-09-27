--[[ Copyright (C) 2019 Google LLC

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
local helpers = require 'common.helpers'
local psychlab_factory = require 'factories.psychlab.factory'
local psychlab_helpers = require 'factories.psychlab.helpers'
local image = require 'dmlab.system.image'
local point_and_click = require 'factories.psychlab.point_and_click'
local random = require 'common.random'
local tensor = require 'dmlab.system.tensor'
local set = require 'common.set'
local log = require 'common.log'
local defaults = require 'levels.contributed.psychlab.visuospatial_suite.factories.defaults'


--[[ A task in which an agent must pursue some objects and avoid others.
In each trial a number of coloured squares appear in motion. Some of them are
targets, some must be avoided, and some are distracters. This is indicated
by color.
]]

local TARGET_SIZE = 0.125 -- fraction of screen to fill for study what target
-- How far from center to place the where location centers, as fraction
local LOCATION_RADIUS = 0.75
-- How often to update the animation
local INTERFRAME_INTERVAL = 4

local NUM_LOCATIONS = {2, 3, 4, 5, 6}
local AVOID_PROPORTION = 0.5

local DISTRACT_LAYER = 3  -- top
local AVOID_LAYER = 2
local TARGET_LAYER = 1  -- bottom

local factory = {}

function factory.createLevelApi(kwargs)
  kwargs.episodeLengthSeconds = kwargs.episodeLengthSeconds or
      defaults.EPISODE_LENGTH_SECONDS
  kwargs.timeToFixateCross = kwargs.timeToFixateCross or
      defaults.TIME_TO_FIXATE_CROSS
  kwargs.fastInterTrialInterval = kwargs.fastInterTrialInterval or
      defaults.FAST_INTER_TRIAL_INTERVAL
  kwargs.screenSize = kwargs.screenSize or defaults.SCREEN_SIZE
  kwargs.bgColor = kwargs.bgColor or defaults.BG_COLOR
  kwargs.colors = kwargs.colors or defaults.COLORS
  kwargs.trialsPerEpisodeCap = kwargs.trialsPerEpisodeCap or
      defaults.TRIALS_PER_EPISODE_CAP
  kwargs.targetSize = kwargs.targetSize or TARGET_SIZE
  kwargs.locationRadius = kwargs.locationRadius or LOCATION_RADIUS
  kwargs.fixationReward = kwargs.fixationReward or defaults.FIXATION_REWARD
  kwargs.correctReward = kwargs.correctReward or defaults.CORRECT_REWARD
  kwargs.incorrectReward = kwargs.incorrectReward or defaults.INCORRECT_REWARD
  kwargs.fixationSize = kwargs.fixationSize or defaults.FIXATION_SIZE
  kwargs.fixationColor = kwargs.fixationColor or defaults.FIXATION_COLOR
  kwargs.buttonSize = kwargs.buttonSize or defaults.BUTTON_SIZE
  kwargs.maxStepsOffScreen = kwargs.maxStepsOffScreen or
      defaults.MAX_STEPS_OFF_SCREEN
  kwargs.numLocations = kwargs.numLocations or NUM_LOCATIONS
  kwargs.avoidProportion = kwargs.avoidProportion or AVOID_PROPORTION

  --[[ What then where psychlab environment class
  ]]
  local env = {}
  env.__index = env

  setmetatable(env, {
      __call = function (cls, ...)
        local self = setmetatable({}, cls)
        self:_init(...)
        return self
      end
  })

  -- init gets called at the start of each episode
  function env:_init(pac, opts)
    self.screenSize = opts.screenSize
    log.info('opts passed to _init:\n' .. helpers.tostring(opts))
    log.info('args passed to _init:\n' .. helpers.tostring(arg))

    self:setupImages()
    -- handle to the point and click api
    self.pac = pac
  end

  -- reset is called after init. It is called only once per episode.
  -- Note: the episodeId passed to this function may not be correct if the job
  -- has resumed from a checkpoint after preemption.
  function env:reset(episodeId, seed, ...)
    random:seed(seed)

    self.pac:setBackgroundColor(kwargs.bgColor)
    self.pac:clearWidgets()
    psychlab_helpers.addFixation(self, kwargs.fixationSize)
    self.reward = 0

    self.currentTrial = {}

    psychlab_helpers.setTrialsPerEpisodeCap(self, kwargs.trialsPerEpisodeCap)

    -- blockId groups together all rows written during the same episode
    self.blockId = random:uniformInt(1, 2 ^ 32)
    self.trialId = 1
  end

  function env:setupImages()
    self.images = {}

    self.images.fixation = psychlab_helpers.getFixationImage(self.screenSize,
      kwargs.bgColor, kwargs.fixationColor, kwargs.fixationSize)

    local h = kwargs.buttonSize * self.screenSize.height
    local w = kwargs.buttonSize * self.screenSize.width

    self.images.greenImage = tensor.ByteTensor(h, w, 3)
    self.images.greenImage:select(3, 1):fill(100)
    self.images.greenImage:select(3, 2):fill(255)
    self.images.greenImage:select(3, 3):fill(100)

    self.images.redImage = tensor.ByteTensor(h, w, 3)
    self.images.redImage:select(3, 1):fill(255)
    self.images.redImage:select(3, 2):fill(100)
    self.images.redImage:select(3, 3):fill(100)

    self.images.whiteImage = tensor.ByteTensor(h, w, 3):fill(255)
    self.images.blackImage = tensor.ByteTensor(h, w, 3)

    local whatPixels = psychlab_helpers.getSizeInPixels(self.screenSize,
                                                        kwargs.targetSize)
    self.whatTensor = tensor.ByteTensor(whatPixels.height,
                                        whatPixels.width,
                                        3)
    self.whereTensor = tensor.ByteTensor(self.screenSize.height,
                                         self.screenSize.width,
                                         3)
  end

  function env:getImage(objectId)
    local h = kwargs.buttonSize * self.screenSize.height
    local w = kwargs.buttonSize * self.screenSize.width
    local img = tensor.ByteTensor(h, w, 3)
    img:fill(kwargs.colors[objectId])
    return img
  end

  function env:finishTrial(delay)
    self.currentTrial.blockId = self.blockId
    self.currentTrial.reactionTime =
        game:episodeTimeSeconds() - self._currentTrialStartTime
    self.currentTrial.responseTime =
        game:episodeTimeSeconds() - self._responseStartTime

    self.currentTrial.stepCount = self.pac:elapsedSteps()
    psychlab_helpers.publishTrialData(self.currentTrial, kwargs.schema)
    psychlab_helpers.finishTrialCommon(self, delay, kwargs.fixationSize)
  end

  function env:fixationCallback(name, mousePos, hoverTime, userData)
    if hoverTime == kwargs.timeToFixateCross then
      self.pac:addReward(kwargs.fixationReward)
      self.pac:removeWidget('fixation')
      self.pac:removeWidget('center_of_fixation')

      -- Fixation initiates the next trial
      self._rewardToDeliver = 0
      self.currentTrial.reward = 0
      self.currentTrial.trialId = self.trialId
      self.trialId = self.trialId + 1

      -- Measure reaction time from trial initiation (in microseconds)
      self._currentTrialStartTime = game:episodeTimeSeconds()
      self.pac:resetSteps()

      self:pursuitPhase()
    end
  end

  function env:onHoverEnd(name, mousePos, hoverTime, userData)
    self.currentTrial.reward = self._rewardToDeliver
    self.pac:addReward(self.currentTrial.reward)
    self:finishTrial(kwargs.fastInterTrialInterval)
  end

  function env:correctResponseCallback(name, mousePos, hoverTime, userData)
    if not self.finished then
      self.finished = true
      self.currentTrial.response = name
      self.currentTrial.correct = 1

      self.pac:clearTimers()
      self.pac:updateWidget(name, self.images.greenImage)
      self._rewardToDeliver = kwargs.correctReward
    end
  end

  function env:incorrectResponseCallback(name, mousePos, hoverTime, userData)
    if not self.finished then
      self.finished = true
      self.currentTrial.response = name
      self.currentTrial.correct = 0

      self.pac:clearTimers()
      self.pac:updateWidget(name, self.images.redImage)
      self._rewardToDeliver = kwargs.incorrectReward
    end
  end

  function env:pursuitPhase()
    self.objects, self.transition = self:setupAnimation()
    self.finished = false
    self:addResponseButtons()
    -- Measure time till response in microseconds
    self._responseStartTime = game:episodeTimeSeconds()
    self.currentTrial.responseSteps = 0

    self:updateResponseButtons()  -- starts animation
  end

  function env:setupAnimation()
    local numLocations = psychlab_helpers.randomFrom(kwargs.numLocations)
    self.currentTrial.numLocations = numLocations
    local objects = {}
    local offsetRadians = (math.pi * random:uniformInt(1, 2 * numLocations)
        / numLocations)
    for i = 1, numLocations do
      local posRadians = 2 * math.pi * (i - 1) / numLocations + offsetRadians
      local x = 0.5 * (1 + kwargs.locationRadius * math.sin(posRadians))
      local y = 0.5 * (1 + kwargs.locationRadius * math.cos(posRadians))
      local velMagnitude = random:uniformReal(0.06, 0.08)
      local velRadians = random:uniformReal(0, 2 * math.pi)
      local dx = velMagnitude * math.sin(velRadians)
      local dy = velMagnitude * math.cos(velRadians)
      local layer = DISTRACT_LAYER
      if i == 1 then
        layer = TARGET_LAYER
      elseif i - 1 <= (numLocations - 1) * kwargs.avoidProportion then
        layer = AVOID_LAYER
      end
      local object = {x = x, y = y, dx = dx, dy = dy, layer = layer}
      table.insert(objects, object)
    end

    local transition = function()
      for i = 1, numLocations do
        local object = objects[i]
        local x = object.x + object.dx
        local y = object.y + object.dy
        local dx, dy = object.dx, object.dy
        if x <= 0.05 or x >= 0.95 then
          dx = -dx
          x = x + 2 * dx
        end
        if y <= 0.05 or y >= 0.95 then
          dy = -dy
          y = y + 2 * dy
        end
        object.x = x
        object.y = y
        object.dx = dx
        object.dy = dy
      end
    end

    return objects, transition
  end

  function env:addResponseButtons()
    for i = 1, self.currentTrial.numLocations do
      local object = self.objects[i]
      local x, y = object.x, object.y
      local pos = psychlab_helpers.getUpperLeftFromCenter(
          {x, y}, kwargs.buttonSize)
      local image = self:getImage(object.layer)
      local responseCallback, endCallback
      if object.layer == TARGET_LAYER then
        responseCallback = self.correctResponseCallback
        endCallback = self.onHoverEnd
      elseif object.layer == AVOID_LAYER then
        responseCallback = self.incorrectResponseCallback
        endCallback = self.onHoverEnd
      end
      self.pac:addWidget{
          name = 'location_' .. i,
          image = image,
          pos = pos,
          size = {kwargs.buttonSize, kwargs.buttonSize},
          imageLayer = object.layer,
          mouseHoverCallback = responseCallback,
          mouseHoverEndCallback = endCallback,
      }
    end
  end

  function env:updateResponseButtons()
    self.transition()
    for i = 1, self.currentTrial.numLocations do
      local object = self.objects[i]
      local x, y = object.x, object.y
      local pos = psychlab_helpers.getUpperLeftFromCenter(
          {x, y}, kwargs.buttonSize)
      local absPos = {pos[1] * self.screenSize.width,
                      pos[2] * self.screenSize.height}
      self.pac:setWidgetAbsPos('location_' .. i, absPos)
    end
    self.pac:addTimer{
        name = 'interframe_interval',
        timeout = INTERFRAME_INTERVAL,
        callback = function(...) self:updateResponseButtons() end
    }
  end

  -- Remove the test array
  function env:removeArray()
    -- remove the response buttons
    self.pac:clearWidgets()
  end

  -- Increment counter to allow measurement of reaction times in steps
  -- This function is automatically called at each tick.
  function env:step(lookingAtScreen)
    if self.currentTrial.responseSteps ~= nil then
      self.currentTrial.responseSteps = self.currentTrial.responseSteps + 1
    end
  end

  return psychlab_factory.createLevelApi{
      env = point_and_click,
      envOpts = {environment = env, screenSize = kwargs.screenSize,
                 maxStepsOffScreen = kwargs.maxStepsOffScreen},
      episodeLengthSeconds = kwargs.episodeLengthSeconds
  }
end

return factory
