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

local brady_konkle_oliva2008 = require 'datasets.brady_konkle_oliva2008'
local game = require 'dmlab.system.game'
local log = require 'common.log'
local helpers = require 'common.helpers'
local random = require 'common.random'
local psychlab_factory = require 'factories.psychlab.factory'
local psychlab_helpers = require 'factories.psychlab.helpers'
local point_and_click = require 'factories.psychlab.point_and_click'
local random = require 'common.random'
local tensor = require 'dmlab.system.tensor'

--[[ The Harlow task from:

Harlow, H. F. (1949). The formation of learning sets. Psychological review,
56(1), 51.

As implemented in:

Wang, J. X., Kurth-Nelson, Z., Tirumala, D., Soyer, H., Leibo, J. Z., Munos, R.,
... & Botvinick, M. (2016). Learning to reinforcement learn. arXiv preprint
arXiv:1611.05763.

and

Wang, J. X., Kurth-Nelson, Z., Kumaran, D., Tirumala, D., Soyer, H., Leibo,
J. Z., ... & Botvinick, M. (2018). Prefrontal cortex as a meta-reinforcement
learning system. Nature neuroscience, 21(6), 860.

Two random images (selected at the beginning of the episode)
are shown per episode for N (default 6) trials.
One image yields positive rewards if agent saccades to it, other
yields negative rewards. Between trials, agent saccades to a red
fixation cross to start the trial.

Images can be placed on either the left or right side of the
screen, determined randomly at the beginning of the trial.

Any images can be used, but should be placed in /data and numbered
according to the template '%04d.png'. There should be at least
TRAIN_BATCH + TEST_BATCH images. Set IS_TRAIN = false for
evaluation images (held out from training).
]]

local TIME_TO_FIXATE_CROSS = 4 -- in frames
local TIME_TO_FIXATE_TARGET = 4 -- in frames
local FAST_INTER_TRIAL_INTERVAL = 5 -- in frames
local SCREEN_SIZE = {width = 512, height = 512}
local BG_COLOR = {0, 0, 0}
local TRIALS_PER_EPISODE_CAP = 6

local TARGET_SIZE = 0.4

local FIXATION_REWARD = 1
local CORRECT_REWARD = 5
local INCORRECT_REWARD = -5

local FIXATION_SIZE = 0.1
local FIXATION_COLOR = {255, 0, 0} -- Red
local CENTER = {0.5, 0.5}
local LEFT = {.25, .5}
local RIGHT = {.75, .5}

local TRAIN_BATCH = 1000
local TEST_BATCH = 1000
local IS_TRAIN = true

local factory = {}

