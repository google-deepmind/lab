--[[ Copyright (C) 2019 Google Inc.

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
Like the Jayazeri 2015 RSG reproduction timing task (citation below):
produce a Set-Go interval matched in duration to the
presented Ready-Set interval.

Paper citation: Jazayeri M, Shadlen MN. A Neural Mechanism for Sensing and
Reproducing a Time Interval. Curr Biol. 2015;25(20):2599-609.
]]

local custom_observations = require "decorators.custom_observations"
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
local BUTTON_SIZE = 0.1
local BG_COLOR = 0
local CENTER = {0.5, 0.5}
local INTERFRAME_INTERVAL = 1 -- In real frames.

-- task params
-- durations
local EPISODE_LENGTH_SECONDS = 300
local TIME_TO_FIXATE_CROSS = 1 -- In frames.
local FIXATION_BREAK_THRESH = 10
local PRE_RSG_DELAYS = {10} -- "Variable foreperiod" in paper.
local TARGET_DISPLAY_TIME = 100 -- 0.5 s in paper.
local RSG_INTERVALS = {200, 300} -- In ascending order.
local PROBE_INTERVALS = {25, 375} -- In ascending order.
local PROBE_PROBABILITY = 0.08
local RSG_FLASH_DURATION = 10
local INTERTRIAL_INTERVAL = 60
local BASE_TOLERANCE = 8
-- Scale width of reward window proportionally to target interval with
-- constant TOLERANCE_SCALING to account for scalar variability in responses.
local TOLERANCE_SCALING = 0.0
local TIMEOUT_FACTOR = 5.0
-- rules
local MAX_STEPS_OFF_SCREEN = 300
local TRIALS_PER_EPISODE_CAP = 50
local FIXATION_HOLD_RULE = false
local FIXATION_HOLD_P = 0.15
-- staircase
local INTERVAL_TOLERANCE_FACTORS = {2.5, 1.5, 1.0}
local N_TO_PROMOTE = 2
-- target aesthetics
local N_POSITIONS = 4
local TARGET_DISTANCE = 0.4
local TARGET_SIZE = 0.1
local READY_COLOR = {255, 10, 10}
local SET_COLOR = {255, 255, 10}
local GO_COLOR = {10, 200, 10}
-- rewards
local FIXATION_REWARD = 0
local CORRECT_REWARD = 1
local INCORRECT_REWARD = 0

local factory = {}

