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

local brady_konkle_oliva2008 = require 'datasets.brady_konkle_oliva2008'
local game = require 'dmlab.system.game'
local log = require 'common.log'
local helpers = require 'common.helpers'
local psychlab_factory = require 'factories.psychlab.factory'
local psychlab_helpers = require 'factories.psychlab.helpers'
local point_and_click = require 'factories.psychlab.point_and_click'
local random = require 'common.random'
local tensor = require 'dmlab.system.tensor'

--[[ In each trial, the agent must indicate whether an image is old or new.
50% of the time the answer is new. The set of old images keeps growing.

Unlike most probe recognition paradigms, in this continuous recognition paradigm
there are no separate study and test phases.

It is a 'varied mapping' paradigm since the same images can appear in different
orders in different episodes. This terminology is standard. It comes from
Schneider & Shifrin (1977).
]]

local TIME_TO_FIXATE_CROSS = 1 -- in frames
local FAST_INTER_TRIAL_INTERVAL = 1 -- in frames
local BG_COLOR = {255, 255, 255}
local TRIALS_PER_EPISODE_CAP = 60

local TARGET_SIZE = 0.75

local FIXATION_REWARD = 0
local CORRECT_REWARD = 1
local INCORRECT_REWARD = 0
local FIXATION_SIZE = 0.1
local FIXATION_COLOR = {255, 0, 0} -- RGB
local BUTTON_WIDTH = 0.1
local BUTTON_HEIGHT = 0.1

local factory = {}

