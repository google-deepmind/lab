--[[ Copyright (C) 2018 Google LLC

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
local psychlab_factory = require 'factories.psychlab.factory'
local psychlab_helpers = require 'factories.psychlab.helpers'
local helpers = require 'common.helpers'
local image = require 'dmlab.system.image'
local point_and_click = require 'factories.psychlab.point_and_click'
local random = require 'common.random'
local tensor = require 'dmlab.system.tensor'
local log = require 'common.log'
local defaults = require 'levels.contributed.psychlab.visuospatial_suite.factories.defaults'

--[[ This factory creates visually and memory guided saccade tasks.

Based on:

Umeno, M. M., & Goldberg, M. E. (1997). Spatial processing in the monkey frontal
eye field. I. Predictive visual responses. Journal of neurophysiology, 78(3),
1373-1383.
]]

local GUIDE_COLOR = {20, 44, 26}
local SUCCESS_COLOR = {0, 223, 0}  -- dark green
local FAILURE_COLOR = {223, 0, 0}  -- dark red

local PRERESPONSE_BUTTON_DIAMETER = 40
local PRERESPONSE_BUTTON_COLOR = {255, 255, 255}  -- white

local NUM_LOCATIONS = 8  -- Must be at least twice maximum sequence length
local LOCATION_RADIUS = 0.7
local GUIDE_SIZE = 0.125

local TRIAL_TYPES = {'pro-cue', 'anti-cue'}
-- In the 'cue_stops' condition, the fixation cue disappears as the stimuli
-- appear. In the 'cue_continues' condition, the fixation cue stays on-screen
-- throughout the trial.
local CONDITION_TYPES = {'cue_stops', 'cue_continues'}

local MEMORY_GUIDED = false
local INITIAL_LOCATION_GUIDE_COLOR = {41, 16, 44}

-- Number of saccades in each trial's sequence.
local SACCADE_SEQUENCE_LENGTHS = {1, 2}

local INITIAL_FIXATION_COLOR = {255, 255, 255} -- RGB, white
local PRO_SACCADE_FIXATION_COLOR = {0, 155, 0} -- RGB, dark green
local ANTI_SACCADE_FIXATION_COLOR = {155, 0, 0} -- RGB, dark red

-- Set image layers for rendering widgets that may occlude one another
local GUIDE_LAYER = 1
local CUE_LAYER = 2
local TARGET_LAYER = 3
local RESULT_LAYER = 4


local factory = {}

function factory.createLevelApi(kwargs)
  kwargs.screenSize = kwargs.screenSize or defaults.SCREEN_SIZE
  kwargs.episodeLengthSeconds = kwargs.episodeLengthSeconds or
      defaults.EPISODE_LENGTH_SECONDS
  kwargs.trialsPerEpisodeCap = kwargs.trialsPerEpisodeCap or
      defaults.TRIALS_PER_EPISODE_CAP
  kwargs.bgColor = kwargs.bgColor or defaults.BG_COLOR
  kwargs.guideColor = kwargs.guideColor or GUIDE_COLOR
  kwargs.successColor = kwargs.successColor or SUCCESS_COLOR
  kwargs.failureColor = kwargs.failureColor or FAILURE_COLOR
  kwargs.colors = kwargs.colors or defaults.COLORS
  kwargs.saccadeSequenceLengths = kwargs.saccadeSequenceLengths or
      SACCADE_SEQUENCE_LENGTHS
  kwargs.locationRadius = kwargs.locationRadius or LOCATION_RADIUS
  kwargs.trialTypes = kwargs.trialTypes or TRIAL_TYPES
  kwargs.conditionTypes = kwargs.conditionTypes or CONDITION_TYPES
  kwargs.memoryGuided = kwargs.memoryGuided or MEMORY_GUIDED
  kwargs.initialLocationGuideColor = kwargs.initialLocationGuideColor or
      INITIAL_LOCATION_GUIDE_COLOR
  kwargs.incorrectReward = kwargs.incorrectReward or defaults.INCORRECT_REWARD
  kwargs.correctReward = kwargs.correctReward or defaults.CORRECT_REWARD
  kwargs.trialTimeout = kwargs.trialTimeout or defaults.TRIAL_TIMEOUT
  kwargs.maxStepsOffScreen = kwargs.maxStepsOffScreen or
      defaults.MAX_STEPS_OFF_SCREEN

  -- Class definition for guided saccade psychlab environment.
  local env = {}
  env.__index = env

  setmetatable(env, {
      __call = function (cls, ...)
        local self = setmetatable({}, cls)
        self:_init(...)
        return self
      end
  })

  -- 'init' gets called at the start of each episode.
  function env:_init(pac, opts)
    log.info('opts passed to _init:\n' .. helpers.tostring(opts))
    self.screenSize = opts.screenSize

    self:setupImages()

    -- Store a copy of the 'point_and_click' api.
    self.pac = pac
  end

  --[[ Reset is called after init. It is called only once per episode.
  Note: the episodeId passed to this function may not be correct if the job
  has resumed from a checkpoint after preemption.
  ]]
  function env:reset(episodeId, seed)
    random:seed(seed)

    self.pac:setBackgroundColor(kwargs.bgColor)
    self.pac:clearWidgets()
    self.pac:clearTimers()
    psychlab_helpers.addFixation(self, defaults.FIXATION_SIZE)

    self.currentTrial = {}

    psychlab_helpers.setTrialsPerEpisodeCap(self, kwargs.trialsPerEpisodeCap)

    -- blockId groups together all rows written during the same episode
    self.blockId = random:uniformInt(1, 2 ^ 32)
  end

  function env:drawCircle(absSize, ballColor)
    local radius = math.floor(absSize / 2)
    local radiusSquared = radius ^ 2
    local ballTensor = tensor.ByteTensor(absSize,
                                         absSize, 3):fill(kwargs.bgColor)
    local ballColor = tensor.ByteTensor(ballColor)
    for x = -radius, radius do
      for y = -radius, radius do
        if x * x + y * y < radiusSquared then
          ballTensor(y + radius + 1, x + radius + 1):copy(ballColor)
        end
      end
    end
    return ballTensor
  end

  function env:setupImages()
    self.images = {}

    self.images.fixation = psychlab_helpers.getFixationImage(
        self.screenSize, kwargs.bgColor, INITIAL_FIXATION_COLOR,
        defaults.FIXATION_SIZE)
    self.images.initialFixation = psychlab_helpers.getFixationImage(
        self.screenSize, kwargs.bgColor, INITIAL_FIXATION_COLOR,
        defaults.FIXATION_SIZE)
    self.images.proSaccadeFixation = psychlab_helpers.getFixationImage(
        self.screenSize, kwargs.bgColor,
        PRO_SACCADE_FIXATION_COLOR, defaults.FIXATION_SIZE)
    self.images.antiSaccadeFixation = psychlab_helpers.getFixationImage(
        self.screenSize, kwargs.bgColor,
        ANTI_SACCADE_FIXATION_COLOR, defaults.FIXATION_SIZE)
    self.images.preresponseButton = self:drawCircle(
        PRERESPONSE_BUTTON_DIAMETER, PRERESPONSE_BUTTON_COLOR)
  end

  function env:drawGuideTensor(guideHeight, guideWidth, color)
    local guideImage = tensor.ByteTensor(guideHeight,
                                         guideWidth,
                                         3):fill(color)
    local border = {math.floor(guideHeight / 6), math.floor(guideWidth / 6)}
    for row = border[1] + 1, guideHeight - border[1] do
      guideImage(row):narrow(1,
                             border[2] + 1,
                             guideWidth - (2 * border[2])):fill(kwargs.bgColor)
    end
    return guideImage, border
  end

  function env:finishTrial(delay)
    self.currentTrial.blockId = self.blockId
    self.currentTrial.reactionTime =
        game:episodeTimeSeconds() - self._currentTrialStartTime

    self.currentTrial.stepCount = self.pac:elapsedSteps()
    psychlab_helpers.publishTrialData(self.currentTrial, kwargs.schema)
    self.pac:clearTimers()
    psychlab_helpers.finishTrialCommon(self, delay, defaults.FIXATION_SIZE)
  end

  function env:fixationCallback(name, mousePos, hoverTime, userData)
    if hoverTime == defaults.TIME_TO_FIXATE_CROSS then
      self.pac:addReward(defaults.FIXATION_REWARD)
      self.pac:removeWidget('center_of_fixation')
      self.currentTrial.reward = 0
      self.currentTrial.numCorrectSaccades = 0
      self:preStudyPhase()
    end
  end

  function env:finishedSaccadeSequence(name)
    -- Trial is marked correct if the saccade sequence was finished.
    self.currentTrial.response = name
    self.currentTrial.correct = 1
    self.currentTrial.reward = kwargs.correctReward
    self.pac:addReward(self.currentTrial.reward)
    self:finishTrial(defaults.FAST_INTER_TRIAL_INTERVAL)
  end

  function env:trialFailure()
    -- Trial is marked incorrect if it timed out.
    self.currentTrial.correct = 0
    self.currentTrial.reward = kwargs.incorrectReward
    self.pac:addReward(self.currentTrial.reward)
    self:finishTrial(defaults.FAST_INTER_TRIAL_INTERVAL)
  end

  function env:maybeOnTargetCallback(name, mousePos, hoverTime, userData)
    if not userData.fixating then
      userData.fixating = true
      -- Ensure targets are selected in the right order.
      if name == self.remainingTargets[1] then
        self:onTargetCallback(name, mousePos, hoverTime, userData)
        table.remove(self.remainingTargets, 1)
      else
        self:incorrectResponse(name, mousePos, hoverTime, userData)
      end
    end
  end

  function env:incorrectResponse(name, mousePos, hoverTime, userData)
    -- Update the corresponding guide widget to the incorrect signal color.
    local guideWidget = self.pac._widgets[userData.guide]
    local guideImageShape = guideWidget.image:shape()
    local failureImage = tensor.ByteTensor(guideImageShape[1],
                                           guideImageShape[2],
                                           3):fill(kwargs.failureColor)
    -- Trial is finished and marked incorrect when wrong guide is deselected.
    guideWidget.mouseHoverEndCallback = self.trialFailure
    self.pac:updateWidget(userData.guide, failureImage, RESULT_LAYER)
  end

  function env:restoreDeselectedGuide(name, userData)
    local guideImage = self:drawGuideTensor(userData.guideHeight,
                                            userData.guideWidth,
                                            kwargs.guideColor)
    self.pac:updateWidget(name, guideImage, GUIDE_LAYER)
  end

  function env:onTargetCallback(name, mousePos, hoverTime, userData)
    -- Increment the number of correct saccades so far in the current trial.
    self.currentTrial.numCorrectSaccades =
        self.currentTrial.numCorrectSaccades + 1
    -- Update the corresponding guide widget.
    local guideWidget = self.pac._widgets[userData.guide]
    local guideImageShape = guideWidget.image:shape()
    local successImage = tensor.ByteTensor(guideImageShape[1],
                                           guideImageShape[2],
                                           3):fill(kwargs.successColor)
    guideWidget.mouseHoverEndCallback = self.restoreDeselectedGuide
    self.pac:updateWidget(userData.guide, successImage, RESULT_LAYER)
  end

  function env:moveOffTargetCallback(name, userData)
    if userData.fixating then
      -- Remove both the cue and target widgets.
      local nameParts = helpers.split(name, '_')
      self.pac:removeWidget('cue_' .. nameParts[2])
      self.pac:removeWidget('target_' .. nameParts[2])
      if self.currentTrial.numCorrectSaccades ==
          self.currentTrial.saccadeSequenceLength then
        self:finishedSaccadeSequence(name)
      end
    end
  end

  function env:removeTargets(cueHeight, cueWidth)
    for _, name in ipairs(self.remainingTargets) do
      local bgImage = tensor.ByteTensor(cueHeight,
                                        cueWidth,
                                        3):fill(kwargs.bgColor)
      self.pac:updateWidget(name, bgImage, TARGET_LAYER)
    end
  end

  function env:preStudyPhase()
    local trialType = psychlab_helpers.randomFrom(kwargs.trialTypes)
    assert(trialType == 'pro-cue' or trialType == 'anti-cue',
        "Trial type must be 'pro-cue' or 'anti-cue'")
    self.currentTrial.trialType = trialType

    if trialType == 'pro-cue' then
      self.pac:updateWidget('fixation', self.images.proSaccadeFixation)
    elseif trialType == 'anti-cue' then
      self.pac:updateWidget('fixation', self.images.antiSaccadeFixation)
    end

    local conditionType
    if kwargs.memoryGuided then
      conditionType = 'cue_stops'
    else
      conditionType = psychlab_helpers.randomFrom(kwargs.conditionTypes)
    end
    assert(conditionType == 'cue_stops' or conditionType == 'cue_continues',
        "Condition type must be 'cue_stops' or 'cue_continues'")
    self.currentTrial.conditionType = conditionType

    self.pac:addTimer{
        name = 'pre_study_phase',
        timeout = 20,
        callback = function(...)
          self:preresponsePhase()
        end
    }
    -- add the timer to keep track of whether trial timed out or not.
    self.pac:addTimer{
        name = 'trial_timeout',
        timeout = kwargs.trialTimeout,
        callback = function(...) return self:trialFailure() end
    }
    -- Measure reaction time since the trial started.
    self._currentTrialStartTime = game:episodeTimeSeconds()
    self.pac:resetSteps()
  end

  function env:preresponsePhase()
    if self.currentTrial.conditionType == 'cue_stops' then
      -- In future, we might have a gap delay here, but for now this condition
      -- simply means that we remove the fixation before displaying
      -- the stimuli.
      self.pac:removeWidget('fixation')
    end

    if kwargs.memoryGuided then
      -- In memory guided we require agent to fixate until stimuli appear,
      -- to avoid exploit where it exits location guide beforehand.
      self.pac:addWidget{
          name = 'preresponse_button',
          image = self.images.preresponseButton,
          posAbs = psychlab_helpers.getUpperLeftFromCenter(
              {self.screenSize.width / 2, self.screenSize.height / 2},
              PRERESPONSE_BUTTON_DIAMETER
          ),
          sizeAbs = {PRERESPONSE_BUTTON_DIAMETER,
                     PRERESPONSE_BUTTON_DIAMETER},
          mouseHoverCallback = function(...) self:studyPhase() end,
        }
    else
      self:studyPhase()
    end
  end

  function env:studyPhase()
    self.pac:removeWidget('preresponse_button')
    self.currentTrial.saccadeSequenceLength = psychlab_helpers.randomFrom(
        kwargs.saccadeSequenceLengths)

    local guideHeight = math.floor(
        kwargs.screenSize.height * GUIDE_SIZE)
    local guideWidth = math.floor(
        kwargs.screenSize.width * GUIDE_SIZE)
    local cueHeight = math.floor(guideHeight / 1.5)
    local cueWidth = math.floor(guideWidth / 1.5)

    -- Convert sizes back to fractional coordinate system
    local guideSize = {guideWidth / kwargs.screenSize.width,
                       guideHeight / kwargs.screenSize.height}
    local cueSize = {cueWidth / kwargs.screenSize.width,
                     cueHeight / kwargs.screenSize.height}

    -- Draw the guide image tensor and compute border size
    local guideImage, border = self:drawGuideTensor(guideHeight,
                                                    guideWidth,
                                                    kwargs.guideColor)


    local locations = {}
    for i = 1, NUM_LOCATIONS do
      local radians = 2 * math.pi * i / NUM_LOCATIONS
      local pos = {
          0.5 * (1 + kwargs.locationRadius * math.sin(radians)),
          0.5 * (1 + kwargs.locationRadius * math.cos(radians))
      }
      locations[i] = pos
      local guideName = 'guide_' .. tostring(i)
      self.pac:addWidget{
          name = guideName,
          pos = psychlab_helpers.getUpperLeftFromCenter(pos, guideSize),
          size = guideSize,
          image = guideImage,
          imageLayer = GUIDE_LAYER,
          mouseHoverCallback = self.incorrectResponse,
          userData = {guide = guideName,
                      guideHeight = guideHeight,
                      guideWidth = guideWidth}
      }
    end

    -- This was decided during pre study phase
    local trialType = self.currentTrial.trialType

    -- Add saccade cue and target widgets on top of randomly selected guides.
    self.remainingTargets = {}
    local selected = {}
    for saccadeIdx = 1, self.currentTrial.saccadeSequenceLength do
      table.insert(self.remainingTargets, 'target_' .. tostring(saccadeIdx))
      local stimulusIdx, antiIdx, targetIdx
      repeat
        stimulusIdx = random:uniformInt(1, NUM_LOCATIONS)
      until not selected[stimulusIdx]
      antiIdx = (stimulusIdx + NUM_LOCATIONS / 2 - 1) % NUM_LOCATIONS + 1
      selected[stimulusIdx] = true
      selected[antiIdx] = true

      local cueImage = tensor.ByteTensor(cueHeight,
                                         cueWidth,
                                         3):fill(kwargs.colors[saccadeIdx])
      self.pac:addWidget{
          name = 'cue_' .. tostring(saccadeIdx),
          pos = psychlab_helpers.getUpperLeftFromCenter(
              locations[stimulusIdx], cueSize),
          size = cueSize,
          image = cueImage,
          imageLayer = CUE_LAYER
      }
      if trialType == 'pro-cue' then
        targetIdx = stimulusIdx
      elseif trialType == 'anti-cue' then
        targetIdx = antiIdx
      end
      local targetGuideName = 'guide_' .. tostring(targetIdx)

      -- Add the invisible target widget.
      self.pac:addWidget{
          name = 'target_' .. tostring(saccadeIdx),
          pos = psychlab_helpers.getUpperLeftFromCenter(
              locations[targetIdx], cueSize),
          size = cueSize,
          imageLayer = TARGET_LAYER,
          mouseHoverCallback = self.maybeOnTargetCallback,
          mouseHoverEndCallback = self.moveOffTargetCallback,
          userData = {guide = targetGuideName, fixating = false}
      }

      -- Override incorrect mouse hover callback for the corresponding guide.
      local guideWidget = self.pac._widgets[targetGuideName]
      guideWidget.mouseHoverCallback = nil
    end

    -- Add the initial fixation tracker widget for memory guided case.
    if kwargs.memoryGuided then
      local initialLocationImage, _ = self:drawGuideTensor(
          guideHeight,
          guideWidth,
          kwargs.initialLocationGuideColor
      )
      self.pac:addWidget{
            name = 'initial_fixation_tracker',
            pos = psychlab_helpers.getUpperLeftFromCenter(
                {0.5, 0.5}, guideSize),
            size = guideSize,
            image = initialLocationImage,
            imageLayer = GUIDE_LAYER,
            mouseHoverEndCallback = function (...)
              return self:removeTargets(cueHeight, cueWidth)
            end,
        }
    end
  end

  function env:removeArray()
    self.pac:clearWidgets()
  end

  return psychlab_factory.createLevelApi{
      env = point_and_click,
      envOpts = {environment = env, screenSize = kwargs.screenSize,
                 maxStepsOffScreen = kwargs.maxStepsOffScreen},
      episodeLengthSeconds = kwargs.episodeLengthSeconds
  }
end

return factory
