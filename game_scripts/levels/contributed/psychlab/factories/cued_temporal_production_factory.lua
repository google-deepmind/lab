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

--[[ Task description:
"Click" the target twice with a specific temporal delay
between clicks 1 and 2. The temporal delay is one of two
and depends on the color of the target.
]]

local events = require 'dmlab.system.events'
local game = require 'dmlab.system.game'
local helpers = require 'common.helpers'
local log = require 'common.log'
local point_and_click = require 'factories.psychlab.point_and_click'
local psychlab_factory = require 'factories.psychlab.factory'
local psychlab_helpers = require 'factories.psychlab.helpers'
local random = require 'common.random'
local tensor = require 'dmlab.system.tensor'

-- display params
local CANVAS_SIZE = 2048
local PRE_RENDER_MASK_SIZE = 1224

-- task screen params
local ANIMATION_SIZE_AS_FRACTION_OF_SCREEN = {0.8, 0.8}
local SCREEN_SIZE = {width = 512, height = 512}
local FIXATION_SIZE = 0.1
local FIXATION_COLOR = {50, 50, 50}
local BG_COLOR = 0
local CENTER = {0.5, 0.5}
local INTERFRAME_INTERVAL = 1 -- In real frames.

-- task params
-- phase constants
local PHASE_PRE_BEGIN = 0
local PHASE_PRE_FIRST_CLICK = 1
local PHASE_PRE_SECOND_CLICK = 2
-- durations
local EPISODE_LENGTH_SECONDS = 300
local TIME_TO_FIXATE_CROSS = 1 -- In frames.
local TARGET_INTERVALS = {40, 80}
local TIMEOUT_FACTOR = 3
local INTERTRIAL_INTERVAL = 60
local WAIT_COMPLEMENT = false
-- rules
local MAX_STEPS_OFF_SCREEN = 300
local TRIALS_PER_EPISODE_CAP = 50
-- target aesthetics
local TARGET_SIZE = 0.1
local TARGET_LOCATION = {0.5, 0.25}
local TARGET_LONG_COLOR = {50, 200, 230}
local TARGET_LONG_COLOR_2 = {200, 230, 50}
local TARGET_SHORT_COLOR = {230, 10, 190}
local TARGET_SHORT_COLOR_2 = {200, 230, 50}
-- rewards
local FIXATION_REWARD = 0
local CORRECT_REWARD = 1
local INCORRECT_REWARD = 0
-- staircase
local INTERVAL_TOLERANCE_FACTORS = {0.8, 0.5, 0.3, 0.1, 0.04}
local NUMBER_TO_PROMOTE = 2

local factory = {}