function factory.createLevelApi(kwargs)
  kwargs.timeToFixateCross = kwargs.timeToFixateCross or TIME_TO_FIXATE_CROSS
  kwargs.timeToFixateTarget = kwargs.timeToFixateTarget or TIME_TO_FIXATE_TARGET
  kwargs.fastInterTrialInterval = kwargs.fastInterTrialInterval or
    FAST_INTER_TRIAL_INTERVAL
  kwargs.screenSize = kwargs.screenSize or SCREEN_SIZE
  kwargs.bgColor = kwargs.bgColor or BG_COLOR
  kwargs.trialsPerEpisodeCap = kwargs.trialsPerEpisodeCap or
    TRIALS_PER_EPISODE_CAP
  kwargs.targetSize = kwargs.targetSize or TARGET_SIZE
  kwargs.fixationReward = kwargs.fixationReward or FIXATION_REWARD
  kwargs.correctReward = kwargs.correctReward or CORRECT_REWARD
  kwargs.incorrectReward = kwargs.incorrectReward or INCORRECT_REWARD
  kwargs.fixationSize = kwargs.fixationSize or FIXATION_SIZE
  kwargs.fixationColor = kwargs.fixationColor or FIXATION_COLOR
  kwargs.center = kwargs.center or CENTER
  kwargs.left = kwargs.left or LEFT
  kwargs.right = kwargs.right or RIGHT
  kwargs.trainBatch = kwargs.trainBatch or TRAIN_BATCH
  kwargs.testBatch = kwargs.testBatch or TEST_BATCH
  kwargs.isTrain = kwargs.isTrain or IS_TRAIN

  local ARG = {
      screenSize = kwargs.screenSize,
  }

  -- Class definition for harlow psychlab environment.
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
    log.info('ARG passed to _init:\n' .. helpers.tostring(ARG))

    if self.dataset == nil then
      self.dataset = brady_konkle_oliva2008(opts)
    end

    self.screenSize = kwargs.screenSize
    self.numTrials = kwargs.trialsPerEpisodeCap

    self.trainBatch = kwargs.trainBatch
    self.testBatch = kwargs.testBatch
    self.isTrain = kwargs.isTrain

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
    psychlab_helpers.addFixation(self, kwargs.fixationSize)

    self.currentTrial = {}
    self._previousTrialId = 0
    self.trialNum = 0

    psychlab_helpers.setTrialsPerEpisodeCap(self, kwargs.trialsPerEpisodeCap)

    local targetPositions = {}
    for i = 1, self.numTrials / 2 do
      table.insert(targetPositions, 1)
    end
    for i = self.numTrials / 2 + 1, self.numTrials do
      table.insert(targetPositions, 2)
    end
    local batch_offset
    if self.isTrain then
      self._numBatch = self.trainBatch
      batch_offset = 0
    else
      self._numBatch = self.testBatch
      batch_offset = self.trainBatch
    end
    self._targetPositions = random:shuffle(targetPositions)
    -- randomly pick a correctID and an incorrectID
    self.correctID = random:uniformInt(1, self._numBatch) + batch_offset
    self.incorrectID = random:uniformInt(1, self._numBatch - 1) + batch_offset
    if self.incorrectID >= self.correctID then
      self.incorrectID = self.incorrectID + 1
    end
    log.info('Episode set up, corrID=' .. self.correctID ..
             ', incorrID=' .. self.incorrectID)
  end

  function env:setupImages()
    self.images = {}

    self.images.fixation = psychlab_helpers.getFixationImage(self.screenSize,
      kwargs.bgColor, kwargs.fixationColor, kwargs.fixationSize)

    self.target = tensor.ByteTensor(256, 256, 3)
  end

  function env:finishTrial(delay)
    self.currentTrial.reactionTime =
      game:episodeTimeSeconds() - self._currentTrialStartTime

    self.currentTrial.stepCount = self.pac:elapsedSteps()
    psychlab_helpers.publishTrialData(self.currentTrial, kwargs.schema)
    psychlab_helpers.finishTrialCommon(self, delay, kwargs.fixationSize)
  end

  function env:fixationCallback(name, mousePos, hoverTime, userData)
    if hoverTime == kwargs.timeToFixateCross then
      self.pac:addReward(kwargs.fixationReward)
      self.pac:removeWidget('fixation')
      self.pac:removeWidget('center_of_fixation')

      self.currentTrial.trialId = self._previousTrialId + 1
      -- Measure reaction time since the trial started.
      self._currentTrialStartTime = game:episodeTimeSeconds()
      self.pac:resetSteps()

      self:startTrial()
    end
  end

  function env:correctResponseCallback(name, mousePos, hoverTime, userData)
    if hoverTime == kwargs.timeToFixateTarget then
      self.currentTrial.response = name
      self.currentTrial.correct = 1
      self.pac:addReward(kwargs.correctReward)
      self:finishTrial(kwargs.fastInterTrialInterval)
    end
  end

  function env:incorrectResponseCallback(name, mousePos, hoverTime, userData)
    if hoverTime == kwargs.timeToFixateTarget then
      self.currentTrial.response = name
      self.currentTrial.correct = 0
      self.pac:addReward(kwargs.incorrectReward)
      self:finishTrial(kwargs.fastInterTrialInterval)
    end
  end

  function env:startTrial()
    self.trialNum = self.trialNum + 1
    -- Fixation initiates the next trial
    self.currentTrial.trial = self.trialNum
    self._targetPos = self._targetPositions[self.trialNum]
    self:addObjects(self.correctID, self.incorrectID, self._targetPos)
  end

  function env:addTargetImage(targetImage, targetSize, targetPos, targetName)
    assert(targetImage, 'targetImage must not be nil')
    assert(targetSize, 'targetSize must not be nil')
    assert(targetSize > 0 and targetSize <= 1, 'targetSize must in (0, 1]')
    self.target:copy(targetImage)
    local sizeInPixels = psychlab_helpers.getSizeInPixels(self.screenSize,
                                                          targetSize)
    local scaledImage = psychlab_helpers.scaleImage(self.target,
                                                    sizeInPixels.width,
                                                    sizeInPixels.height)

    self.pac:addWidget{
        name = targetName,
        image = scaledImage,
        pos = targetPos,
        size = {targetSize, targetSize},
    }
  end

  function env:addObjects(correctID, incorrectID, targetPos)
    local posCorrect, posIncorrect, posCorrectButton, posIncorrectButton
    self.currentTrial.correctResponse = targetPos
    local upperLeft = psychlab_helpers.getUpperLeftFromCenter
    if targetPos == 1 then
      posCorrect = upperLeft(kwargs.left, kwargs.targetSize)
      posIncorrect = upperLeft(kwargs.right, kwargs.targetSize)
      posCorrectButton = upperLeft(kwargs.left, kwargs.targetSize / 2)
      posIncorrectButton = upperLeft(kwargs.right, kwargs.targetSize / 2)
    elseif targetPos == 2 then
      posCorrect = upperLeft(kwargs.right, kwargs.targetSize)
      posIncorrect = upperLeft(kwargs.left, kwargs.targetSize)
      posCorrectButton = upperLeft(kwargs.right, kwargs.targetSize / 2)
      posIncorrectButton = upperLeft(kwargs.left, kwargs.targetSize / 2)
    else
      error('targetPos not valid')
    end
    self:addTargetImage(self.dataset:getImage(correctID),
                       kwargs.targetSize,
                       posCorrect,
                       'correct_image')
    self:addTargetImage(self.dataset:getImage(incorrectID),
                       kwargs.targetSize,
                       posIncorrect,
                       'incorrect_image')

    -- Add buttons, which are invisible and colocated with the images
    self.pac:addWidget{
        name = 'correct_image_button',
        image = nil,
        pos = posCorrectButton,
        size = {kwargs.targetSize / 2, kwargs.targetSize / 2},
        mouseHoverCallback = self.correctResponseCallback,
    }
    self.pac:addWidget{
        name = 'incorrect_image_button',
        image = nil,
        pos = posIncorrectButton,
        size = {kwargs.targetSize / 2, kwargs.targetSize / 2},
        mouseHoverCallback = self.incorrectResponseCallback,
    }
  end

  function env:removeArray()
    self.pac:removeWidget('correct_image')
    self.pac:removeWidget('incorrect_image')
    self.pac:removeWidget('correct_image_button')
    self.pac:removeWidget('incorrect_image_button')
  end

  return psychlab_factory.createLevelApi{
      env = point_and_click,
      envOpts = {environment = env, screenSize = ARG.screenSize}
  }
end

return factory
