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

--[[ This task goes by various names in the literature, they include:

---- conditional visuomotor learning
---- conditional discrimination
---- arbitrary visuomotor mapping

See Wise & Murray. Trends Neurosci. (2000) 23, 271-276.

It can be seen as a special kind of cued recall task where the item to be
be recalled is the response itself.  That is, the associations to be learned in
this task are between images and responses (locations to point at).

This experiment measures how many arbitrary visuomotor maps can be learned and
maintained in memory for the duration of an episode.

The specific image-response pairs to be remembered are generated anew for each
episode.
]]

local TIME_TO_FIXATE_CROSS = 1 -- in frames
local FAST_INTER_TRIAL_INTERVAL = 1 -- in frames
local SCREEN_SIZE = {width = 512, height = 512}
local BG_COLOR = {255, 255, 255}
local TRIALS_PER_EPISODE_CAP = 60

local TARGET_SIZE = 0.75

local FIXATION_REWARD = 0
local CORRECT_REWARD = 1
local INCORRECT_REWARD = 0

local FIXATION_SIZE = 0.1
local FIXATION_COLOR = {255, 0, 0} -- RGB
local CENTER = {0.5, 0.5}
local BUTTON_SIZE = 0.1
local BUTTONS = {'north', 'south', 'east', 'west'}

local BUTTON_POSITIONS = {
    north = psychlab_helpers.getUpperLeftFromCenter(
      {CENTER[1], BUTTON_SIZE / 2}, BUTTON_SIZE),
    south = psychlab_helpers.getUpperLeftFromCenter(
      {CENTER[1], 1 - BUTTON_SIZE / 2}, BUTTON_SIZE),
    east = psychlab_helpers.getUpperLeftFromCenter(
      {BUTTON_SIZE / 2, CENTER[2]}, BUTTON_SIZE),
    west = psychlab_helpers.getUpperLeftFromCenter(
      {1 - BUTTON_SIZE / 2, CENTER[2]}, BUTTON_SIZE)
}

local factory = {}