function factory.createLevelApi(kwargs)
  kwargs.timeToFixateCross = kwargs.timeToFixateCross or TIME_TO_FIXATE_CROSS
  kwargs.fastInterTrialInterval = kwargs.fastInterTrialInterval or
    FAST_INTER_TRIAL_INTERVAL
  kwargs.bgColor = kwargs.bgColor or BG_COLOR
  kwargs.trialsPerEpisodeCap = kwargs.trialsPerEpisodeCap or
    TRIALS_PER_EPISODE_CAP
  kwargs.targetSize = kwargs.targetSize or TARGET_SIZE
  kwargs.fixationReward = kwargs.fixationReward or FIXATION_REWARD
  kwargs.correctReward = kwargs.correctReward or CORRECT_REWARD
  kwargs.incorrectReward = kwargs.incorrectReward or INCORRECT_REWARD
  kwargs.fixationSize = kwargs.fixationSize or FIXATION_SIZE
  kwargs.fixationColor = kwargs.fixationColor or FIXATION_COLOR
  kwargs.buttonWidth = kwargs.buttonWidth or BUTTON_WIDTH
  kwargs.buttonHeight = kwargs.buttonHeight or BUTTON_HEIGHT

  local ARG = {
      screenSize = {width = 512, height = 512},
      jitter = false,
  }

  -- Class definition for continuous recognition psychlab environment.
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

    self.screenSize = opts.screenSize

    -- If requested, randomly perturb the target location for each trial.
    self.jitter = ARG.jitter
    if self.jitter then
      log.info('Jitter target location')
      self.jitteredCenter = {}
    end

    self:setupImages()

    -- Store a copy of the 'point_and_click' api.
    self.pac = pac
  end

  -- 'env:setupOrder' determines the order in which images are shown.
  function env:setupOrder()
    local firstTrial = true
    local newIDs = tensor.Int64Tensor{range = {self.dataset:getSize()}}:shuffle(
      random:generator())
    self._oldIDs = {}
    self._trialLastDisplayed = {}
    self._timesPreviouslyDisplayed = {}

    local function newTrial(trialId)
      assert(#self._oldIDs < self.dataset:getSize(),
             'Unable to get new trial id.')
      local stimID = newIDs(#self._oldIDs + 1):val()
      table.insert(self._oldIDs, stimID)
      table.insert(self._trialLastDisplayed, trialId)
      table.insert(self._timesPreviouslyDisplayed, 0)

      self.currentTrial.imageIndex = stimID
      self.currentTrial.isNew = true
      -- 'recency' is -1 the first time a new association is introduced.
      self.currentTrial.recency = -1
      self.currentTrial._timesPreviouslyDisplayed = 0

      return self.currentTrial.imageIndex, self.currentTrial.isNew
    end

    local function oldTrial(trialId)
      local stimID, stimIndex = psychlab_helpers.randomFrom(self._oldIDs)

      self.currentTrial.imageIndex = stimID
      self.currentTrial.isNew = false
      -- 'recency' denotes the number of trials since item was last shown.
      self.currentTrial.recency = trialId - self._trialLastDisplayed[stimIndex]
      self.currentTrial._timesPreviouslyDisplayed =
        self._timesPreviouslyDisplayed[stimIndex]

      -- Keep track of the last trial id when each oldID was last shown.
      self._trialLastDisplayed[stimIndex] = trialId
      self._timesPreviouslyDisplayed[stimIndex] =
        self._timesPreviouslyDisplayed[stimIndex] + 1
      return self.currentTrial.imageIndex, self.currentTrial.isNew
    end

    self.getNextTrial = function (trialId)
      if firstTrial then
        firstTrial = false
        return newTrial(trialId)
      else
        if #self._oldIDs < self.dataset:getSize() and
           random:uniformReal(0, 1) > .5 then
          return newTrial(trialId)
        else
          return oldTrial(trialId)
        end
      end
    end
  end

  --[[ Reset is called after init. It is called only once per episode.
  Note: the episodeId passed to this function may not be correct if the job
  has resumed from a checkpoint after preemption.
  ]]
  function env:reset(episodeId, seed, ...)
    random:seed(seed)

    self.pac:setBackgroundColor(kwargs.bgColor)
    self.pac:clearWidgets()
    psychlab_helpers.addFixation(self, kwargs.fixationSize)

    self.currentTrial = {}
    self._previousTrialId = 0

    psychlab_helpers.setTrialsPerEpisodeCap(self, kwargs.trialsPerEpisodeCap)

    self:setupOrder()
  end

  function env:setupImages()
    self.images = {}

    self.images.fixation = psychlab_helpers.getFixationImage(self.screenSize,
                                                    kwargs.bgColor,
                                                    kwargs.fixationColor,
                                                    kwargs.fixationSize)

    local h = kwargs.buttonHeight * self.screenSize.height
    local w = kwargs.buttonWidth * self.screenSize.width

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

    self.target = tensor.ByteTensor(256, 256, 3)
  end

  function env:finishTrial(delay)
    self.currentTrial.memorySetSize = #self._oldIDs
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

      -- 'trialId' must be set before adding array to compute recency.
      self:addArray(self.currentTrial.trialId)
    end
  end

  function env:onHoverEnd(name, mousePos, hoverTime, userData)
    self.pac:addReward(self._rewardToDeliver)
    self:finishTrial(kwargs.fastInterTrialInterval)
  end

  function env:correctResponseCallback(name, mousePos, hoverTime, userData)
    self.currentTrial.response = name
    self.currentTrial.correct = 1

    self.pac:updateWidget(name, self.images.greenImage)
    self._rewardToDeliver = kwargs.correctReward
  end

  function env:incorrectResponseCallback(name, mousePos, hoverTime, userData)
    self.currentTrial.response = name
    self.currentTrial.correct = 0

    self.pac:updateWidget(name, self.images.redImage)
    self._rewardToDeliver = kwargs.incorrectReward
  end

  function env:addResponseButtons(isNew)
    local buttonPosY = 0.5 - kwargs.buttonHeight / 2
    local buttonSize = {kwargs.buttonWidth, kwargs.buttonHeight}

    local newCallback, oldCallback
    if isNew then
      newCallback = self.correctResponseCallback
      oldCallback = self.incorrectResponseCallback
    else
      newCallback = self.incorrectResponseCallback
      oldCallback = self.correctResponseCallback
    end

    self.pac:addWidget{
        name = 'newButton',
        image = self.images.blackImage,
        pos = {0, buttonPosY},
        size = buttonSize,
        mouseHoverCallback = newCallback,
        mouseHoverEndCallback = self.onHoverEnd,
    }
    self.pac:addWidget{
        name = 'oldButton',
        image = self.images.blackImage,
        pos = {1 - kwargs.buttonWidth, buttonPosY},
        size = buttonSize,
        mouseHoverCallback = oldCallback,
        mouseHoverEndCallback = self.onHoverEnd,
    }
  end

  function env:addArray(trialId)
    self.currentTrial.imageIndex, self.currentTrial.isNew =
      self.getNextTrial(trialId)
    local img = self.dataset:getImage(self.currentTrial.imageIndex)
    psychlab_helpers.addTargetImage(self, img, kwargs.targetSize)
    self:addResponseButtons(self.currentTrial.isNew)
  end

  function env:removeArray()
    self.pac:removeWidget('target')
    self.pac:removeWidget('newButton')
    self.pac:removeWidget('oldButton')
  end

  return psychlab_factory.createLevelApi{
      env = point_and_click,
      envOpts = {environment = env, screenSize = ARG.screenSize}
  }
end

return factory