function factory.createLevelApi(kwargs)
  kwargs.canvasSize = kwargs.canvasSize or CANVAS_SIZE
  kwargs.preRenderMaskSize = kwargs.preRenderMaskSize or PRE_RENDER_MASK_SIZE
  kwargs.animationSizeAsFractionOfScreen =
  kwargs.animationSizeAsFractionOfScreen or ANIMATION_SIZE_AS_FRACTION_OF_SCREEN
  kwargs.screenSize = kwargs.screenSize or SCREEN_SIZE
  kwargs.fixationSize = kwargs.fixationSize or FIXATION_SIZE
  kwargs.fixationColor = kwargs.fixationColor or FIXATION_COLOR
  kwargs.bgColor = kwargs.bgColor or BG_COLOR
  kwargs.center = kwargs.center or CENTER
  kwargs.interframeInterval = kwargs.interframeInterval or INTERFRAME_INTERVAL
  kwargs.episodeLengthSeconds = kwargs.episodeLengthSeconds or
      EPISODE_LENGTH_SECONDS
  kwargs.timeToFixateCross = kwargs.timeToFixateCross or TIME_TO_FIXATE_CROSS
  kwargs.targetIntervals = kwargs.targetIntervals or TARGET_INTERVALS
  kwargs.timeoutFactor = kwargs.timeoutFactor or TIMEOUT_FACTOR
  kwargs.intertrialInterval = kwargs.intertrialInterval or INTERTRIAL_INTERVAL
  kwargs.waitComplement = kwargs.waitComplement or WAIT_COMPLEMENT
  kwargs.maxStepsOffScreen = kwargs.maxStepsOffScreen or MAX_STEPS_OFF_SCREEN
  kwargs.trialsPerEpisodeCap = kwargs.trialsPerEpisodeCap or
      TRIALS_PER_EPISODE_CAP
  kwargs.targetSize = kwargs.targetSize or TARGET_SIZE
  kwargs.targetLocation = kwargs.targetLocation or TARGET_LOCATION
  kwargs.targetLongColor = kwargs.targetLongColor or TARGET_LONG_COLOR
  kwargs.targetLongColor2 = kwargs.targetLongColor2 or TARGET_LONG_COLOR_2
  kwargs.targetShortColor = kwargs.targetShortColor or TARGET_SHORT_COLOR
  kwargs.targetShortColor2 = kwargs.targetShortColor2 or TARGET_SHORT_COLOR_2
  kwargs.fixationReward = kwargs.fixationReward or FIXATION_REWARD
  kwargs.correctReward = kwargs.correctReward or CORRECT_REWARD
  kwargs.incorrectReward = kwargs.incorrectReward or INCORRECT_REWARD
  kwargs.intervalToleranceFactors = kwargs.intervalToleranceFactors or
      INTERVAL_TOLERANCE_FACTORS
  kwargs.numberToPromote = kwargs.numberToPromote or NUMBER_TO_PROMOTE

  local env = {}
  env.__index = env

  setmetatable(env, {
      __call = function (cls, ...)
        local self = setmetatable({}, cls)
        self:_init(...)
        return self
      end
  })

  -- init methods --

  function env:_init(pac, opts)
    self.screenSize = opts.screenSize
    self:setupImages()
    self:setupCoordinateBounds(kwargs.canvasSize, kwargs.preRenderMaskSize)

    self._trialPhase = PHASE_PRE_BEGIN
    self._click_t0 = nil

    self.pac = pac
  end

  function env:setupImages()
    self.images = {}
    self.images.fixation = psychlab_helpers.getFixationImage(self.screenSize,
        kwargs.bgColor, kwargs.fixationColor, kwargs.fixationSize)
  end

  -- trial methods --

  function env:showTarget()

    self:widgetsOff({'fixation', 'center_of_fixation'})

    local firstColors = {kwargs.targetShortColor, kwargs.targetLongColor}
    local targetColor = firstColors[self.currentTrial.targetIntervalIdx]
    local target = tensor.ByteTensor(self.screenSize.height * kwargs.targetSize,
        self.screenSize.width * kwargs.targetSize, 3):fill(targetColor)
    self.pac:addWidget{
        name = 'target',
        image = target,
        pos = psychlab_helpers.getUpperLeftFromCenter(kwargs.targetLocation,
            kwargs.targetSize),
        size = {kwargs.targetSize, kwargs.targetSize},
        mouseClickCallback = self.clickCallback}

    events:add('task_events', "target_on_duration" ..
        tostring(self.currentTrial.targetIntervalIdx - 1))

  end

  function env:finishTrial(delay)
    self._trialPhase = PHASE_PRE_BEGIN
    self._click_t0 = nil
    self.currentTrial.blockId = self.blockId
    self.currentTrial.episodeId = self.episodeId
    self.currentTrial.reactionTime =
        game:episodeTimeSeconds() - self.currentTrial.trialStartTime
    events:add('task_events', "trial_end")

    if self.levelNumberCorrect[self.currentTrial.targetIntervalIdx]
        >= kwargs.numberToPromote then
      log.info('Promoting...')
      self.intervalToleranceIdx[self.currentTrial.targetIntervalIdx] =
      self.intervalToleranceIdx[self.currentTrial.targetIntervalIdx] + 1
      if self.intervalToleranceIdx[self.currentTrial.targetIntervalIdx] >
          #kwargs.intervalToleranceFactors then
        self.intervalToleranceIdx[self.currentTrial.targetIntervalIdx] =
            #kwargs.intervalToleranceFactors
      end
      self.levelNumberCorrect[self.currentTrial.targetIntervalIdx] = 0
    end

    self.pac:resetSteps()

    local trialData = helpers.tostring(self.currentTrial)
    events:add('task_events', trialData, kwargs.schema)

    psychlab_helpers.finishTrialCommon(self, delay, kwargs.fixationSize)
    self.pac:removeWidget('target')
  end

  -- callbacks --

  function env:fixationCallback(name, mousePos, hoverTime, userData)
    if hoverTime == kwargs.timeToFixateCross and
        self._trialPhase == PHASE_PRE_BEGIN then
          self.currentTrial.stepCount = 0
          self.currentTrial.outcome = -1
          events:add('task_events', "fixated")
          self.pac:addReward(kwargs.fixationReward)
          self._stepsSinceInteraction = 0
          self._trialPhase = PHASE_PRE_FIRST_CLICK
          self.currentTrial.trialStartTime = game:episodeTimeSeconds()
          self.currentTrial.trialStartStep = self.pac:elapsedSteps()
          self.currentTrial.targetIntervalIdx =
              psychlab_helpers.randomFrom({1, 2})
          self.currentTrial.targetIntervalSteps = kwargs.targetIntervals[
              self.currentTrial.targetIntervalIdx]
          self.currentTrial.intervalTolerance = kwargs.intervalToleranceFactors[
              self.intervalToleranceIdx[
              self.currentTrial.targetIntervalIdx]] *
              self.currentTrial.targetIntervalSteps
          self:showTarget()
    end

  end

  function env:step(lookingAtScreen)
    if self.currentTrial.stepCount ~= nil then
      self.currentTrial.stepCount = self.currentTrial.stepCount + 1
    end
  end

  function env:clickCallback(name, mousePos, hoverTime, userData)
      if self._trialPhase == PHASE_PRE_FIRST_CLICK then
        local secondColors = {kwargs.targetShortColor2,
            kwargs.targetLongColor2}
        local targetColor = secondColors[self.currentTrial.targetIntervalIdx]
        local new_target = tensor.ByteTensor(self.screenSize.height *
            kwargs.targetSize, self.screenSize.width * kwargs.targetSize,
            3):fill(targetColor)
        self.pac:updateWidget("target", new_target)
        self._trialPhase = PHASE_PRE_SECOND_CLICK
        self._click_t0 = self.pac:elapsedSteps()
        self.currentTrial.click1Step = self.pac:elapsedSteps()
        self.currentTrial.click1StepTrial = self.currentTrial.stepCount
        events:add("task_events", "click_1")

        -- timeout timer
        self.pac:addTimer{
            name = "timeout",
            timeout = kwargs.timeoutFactor * kwargs.targetIntervals[2],
            callback = function(...) self:timeout() end}

      elseif self._trialPhase == PHASE_PRE_SECOND_CLICK then
        -- end trial
        local elapsed = self.pac:elapsedSteps() - self._click_t0
        local dif = elapsed - self.currentTrial.targetIntervalSteps
        local abs_dif = math.abs(dif)
        local extraWait = 0
        if dif < 0 and kwargs.waitComplement == true then
          extraWait = -dif
        end

        if abs_dif < self.currentTrial.intervalTolerance then
          self.currentTrial.outcome = 1
          self.currentTrial.trialReward = kwargs.correctReward
          events:add("task_events", "reward")
          self.levelNumberCorrect[self.currentTrial.targetIntervalIdx] =
              self.levelNumberCorrect[self.currentTrial.targetIntervalIdx] + 1
        else
          self.currentTrial.outcome = 0
          self.currentTrial.trialReward = kwargs.incorrectReward
        end
        self.pac:addReward(self.currentTrial.trialReward)
        self.currentTrial.click2Step = self.pac:elapsedSteps()
        self.currentTrial.click2StepTrial = self.currentTrial.stepCount
        self.currentTrial.interval = elapsed
        events:add("task_events", "click_2")
        self:finishTrial(kwargs.intertrialInterval + extraWait)
      end
  end

  function env:timeout()
    local elapsed = self.pac:elapsedSteps() - self._click_t0
    self.currentTrial.outcome = -1
    self.currentTrial.trialReward = 0
    self.currentTrial.click2Step = -1
    self.currentTrial.click2StepTrial = -1
    self.currentTrial.interval = elapsed
    events:add("task_events", "timeout")
    self:finishTrial(kwargs.intertrialInterval)
  end

  -- helpers --

  function env:widgetsOff(widgets)
    for i, w in ipairs(widgets) do
      self.pac:removeWidget(w)
    end
  end

  function env:removeArray()
    self.pac:removeWidget('main_image')
    self.pac:clearTimers()
  end

  function env:reset(episodeId, seed, ...)
    random:seed(seed)

    psychlab_helpers.setTrialsPerEpisodeCap(self, kwargs.trialsPerEpisodeCap)

    self.pac:setBackgroundColor{kwargs.bgColor, kwargs.bgColor, kwargs.bgColor}
    self.pac:clearWidgets()

    psychlab_helpers.addFixation(self, kwargs.fixationSize)
    self.reward = 0
    self:initAnimation(kwargs.animationSizeAsFractionOfScreen)

    self.currentTrial = {}

    -- staircase
    self.levelNumberCorrect = {0, 0}
    self.intervalToleranceIdx = {1, 1}

    self.blockId = seed
    self.episodeId = episodeId
  end

  function env:setupCoordinateBounds(canvasSize, preRenderMaskSize)
    local preRenderLowerBound = math.floor((canvasSize - preRenderMaskSize) / 2)
    local preRenderUpperBound = canvasSize - preRenderLowerBound
    local function isAboveLowerBound(coord)
      return coord > preRenderLowerBound
    end
    local function isBelowUpperBound(coord)
      return coord < preRenderUpperBound
    end
    self.isInBounds = function (coord)
      return isAboveLowerBound(coord) and isBelowUpperBound(coord)
    end
  end

  function env:initAnimation(sizeAsFractionOfScreen)
    local imageHeight = sizeAsFractionOfScreen[1] * self.screenSize.height
    local imageWidth = sizeAsFractionOfScreen[2] * self.screenSize.width
    self.animation = {
        currentFrame = tensor.ByteTensor(imageHeight, imageWidth, 3):fill(0),
        nextFrame = tensor.ByteTensor(imageHeight, imageWidth, 3):fill(0),
        imageSize = imageHeight,  -- Assume height and width are the same.
    }
  end

  function env:renderFrame(coords)
    local frame = tensor.Tensor(unpack(self.animation.nextFrame:shape()))
        :fill(kwargs.bgColor)
    return frame
  end

  function env:displayFrame(videoCoords, index)
    -- Show the current frame.
    self.pac:updateWidget('main_image', self.animation.currentFrame)

    -- Recursively call this function after the interframe interval.
    self.pac:addTimer{
        name = 'interframe_interval',
        timeout = kwargs.interframeInterval,
        callback = function(...) self:displayFrame() end
    }

    -- Render the next frame.
    self.animation.nextFrame = self:renderFrame()
    self.animation.currentFrame = self.animation.nextFrame
  end

  return psychlab_factory.createLevelApi{
      env = point_and_click,
      envOpts = {environment = env, screenSize = kwargs.screenSize,
          maxStepsOffScreen = kwargs.maxStepsOffScreen},
      episodeLengthSeconds = kwargs.episodeLengthSeconds
  }
end

return factory