function factory.createLevelApi(kwargs)
  kwargs.canvasSize = kwargs.canvasSize or CANVAS_SIZE
  kwargs.preRenderMaskSize = kwargs.preRenderMaskSize or PRE_RENDER_MASK_SIZE
  kwargs.animationSizeAsFractionOfScreen =
  kwargs.animationSizeAsFractionOfScreen or ANIMATION_SIZE_AS_FRACTION_OF_SCREEN
  kwargs.screenSize = kwargs.screenSize or SCREEN_SIZE
  kwargs.fixationSize = kwargs.fixationSize or FIXATION_SIZE
  kwargs.fixationColor = kwargs.fixationColor or FIXATION_COLOR
  kwargs.buttonSize = kwargs.buttonSize or BUTTON_SIZE
  kwargs.bgColor = kwargs.bgColor or BG_COLOR
  kwargs.center = kwargs.center or CENTER
  kwargs.interframeInterval = kwargs.interframeInterval or INTERFRAME_INTERVAL
  kwargs.episodeLengthSeconds = kwargs.episodeLengthSeconds or
      EPISODE_LENGTH_SECONDS
  kwargs.timeToFixateCross = kwargs.timeToFixateCross or TIME_TO_FIXATE_CROSS
  kwargs.fixationBreakThresh = kwargs.fixationBreakThresh or
  FIXATION_BREAK_THRESH
  kwargs.preRsgDelays = kwargs.preRsgDelays or PRE_RSG_DELAYS
  kwargs.targetDisplayTime = kwargs.targetDisplayTime or TARGET_DISPLAY_TIME
  kwargs.rsgIntervals = kwargs.rsgIntervals or RSG_INTERVALS
  kwargs.probeIntervals = kwargs.probeIntervals or PROBE_INTERVALS
  kwargs.probeProbability = kwargs.probeProbability or PROBE_PROBABILITY
  kwargs.rsgFlashDuration = kwargs.rsgFlashDuration or RSG_FLASH_DURATION
  kwargs.intertrialInterval = kwargs.intertrialInterval or INTERTRIAL_INTERVAL
  kwargs.baseTolerance = kwargs.baseTolerance or BASE_TOLERANCE
  kwargs.toleranceScaling = kwargs.toleranceScaling or TOLERANCE_SCALING
  kwargs.timeoutFactor = kwargs.timeoutFactor or TIMEOUT_FACTOR
  kwargs.maxStepsOffScreen = kwargs.maxStepsOffScreen or MAX_STEPS_OFF_SCREEN
  kwargs.trialsPerEpisodeCap = kwargs.trialsPerEpisodeCap or
      TRIALS_PER_EPISODE_CAP
  kwargs.fixationHoldRule = kwargs.fixationHoldRule or
      FIXATION_HOLD_RULE
  kwargs.fixationHoldP = kwargs.fixationHoldP or
      FIXATION_HOLD_P
  kwargs.intervalToleranceFactors = kwargs.intervalToleranceFactors or
      INTERVAL_TOLERANCE_FACTORS
  kwargs.nToPromote = kwargs.nToPromote or N_TO_PROMOTE
  kwargs.nPositions = kwargs.nPositions or N_POSITIONS
  kwargs.targetDistance = kwargs.targetDistance or TARGET_DISTANCE
  kwargs.targetSize = kwargs.targetSize or TARGET_SIZE
  kwargs.readyColor = kwargs.readyColor or READY_COLOR
  kwargs.setColor = kwargs.setColor or SET_COLOR
  kwargs.goColor = kwargs.goColor or GO_COLOR
  kwargs.fixationReward = kwargs.fixationReward or FIXATION_REWARD
  kwargs.correctReward = kwargs.correctReward or CORRECT_REWARD
  kwargs.incorrectReward = kwargs.incorrectReward or INCORRECT_REWARD

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

    self._stepsSinceInteraction = 0
    self._trialBegan = false
    self._fixationBrokenFrames = 0

    self:setupImages()
    self:setupCoordinateBounds(kwargs.canvasSize, kwargs.preRenderMaskSize)

    self.targetPositions = {}
    local ypos = kwargs.center[2] - kwargs.targetSize * 3
    self.targetPositions[1] = {kwargs.center[1] - 1.5 * kwargs.targetSize, ypos}
    self.targetPositions[2] = {kwargs.center[1] - 0.5 * kwargs.targetSize, ypos}
    self.targetPositions[3] = {kwargs.center[1] + 0.5 * kwargs.targetSize, ypos}

    local maxInterval = kwargs.rsgIntervals[#kwargs.rsgIntervals]
    local maxProbe = kwargs.probeIntervals[#kwargs.probeIntervals]
    local maxDuration = math.max(maxInterval, maxProbe)
    self.globalTimeout = kwargs.timeoutFactor * maxDuration

    self.pac = pac
  end

  function env:setupImages()
    self.images = {}
    self.images.fixation = psychlab_helpers.getFixationImage(self.screenSize,
        kwargs.bgColor, kwargs.fixationColor, kwargs.fixationSize)
  end

  -- trial methods --

  function env:preReadyPhase()
    self._currentTrialStartTime = game:episodeTimeSeconds()
    self.goTimeSteps = -1

    -- Determine pre-RSG interval timing for this trial.
    local predelay, prIdx = psychlab_helpers.randomFrom(kwargs.preRsgDelays)
    predelay = kwargs.targetDisplayTime + predelay

    -- Determine RSG interval timing for this trial.
    local r = random:uniformReal(0.0, 1.0)
    local interval = nil
    local intervalIdx = nil
    if r < kwargs.probeProbability then
      interval, intervalIdx = psychlab_helpers.randomFrom(kwargs.probeIntervals)
      self.currentTrial.interval = interval
      self.currentTrial.intervalIdx = intervalIdx
      self.currentTrial.isProbe = true
    else
      interval, intervalIdx = psychlab_helpers.randomFrom(kwargs.rsgIntervals)
      self.currentTrial.interval = interval
      self.currentTrial.intervalIdx = intervalIdx
      self.currentTrial.isProbe = false
    end

    self:log('preready')
    self:log('trial_duration_idx_' .. tostring(intervalIdx - 1))

    -- Determine "go" widget position, which will determine all others.
    local idx = 3
    local go_pos = self.targetPositions[idx]

    -- Add "go" widget.
    local go = tensor.ByteTensor(self.screenSize.height * kwargs.targetSize,
        self.screenSize.width * kwargs.targetSize, 3):fill(kwargs.goColor)
    self.pac:addWidget{
        name = 'go',
        image = go,
        pos = go_pos,
        size = {kwargs.targetSize, kwargs.targetSize},
        mouseHoverCallback = self.goCallback,
    }

    -- Determine "ready" widget position based on "go" position.
    local ready_idx = idx - 2
    if ready_idx < 1 then
        ready_idx = ready_idx + kwargs.nPositions
    end

    -- Start timer to trigger "ready" phase.
    self.pac:addTimer{
        name = 'ready_timer',
        timeout = predelay,
        callback = function(...) return
            self.readyPhase(self, ready_idx, interval) end
    }

    -- Start timeout timer.
    self.pac:addTimer{
        name = 'timeout',
        timeout = self.globalTimeout,
        callback = function(...) return self.timeout(self) end
      }
  end

  function env:readyPhase(idx, interval)
    self:log("ready")

    local ready_pos = self.targetPositions[idx]

    -- Add "ready" widget.
    local ready = tensor.ByteTensor(self.screenSize.height * kwargs.targetSize,
    self.screenSize.width * kwargs.targetSize, 3):fill(kwargs.readyColor)
    self.pac:addWidget{
        name = 'ready',
        image = ready,
        pos = ready_pos,
        size = {kwargs.targetSize, kwargs.targetSize},
    }

    -- Determine "set" widget position based on "ready" position.
    local set_idx = idx + 1
    if set_idx > kwargs.nPositions then
      set_idx = set_idx - kwargs.nPositions
    end

    -- Start timer to flash off "ready" symbol.
    self.pac:addTimer{
        name = 'ready_off_timer',
        timeout = kwargs.rsgFlashDuration,
        callback = function(...) return self.widgetsOff(self, {'ready'}) end
    }

    -- Start timer to trigger "set" phase.
    self.pac:addTimer{
        name = 'set_timer',
        timeout = interval,
        callback = function(...)
        return self.setPhase(self, set_idx, interval) end
    }
  end

  function env:setPhase(idx, interval)
    self:log("set")
    local pos = self.targetPositions[idx]

    -- Add "set" widget.
    local set = tensor.ByteTensor(self.screenSize.height * kwargs.targetSize,
    self.screenSize.width * kwargs.targetSize, 3):fill(kwargs.setColor)
    self.pac:addWidget{
        name = 'set',
        image = set,
        pos = pos,
        size = {kwargs.targetSize, kwargs.targetSize},
    }

    -- Start timer to flash off "set" symbol and fixation cross.
    self.pac:addTimer{
        name = 'set_off_timer',
        timeout = kwargs.rsgFlashDuration,
        callback = function(...) return self.widgetsOff(self,
            {'set', 'fixation', 'center_of_fixation'}) end
    }

    -- Now it's the go phase - i.e. we start timing from the onset of
    -- the "set" phase in order to measure the set-go interval.
    self._fixationRequired = false
    self.goTime = game:episodeTimeSeconds()
    self.goTimeSteps = self.pac:elapsedSteps()
  end

  function env:fixationBroken()
    if self._fixationHoldRule then
      self:removeArray()
      self:finishTrial(kwargs.intertrialInterval)
    end
  end

  function env:timeout()
    local elapsed = self.pac:elapsedSteps() - self.goTimeSteps

    self.currentTrial.outcome = -1
    self.currentTrial.interval = elapsed
    self:log('timeout')
    self:finishTrial(kwargs.intertrialInterval)
  end

  function env:finishTrial(delay)
    self.pac:removeWidget('go')
    self:log('trial_end')

    self._stepsSinceInteraction = 0
    self._fixationRequired = false
    self._trialBegan = false
    self.currentTrial.blockId = self.blockId
    self.currentTrial.episodeId = self.episodeId
    self.currentTrial.reactionTime =
        game:episodeTimeSeconds() - self._currentTrialStartTime

    -- Consider promotion of difficulty.
    local ii = self.currentTrial.intervalIdx
    if (self.currentTrial.isProbe == false) and
        (self.levelNCorrect[ii] >= kwargs.nToPromote) then
      log.info('Promoting...')
      self.intervalToleranceIdx[ii] = self.intervalToleranceIdx[ii] + 1
      if self.intervalToleranceIdx[ii] > #kwargs.intervalToleranceFactors then
        self.intervalToleranceIdx[ii] = #kwargs.intervalToleranceFactors
      end
      self.levelNCorrect[ii] = 0
    end

    self.pac:resetSteps()

    local trialData = helpers.tostring(self.currentTrial)
    events:add('task_events', trialData, kwargs.schema)

    psychlab_helpers.finishTrialCommon(self, delay, kwargs.fixationSize)
  end

  -- callbacks --

  function env:fixationCallback(name, mousePos, hoverTime, userData)
    if hoverTime == kwargs.timeToFixateCross
          and self._trialBegan ~= true then
      self.trialId = self.trialId + 1
      self.currentTrial.trialId = self.trialId
      self:log('fixated')
      self:preReadyPhase()
      self.currentTrial.stepCount = 0
      self.pac:addReward(kwargs.fixationReward)
      self._stepsSinceInteraction = 0
      self._fixationRequired = true
      self._trialBegan = true
      self._fixationBrokenFrames = 0

      if kwargs.fixationHoldRule and random:uniformReal(0.0, 1.0) <
          kwargs.fixationHoldP then
        self._fixationHoldRule = true
      else
        self._fixationHoldRule = false
      end
      self.currentTrial.fixationHoldRule = self._fixationHoldRule
    end

    if self._fixationRequired == true then
      self._fixationBrokenFrames = 0
    end
  end

  function env:step(lookingAtScreen)
    -- Auto-called at each tick; increment counter to
    -- allow measurement of reaction times in steps.
    if self.currentTrial.stepCount ~= nil then
      self.currentTrial.stepCount = self.currentTrial.stepCount + 1
    end

    if self._fixationRequired == true then
      if self._fixationBrokenFrames > kwargs.fixationBreakThresh then
        self:fixationBroken()
      end
      self._fixationBrokenFrames = self._fixationBrokenFrames + 1
    end

    for playerId, inv in pairs(custom_observations.playerInventory) do
      local v, h, _ = unpack(inv:eyeAngles())
      self:logEyes(v, h)
    end
  end

  function env:goCallback(name, mousePos, hoverTime, userData)
    self:log('response_made')
    if self.goTimeSteps == -1 then
      self.currentTrial.outcome = -1
      log.info('early selection')
    else
      local elapsed = self.pac:elapsedSteps() - self.goTimeSteps
      local dif = elapsed - self.currentTrial.interval
      local abs_dif = math.abs(dif)

      self.currentTrial.response = name
      local tolerance = kwargs.baseTolerance +
          kwargs.toleranceScaling * self.currentTrial.interval

      -- Increment staircase.
      if self.currentTrial.isProbe == false then
        local ii = self.currentTrial.intervalIdx
        tolerance = kwargs.intervalToleranceFactors[
            self.intervalToleranceIdx[ii]] * tolerance
      end

      self.currentTrial.produced = elapsed
      self.currentTrial.temporal_difference = dif
      self.currentTrial.tolerance = tolerance

      if abs_dif < tolerance then
        self.currentTrial.outcome = 1
        self.pac:addReward(kwargs.correctReward)
        if self.currentTrial.isProbe == false then
          local ii = self.currentTrial.intervalIdx
          self.levelNCorrect[ii] = self.levelNCorrect[ii] + 1
        end
        log.info('+1 reward')
        self:log('reward')
      else
        self.currentTrial.outcome = 0
        self.pac:addReward(kwargs.incorrectReward)
        log.info('no reward')
        self:log('error')
      end
    end
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
    self.pac:removeWidget('fixation')
    self.pac:removeWidget('center_of_fixation')
    self.pac:removeWidget('ready')
    self.pac:removeWidget('set')
    self.pac:removeWidget('go')
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

    -- Setup staircase.
    self.levelNCorrect = {}
    self.intervalToleranceIdx = {}
    for i = 1, #kwargs.rsgIntervals do
      self.intervalToleranceIdx[i] = 1
      self.levelNCorrect[i] = 0
    end

    self.blockId = seed
    self.episodeId = episodeId
    self.trialId = 0
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

  function env:log(message)
    events:add('task_events', string.format(
        'block_%d_episode_%d_trial_%d_%s',
        self.blockId, self.episodeId, self.trialId, message))
  end

  function env:logEyes(v, h)
    events:add('eyes', string.format(
        'block_%d_episode_%d_trial_%d_%f-%f',
        self.blockId, self.episodeId, self.trialId, v, h))
  end

  return psychlab_factory.createLevelApi{
      env = point_and_click,
      envOpts = {environment = env, screenSize = kwargs.screenSize,
          maxStepsOffScreen = kwargs.maxStepsOffScreen},
      episodeLengthSeconds = kwargs.episodeLengthSeconds
  }
end

return factory