function factory.createLevelApi(kwargs)
  kwargs.timeToFixateCross = kwargs.timeToFixateCross or TIME_TO_FIXATE_CROSS
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
  kwargs.buttonSize = kwargs.buttonSize or BUTTON_SIZE
  kwargs.buttons = kwargs.buttons or BUTTONS
  kwargs.buttonPositions = kwargs.buttonPositions or BUTTON_POSITIONS

  local ARG = {
      screenSize = kwargs.screenSize,
      jitter = false,
  }

  --[[ 'initAssociationsArray' defines the array of associations and its
  methods.
  (a 'class')
  ]]
  local function initAssociationsArray(dataset)
    local associations = {
        _array = {},
        _order = {},
        _index = 0
    }

    local unusedIds = tensor.Int64Tensor{range = {dataset:getSize()}}:shuffle(
      random:generator())
    local usedIdCount = 0

    local function getNewId()
      if usedIdCount < dataset:getSize() then
        usedIdCount = usedIdCount + 1
        return unusedIds(usedIdCount):val()
      else
        error('Unable to get new image id. Not enough remain.')
      end
    end

    function associations:shuffle()
      local perm = tensor.Int64Tensor{range = {#self._order}}:shuffle(
        random:generator())
      for i = 1, #self._order do
        self._order[i] = perm(i):val()
      end
      self._index = 0
    end

    function associations:add()
      local association = {
          imageId = getNewId(),
          correctResponse = psychlab_helpers.randomFrom(kwargs.buttons),
          timesPreviouslyDisplayed = 0,
          mostRecentTrial = -1
      }
      table.insert(self._array, association)
      table.insert(self._order, #self._order + 1)
    end

    -- 'associations.step' is called during each trial.
    function associations:step(trialId)
      self._index = self._index + 1

      -- Copy the association so as to return it before updating its recency
      -- data.
      self._output = helpers.shallowCopy(self._array[self._order[
        self._index]])

      -- Update the recency data.
      local ref = self._array[self._order[self._index]]
      ref.timesPreviouslyDisplayed = ref.timesPreviouslyDisplayed + 1
      ref.mostRecentTrial = trialId

      -- Reset _index if necessary and return.
      if self._index == #self._array then
        self._index = 0
      end
      return self._output
    end

    return associations
  end

  --[[ Function to define the adaptive staircase procedure (a 'class').
  This procedure promotes from difficulty level K to level K + 1 when K
  consecutive trials are correct.
  ]]
  local function initStaircase(opt)
    local staircase = {
        _difficultyLevel = 1,
        _perfectSoFar = true,
        _index = 0,
        _promoteLevel = opt.promoteFunction,
        _repeatLevel = opt.repeatFunction
    }

    function staircase.promoteLevel(self)
      self._difficultyLevel = self._difficultyLevel + 1
      self._promoteLevel()
    end

    function staircase.repeatLevel(self)
      self._repeatLevel()
    end

    function staircase.endLevel(self)
      if self._perfectSoFar then
        self:promoteLevel()
      else
        self:repeatLevel()
      end
      self._perfectSoFar = true
    end

    -- 'staircase.step' is called at the end of each trial.
    function staircase.step(self, correct)
      self._index = self._index + 1

      -- Track whether all trials are correct.
      if correct ~= 1 then
        self._perfectSoFar = false
      end

      -- Reset _index if necessary and call the endLevel function.
      if self._index == self._difficultyLevel then
        self._index = 0
        self:endLevel()
      end
    end

    return staircase
  end

  -- Class definition for arbitrary visuomotor mapping psychlab environment.
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

    psychlab_helpers.setTrialsPerEpisodeCap(self, kwargs.trialsPerEpisodeCap)

    -- Initialize associations array and adaptive staircase objects.
    self.associations = initAssociationsArray(self.dataset)
    self.staircase = initStaircase{
        repeatFunction = function ()
          self.associations:shuffle()
        end,
        promoteFunction = function ()
          self.associations:add()
          self.associations:shuffle()
        end
    }

    -- Start out with one association in the array. Do this after the seed has
    -- been set.
    self.associations:add()
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

    self.images.dullGreenImage = tensor.ByteTensor(h, w, 3)
    self.images.dullGreenImage:select(3, 1):fill(100)
    self.images.dullGreenImage:select(3, 2):fill(200)
    self.images.dullGreenImage:select(3, 3):fill(100)

    self.images.redImage = tensor.ByteTensor(h, w, 3)
    self.images.redImage:select(3, 1):fill(255)
    self.images.redImage:select(3, 2):fill(100)
    self.images.redImage:select(3, 3):fill(100)

    self.images.dullRedImage = tensor.ByteTensor(h, w, 3)
    self.images.dullRedImage:select(3, 1):fill(200)
    self.images.dullRedImage:select(3, 2):fill(100)
    self.images.dullRedImage:select(3, 3):fill(100)

    self.images.whiteImage = tensor.ByteTensor(h, w, 3):fill(255)
    self.images.blackImage = tensor.ByteTensor(h, w, 3)

    self.target = tensor.ByteTensor(256, 256, 3)
  end

  function env:finishTrial(delay)
    self.currentTrial.memorySetSize = #self.associations._array
    self.currentTrial.reactionTime =
      game:episodeTimeSeconds() - self._currentTrialStartTime
    -- It is necessary to record memorySetSize before stepping the staircase.
    self.staircase:step(self.currentTrial.correct)

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

  function env:revealCorrectResponse()
    for _, button in ipairs(kwargs.buttons) do
      if button == self.currentTrial.correctResponse then
        self.pac:updateWidget(button, self.images.greenImage)
      else
        self.pac:updateWidget(button, self.images.redImage)
      end
    end
  end

  function env:onHoverEnd(name, mousePos, hoverTime, userData)
    self.pac:addReward(self._rewardToDeliver)
    self:finishTrial(kwargs.fastInterTrialInterval)
  end

  function env:correctResponseCallback(name, mousePos, hoverTime, userData)
    self.currentTrial.response = name
    self.currentTrial.correct = 1
    self._rewardToDeliver = kwargs.correctReward

    self:revealCorrectResponse()
  end

  function env:incorrectResponseCallback(name, mousePos, hoverTime, userData)
    self.currentTrial.response = name
    self.currentTrial.correct = 0
    self._rewardToDeliver = kwargs.incorrectReward

    self:revealCorrectResponse()
  end

  function env:addResponseButtons()
    for _, button in ipairs(kwargs.buttons) do
      local callback
      if button == self.currentTrial.correctResponse then
        callback = self.correctResponseCallback
      else
        callback = self.incorrectResponseCallback
      end

      -- When an association is shown for the first time, show the correct
      -- answer.
      local buttonImage
      if self.currentTrial.timesPreviouslyDisplayed > 0 then
        buttonImage = self.images.blackImage
      else
        if button == self.currentTrial.correctResponse then
          buttonImage = self.images.dullGreenImage
        else
          buttonImage = self.images.dullRedImage
        end
      end

      self.pac:addWidget{
          name = button,
          image = buttonImage,
          pos = kwargs.buttonPositions[button],
          size = {kwargs.buttonSize, kwargs.buttonSize},
          mouseHoverCallback = callback,
          mouseHoverEndCallback = self.onHoverEnd,
      }
    end
  end

  function env:getNextTrial(trialId)
    local pair = self.associations:step(trialId)
    self.currentTrial.imageIndex = pair.imageId
    self.currentTrial.correctResponse = pair.correctResponse

    self.currentTrial.timesPreviouslyDisplayed = pair.timesPreviouslyDisplayed

    if pair.mostRecentTrial > 0 then
      self.currentTrial.recency = trialId - pair.mostRecentTrial
    else
      -- 'recency' is -1 the first time a new association is introduced.
      self.currentTrial.recency = -1
    end
  end

  function env:addArray(trialId)
    self:getNextTrial(trialId)
    psychlab_helpers.addTargetImage(self,
                                    self.dataset:getImage(
                                      self.currentTrial.imageIndex),
                                    kwargs.targetSize)
    self:addResponseButtons()
  end

  function env:removeArray()
    self.pac:removeWidget('target')
    for _, button in ipairs(kwargs.buttons) do
      self.pac:removeWidget(button)
    end
  end

  return psychlab_factory.createLevelApi{
      env = point_and_click,
      envOpts = {environment = env, screenSize = ARG.screenSize}
  }
end

return factory
