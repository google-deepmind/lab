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
After a colored patch is displayed for a long/short duration,
saccade left for short and right for long.
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

-- Task screen params
local ANIMATION_SIZE_AS_FRACTION_OF_SCREEN = {0.8, 0.8}
local SCREEN_SIZE = {width = 512, height = 512}
local FIXATION_SIZE = 0.1
local FIXATION_COLOR = {50, 50, 50}
local BG_COLOR = 0
local CENTER = {0.5, 0.5}
local INTERFRAME_INTERVAL = 1 -- In real frames.

-- task params
-- durations
local EPISODE_LENGTH_SECONDS = 300
local TIME_TO_FIXATE_CROSS = 1 -- In frames.
local INTERTRIAL_INTERVAL = 60
local TARGET_DURATIONS = {40, 80} -- In frames.
-- rules
local MAX_STEPS_OFF_SCREEN = 300
local TRIALS_PER_EPISODE_CAP = 50
local RANDOM_TARGET_PLACEMENTS = true
local TARGET_FLASH_DURATION = -1 -- In frames, where -1 means persistent.
-- target aesthetics
local CENTRAL_TARGET_LOCATION = {0.5, 0.5}
local SACCADE_TARGET_DISTANCE = 0.2
local TARGET_SIZE = 0.1
local PROBE_COLOR = {50, 50, 90}
local TARGET_COLORS = {{50, 100, 150}, {150, 10, 30}}
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
  kwargs.bgColor = kwargs.bgColor or BG_COLOR
  kwargs.center = kwargs.center or CENTER
  kwargs.interframeInterval = kwargs.interframeInterval or INTERFRAME_INTERVAL
  kwargs.episodeLengthSeconds = kwargs.episodeLengthSeconds or
      EPISODE_LENGTH_SECONDS
  kwargs.timeToFixateCross = kwargs.timeToFixateCross or TIME_TO_FIXATE_CROSS
  kwargs.intertrialInterval = kwargs.intertrialInterval or INTERTRIAL_INTERVAL
  kwargs.targetDurations = kwargs.targetDurations or TARGET_DURATIONS
  kwargs.maxStepsOffScreen = kwargs.maxStepsOffScreen or MAX_STEPS_OFF_SCREEN
  kwargs.trialsPerEpisodeCap = kwargs.trialsPerEpisodeCap or
      TRIALS_PER_EPISODE_CAP
  kwargs.randomTargetPlacements = kwargs.randomTargetPlacements or
      RANDOM_TARGET_PLACEMENTS
  kwargs.targetFlashDuration = kwargs.targetFlashDuration or
      TARGET_FLASH_DURATION
  kwargs.centralTargetLocation = kwargs.centralTargetLocation or
      CENTRAL_TARGET_LOCATION
  kwargs.saccadeTargetDistance = kwargs.saccadeTargetDistance or
      SACCADE_TARGET_DISTANCE
  kwargs.targetSize = kwargs.targetSize or TARGET_SIZE
  kwargs.probeColor = kwargs.probeColor or PROBE_COLOR
  kwargs.targetColors = kwargs.targetColors or TARGET_COLORS
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
    self:setupImages()
    self:setupCoordinateBounds(kwargs.canvasSize, kwargs.preRenderMaskSize)

    local function targetPosition(angle)
      local h = kwargs.saccadeTargetDistance
      local hSize = kwargs.targetSize / 2
      local center = kwargs.center
      return {center[1] + (h * math.cos(angle)) - hSize,
          1 - (center[2] + (h * math.sin(angle)) + hSize)}
    end

    self.targetPositions = {targetPosition(math.pi), targetPosition(0)}
    self.pac = pac
  end

  function env:setupImages()
    self.images = {}

    self.images.fixation = psychlab_helpers.getFixationImage(self.screenSize,
        kwargs.bgColor, kwargs.fixationColor, kwargs.fixationSize)
  end

  -- trial methods --

  function env:runTrial()
    -- Select timing for this trial randomly.
    self._trialDurIdx = psychlab_helpers.randomFrom({1, 2})
    self.currentTrial.durationIdx = self._trialDurIdx - 1
    self._targetDur = kwargs.targetDurations[self._trialDurIdx]

    self:widgetsOff({'fixation', 'center_of_fixation'})

    events:add('task_events', "target_on_duration" ..
        tostring(self._trialDurIdx - 1))

    local target = tensor.ByteTensor(self.screenSize.height * kwargs.targetSize,
        self.screenSize.width * kwargs.targetSize, 3):fill(kwargs.probeColor)
    self.pac:addWidget{
        name = 'target',
        image = target,
        pos = psychlab_helpers.getUpperLeftFromCenter(
            kwargs.centralTargetLocation, kwargs.targetSize),
        size = {kwargs.targetSize, kwargs.targetSize},
    }

    local timer_dur = self._targetDur
    if type(timer_dur) == "table" then
      -- Draw duration from specified distribution.
      timer_dur = random:normalDistribution(unpack(timer_dur))
    end
    self.currentTrial.duration = timer_dur

    -- Account for the option of flashing the target.
    local callback = nil
    if kwargs.targetFlashDuration == -1 then
      callback = function(...) return self.selectionPhase(self) end
    else
      timer_dur = kwargs.targetFlashDuration
      callback = function(...) return self.interFlashPhase(self, target) end
    end

    self.pac:addTimer{
        name = 'target_timer',
        timeout = timer_dur,
        callback = callback}
  end

  function env:interFlashPhase(target)
    self:widgetsOff({'target'})

    -- To maintain equal intervals in the flash and no-flash
    -- conditions, 2nd flash *ends* at the trial interval.
    local interDuration = self.currentTrial.duration - 2 *
        kwargs.targetFlashDuration
    self.pac:addTimer{
        name = 'target_timer2',
        timeout = interDuration,
        callback = function(...) return self.secondFlashPhase(self, target) end
    }
  end

  function env:secondFlashPhase(target)
    self.pac:addWidget{
        name = 'target',
        image = target,
        pos = psychlab_helpers.getUpperLeftFromCenter(
            kwargs.centralTargetLocation,
        kwargs.targetSize),
        size = {kwargs.targetSize, kwargs.targetSize},
    }

    self.pac:addTimer{
        name = 'target_timer3',
        timeout = kwargs.targetFlashDuration,
        callback = function(...) return self.selectionPhase(self) end
    }
  end

  function env:prepareSelectionTargets()
    local targetLong = tensor.ByteTensor(self.screenSize.height
      * kwargs.targetSize, self.screenSize.width * kwargs.targetSize,
      3):fill(kwargs.targetColors[2])
    local targetShort = tensor.ByteTensor(self.screenSize.height
      * kwargs.targetSize, self.screenSize.width * kwargs.targetSize,
      3):fill(kwargs.targetColors[1])

    return targetLong, targetShort
  end

  function env:selectionPhase()
    self:widgetsOff({'target'})
    events:add('task_events', "target_off")
    local targetLong, targetShort = self:prepareSelectionTargets()

    local targetCorrect = nil
    local targetIncorrect = nil
    if self._trialDurIdx == 1 then -- short trial
      targetCorrect = targetShort
      targetIncorrect = targetLong
    elseif self._trialDurIdx == 2 then -- long trial
      targetCorrect = targetLong
      targetIncorrect = targetShort
    end

    -- Select the axis of positions for this trial.
    local posCorrectIdx = nil
    if kwargs.randomTargetPlacements == true then
      posCorrectIdx = psychlab_helpers.randomFrom({1, 2})
    else
      posCorrectIdx = self._trialDurIdx
    end
    local posCorrect = self.targetPositions[posCorrectIdx]
    local posIncorrect = self.targetPositions[-(posCorrectIdx - 1) + 2]
    self.currentTrial.positionCorrect = posCorrectIdx

    self.pac:addWidget{
        name = 'target_correct',
        image = targetCorrect,
        pos = posCorrect,
        size = {kwargs.targetSize, kwargs.targetSize},
        mouseHoverCallback = self.correctCallback,
    }

    self.pac:addWidget{
        name = 'target_incorrect',
        image = targetIncorrect,
        pos = posIncorrect,
        size = {kwargs.targetSize, kwargs.targetSize},
        mouseHoverCallback = self.incorrectCallback,
    }

  end

  function env:finishTrial(delay)
    self.currentTrial.blockId = self.blockId
    self.currentTrial.episodeId = self.episodeId
    self.currentTrial.reactionTime =
        game:episodeTimeSeconds() - self._currentTrialStartTime

    self.pac:resetSteps()

    local trialData = helpers.tostring(self.currentTrial)
    events:add('task_events', 'trial_end')
    events:add('task_events', trialData, kwargs.schema)

    psychlab_helpers.finishTrialCommon(self, delay, kwargs.fixationSize)
  end

  -- callbacks --

  function env:correctCallback(name, mousePos, hoverTime, userData)
    self.pac:addReward(kwargs.correctReward)
    self.currentTrial.outcome = 1
    events:add('task_events', 'reward')
    self:finishTrial(kwargs.intertrialInterval)
  end

  function env:incorrectCallback(name, mousePos, hoverTime, userData)
    self.pac:addReward(kwargs.incorrectReward)
    self.currentTrial.outcome = 0
    events:add('task_events', 'error')
    self:finishTrial(kwargs.intertrialInterval)
  end

  function env:fixationCallback(name, mousePos, hoverTime, userData)
    if hoverTime == kwargs.timeToFixateCross then
      self.currentTrial.stepCount = 0
      self.currentTrial.outcome = -1
      self.pac:addReward(kwargs.fixationReward)
      self._currentTrialStartTime = game:episodeTimeSeconds()
      events:add('task_events', 'fixated')
      self:runTrial()
    end
  end

  function env:step(lookingAtScreen)
    if self.currentTrial.stepCount ~= nil then
      self.currentTrial.stepCount = self.currentTrial.stepCount + 1
    end
  end

  -- helpers --
  function env:widgetsOff(widgets)
    for i, w in ipairs(widgets) do
      self.pac:removeWidget(w)
    end
  end

  function env:removeArray()
    self.pac:removeWidget('target_correct')
    self.pac:removeWidget('target_incorrect')
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

  return psychlab_factory.createLevelApi{
      env = point_and_click,
      envOpts = {environment = env, screenSize = kwargs.screenSize,
          maxStepsOffScreen = kwargs.maxStepsOffScreen},
      episodeLengthSeconds = kwargs.episodeLengthSeconds
  }
end

return factory
