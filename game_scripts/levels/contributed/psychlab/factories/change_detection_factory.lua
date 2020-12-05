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

local game = require 'dmlab.system.game'
local helpers = require 'common.helpers'
local psychlab_factory = require 'factories.psychlab.factory'
local psychlab_helpers = require 'factories.psychlab.helpers'
local image = require 'dmlab.system.image'
local point_and_click = require 'factories.psychlab.point_and_click'
local random = require 'common.random'
local tensor = require 'dmlab.system.tensor'
local psychlab_staircase = require 'levels.contributed.psychlab.factories.staircase'
local log = require 'common.log'

--[[ Each trial consists of a study and test phase separated by a brief delay
interval. The agent must indicate whether or not the test array is the same
as the study array.

This protocol has been used extensively in the literature concerned with visual
working memory. It is usually called 'change detection' in the literature. It
could also be called 'sequential comparison'.

See Luck and Vogel (1997) for an influential example of this kind of experiment.
]]

-- setup default parameters of the task
local TIME_TO_FIXATE_CROSS = 1 -- in frames
local FAST_INTER_TRIAL_INTERVAL = 1 -- in frames
local SCREEN_SIZE = {width = 256, height = 256}
local BG_COLOR = {255, 255, 255}
local EPISODE_LENGTH_SECONDS = 180
local TRIALS_PER_EPISODE_CAP = 60

local TARGET_SIZE = .75 -- fraction of screen to fill

local GRID = {size = 64, step = 8}

local STUDY_TIME = 90 -- 120 -- in frames  (ignored in self paced mode)
local DELAY_TIMES = {8, 16, 32, 64, 128, 256} -- in frames

local SET_SIZES = {1, 2, 3, 4, 5, 6, 7, 8, 9}

local CORRECT_REWARD_SEQUENCE = 1
local ADVANCE_TRIAL_REWARD = 0
local INCORRECT_REWARD = 0
local FIXATION_SIZE = .1
local FIXATION_COLOR = {255, 0, 0} -- RGB
local CENTER = {.5, .5}
local BUTTON_SIZE = 0.1
local END_STUDY_BUTTON_SIZE = .09375
local SELF_PACED = '1'
local ALLOW_TRANSLATION = false

-- These are RGB values, they have a fixed luminance in HSL space.
local COLORS = {
    {255, 0, 191}, -- magenta
    {255, 0, 0}, -- red
    {255, 191, 0}, -- sunflower yellow
    {127, 255, 0}, -- lime green
    {0, 255, 255}, -- light blue
    {0, 63, 255}, -- dark blue
    {127, 0, 255}, -- purple
}

-- Staircase parameters
local FRACTION_TO_PROMOTE = 1.0
local FRACTION_TO_DEMOTE = 0.75
local PROBE_PROBABILITY = 0.1

-- Frames before response buttons appear in test phase.
local PRERESPONSE_TIME = 1

-- If REQUIRE_AT_CENTER_PRERESPONSE set to true, then require agent to look to
-- a circle in the center of the screen just before the target. This prevents
-- immediately looking near the buttons at the start of the study phase.
local REQUIRE_AT_CENTER_PRERESPONSE = false
local PRERESPONSE_BUTTON_DIAMETER = 21
local PRERESPONSE_BUTTON_COLOR = {0, 0, 0}

local MAX_STEPS_OFF_SCREEN = 300  -- 5 seconds

local factory = {}

function factory.createLevelApi(kwargs)
  kwargs.episodeLengthSeconds = kwargs.episodeLengthSeconds or
      EPISODE_LENGTH_SECONDS
  kwargs.trialsPerEpisodeCap = kwargs.trialsPerEpisodeCap or
      TRIALS_PER_EPISODE_CAP
  kwargs.timeToFixateCross = kwargs.timeToFixateCross or TIME_TO_FIXATE_CROSS
  kwargs.fastInterTrialInterval = kwargs.fastInterTrialInterval or
      FAST_INTER_TRIAL_INTERVAL
  kwargs.screenSize = kwargs.screenSize or SCREEN_SIZE
  kwargs.bgColor = kwargs.bgColor or BG_COLOR
  kwargs.targetSize = kwargs.targetSize or TARGET_SIZE
  kwargs.grid = kwargs.grid or GRID
  kwargs.studyTime = kwargs.studyTime or STUDY_TIME
  kwargs.delayTimes = kwargs.delayTimes or DELAY_TIMES
  kwargs.correctRewardSequence = kwargs.correctRewardSequence or
      CORRECT_REWARD_SEQUENCE
  kwargs.advanceTrialReward = kwargs.advanceTrialReward or ADVANCE_TRIAL_REWARD
  kwargs.incorrectReward = kwargs.incorrectReward or INCORRECT_REWARD
  kwargs.fixationSize = kwargs.fixationSize or FIXATION_SIZE
  kwargs.fixationColor = kwargs.fixationColor or FIXATION_COLOR
  kwargs.center = kwargs.center or CENTER
  kwargs.buttonSize = kwargs.buttonSize or BUTTON_SIZE
  kwargs.endStudyButtonSize = kwargs.endStudyButtonSize or END_STUDY_BUTTON_SIZE
  kwargs.colors = kwargs.colors or COLORS
  kwargs.setSizes = kwargs.setSizes or SET_SIZES
  kwargs.selfPaced = kwargs.selfPaced or SELF_PACED
  kwargs.allowTranslation = kwargs.allowTranslation or ALLOW_TRANSLATION
  kwargs.fractionToPromote = kwargs.fractionToPromote or FRACTION_TO_PROMOTE
  kwargs.fractionToDemote = kwargs.fractionToDemote or FRACTION_TO_DEMOTE
  kwargs.probeProbability = kwargs.probeProbability or PROBE_PROBABILITY
  kwargs.fixedTestLength = kwargs.fixedTestLength or false
  kwargs.initialDifficultyLevel = kwargs.initialDifficultyLevel or 1
  kwargs.preresponseTime = kwargs.preresponseTime or PRERESPONSE_TIME
  if kwargs.requireAtCenterPreresponse == nil then
    kwargs.requireAtCenterPreresponse = REQUIRE_AT_CENTER_PRERESPONSE
  end
  kwargs.preresponseButtonDiameter = kwargs.preresponseButtonDiameter or
      PRERESPONSE_BUTTON_DIAMETER
  kwargs.preresponseButtonColor = kwargs.preresponseButtonColor or
      PRERESPONSE_BUTTON_COLOR
  kwargs.maxStepsOffScreen = kwargs.maxStepsOffScreen or MAX_STEPS_OFF_SCREEN

  -- Types of objects to display
  local ALL_OPTO_TYPES = {'E', 'Square'}

  -- Orientations at which to display the 'E' optotype
  local ORIENTATIONS = {'left', 'right', 'up', 'down'}

  --[[ Domains from which to sample objects for study phase. Within a trial, all
  objects are sampled from the same domain. Domains may be interleaved between
  trials. ]]
  local DOMAINS = {
      'E_ALL',
      'E_COLOR',
      'E_ORIENTATION',
      'SQUARE_COLOR',
      'ALL',
  }

  local function getArrayMedian(array)
    if #array % 2 == 1 then
      return array[math.ceil(#array / 2)]
    else
      return (array[math.floor(#array / 2)] + array[math.ceil(#array / 2)]) / 2
    end
  end

  --[[ Returns a random point with integer-valued x, y coordinates.
  'limit' (integer) Largest possible x value
  'step' (integer) Defines the size of a grid square, should evenly divide limit
  Returns: (table {x, y}) where x and y are sampled from the grid.
  ]]
  local function getRandomCoordinates(limit, step)
    -- prepare domain
    local fullDomain = psychlab_helpers.range(math.floor(step / 2), limit, step)
    local gridCenter = getArrayMedian(fullDomain)
    local invalidDomain = {}
    -- Exclude the 4 cells where the end-study button appears in self-paced
    -- mode.
    for _, i in pairs{-2, -1, 1, 2} do
      invalidDomain[gridCenter + i * step] = true
    end

    -- sample coordinates
    local x, _ = psychlab_helpers.randomFrom(fullDomain)
    local y, _ = psychlab_helpers.randomFrom(fullDomain)
    while invalidDomain[x] and invalidDomain[y] do
      x, _ = psychlab_helpers.randomFrom(fullDomain)
      y, _ = psychlab_helpers.randomFrom(fullDomain)
    end
    return {x, y}
  end

  -- Converts a nil or string flag to a boolean, if nil or '0', then returns
  -- false.
  local function stringOrNilToBool(flag)
    return tonumber(flag or '0') > 0
  end

  --[[ Sequential comparison psychlab environment class
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

    -- apply command line arguments
    self.jitter = stringOrNilToBool(opts.jitter)
    if self.jitter then
      -- if ARG.jitter then randomly perturb the target location on each trial
      log.info('Jitter target location')
      self.jitteredCenter = {}
    end
    self.domains = DOMAINS
    kwargs.selfPaced = stringOrNilToBool(kwargs.selfPaced)

    -- setup images and grid parameters
    self:setupImages()
    self:setupGrid()

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

    -- setup the adaptive staircase procedure
    self.staircase = psychlab_staircase.createStaircase1D{
        sequence = kwargs.setSizes,
        correctRewardSequence = kwargs.correctRewardSequence,
        fractionToPromote = kwargs.fractionToPromote,
        fractionToDemote = kwargs.fractionToDemote,
        probeProbability = kwargs.probeProbability,
        fixedTestLength = kwargs.fixedTestLength,
        initialDifficultyLevel = kwargs.initialDifficultyLevel,
    }

    -- blockId groups together all rows written during the same episode
    self.blockId = random:uniformInt(1, 2 ^ 32)
    self.trialId = 1
  end

  function env:setupGrid()
    self._gridLimit = kwargs.grid.size - kwargs.grid.step
    self._gridStep = kwargs.grid.step

    self.targetPixels = {
        width = kwargs.targetSize * self.screenSize.width,
        height = kwargs.targetSize * self.screenSize.height
    }
    self._hFactor = self.targetPixels.width / kwargs.grid.size
    self._vFactor = self.targetPixels.height / kwargs.grid.size

    if kwargs.selfPaced then
      local endStudyGridLocs = {
          {
              self._gridLimit / 2 - 2 * self._gridStep - self._gridStep,
              self._gridLimit / 2 - 2 * self._gridStep - self._gridStep
          },
          {
              self._gridLimit / 2 - 2 * self._gridStep - self._gridStep,
              self._gridLimit / 2 - 2 * self._gridStep + self._gridStep
          },
          {
              self._gridLimit / 2 - 2 * self._gridStep + self._gridStep,
              self._gridLimit / 2 - 2 * self._gridStep - self._gridStep
          },
          {
              self._gridLimit / 2 - 2 * self._gridStep + self._gridStep,
              self._gridLimit / 2 - 2 * self._gridStep + self._gridStep
          },
      }
      local offset = 1 - kwargs.targetSize
      self._endStudyButtonLocs = {}
      for i = 1, #endStudyGridLocs do
        self._endStudyButtonLocs[i] = {
            offset + endStudyGridLocs[i][1] / kwargs.grid.size,
            offset + endStudyGridLocs[i][2] / kwargs.grid.size
        }
      end
    end
  end

  local function borderFill(sourceImage, height, width, borderSize, color)
    for i = 1, 3 do
      sourceImage:narrow(1, 1, borderSize):narrow(2, 1, width):select(3, i
        ):fill(color[i])
      local h0 = math.floor(height - borderSize + 1.5)
      local h1 = math.floor(height + .5)
      sourceImage:narrow(1, h0, h1 - h0):narrow(2, 1, width):select(3, i):fill(
        color[i])
      sourceImage:narrow(1, 1, height):narrow(2, 1, borderSize):select(3, i
        ):fill(color[i])
      local w0 = math.floor(width - borderSize + 1.5)
      local w1 = math.floor(width + .5)
      sourceImage:narrow(1, 1, height):narrow(2, w0, w1 - w0):select(3, i):fill(
        color[i])
    end
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

    local targetPixels = psychlab_helpers.getSizeInPixels(self.screenSize,
                                                          kwargs.targetSize)
    self.target = tensor.ByteTensor(targetPixels.height, targetPixels.width, 3)

    -- create self-paced endStudyPhase button image
    local borderSize = (h + w) / 8
    local TURQUOISE = {59, 165, 170}
    self.images.turquoiseBox = tensor.ByteTensor(h, w, 3)
    borderFill(self.images.turquoiseBox, h, w, borderSize, TURQUOISE)

    self.images.turquoiseImage = tensor.ByteTensor(h, w, 3)
    for i = 1, 3 do
      self.images.turquoiseImage:select(3, i):fill(TURQUOISE[i])
    end

    self.images.preresponseButton = psychlab_helpers.makeFilledCircle(
        kwargs.preresponseButtonDiameter,
        kwargs.preresponseButtonColor,
        kwargs.bgColor
    )
  end

  function env:finishTrial(delay)
    self.currentTrial.blockId = self.blockId
    -- Times are recorded in microseconds.
    self.currentTrial.reactionTime =
      game:episodeTimeSeconds() - self._currentTrialStartTime
    self.currentTrial.responseTime =
      game:episodeTimeSeconds() - self._responseStartTime
    self.staircase:step(self.currentTrial.correct == 1)

    self.currentTrial.stepCount = self.pac:elapsedSteps()
    psychlab_helpers.publishTrialData(self.currentTrial, kwargs.schema)
    psychlab_helpers.finishTrialCommon(self, delay, kwargs.fixationSize)
  end

  function env:fixationCallback(name, mousePos, hoverTime, userData)
    if hoverTime == kwargs.timeToFixateCross then
      self.pac:addReward(kwargs.advanceTrialReward)
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

      -- go to the study phase
      self:studyPhase()
    end
  end

  function env:onHoverEnd(name, mousePos, hoverTime, userData)
    self.currentTrial.reward = self._rewardToDeliver
    self.pac:addReward(self.currentTrial.reward)
    self:finishTrial(kwargs.fastInterTrialInterval)
  end

  function env:correctResponseCallback(name, mousePos, hoverTime, userData)
    self.currentTrial.response = name
    self.currentTrial.correct = 1

    self.pac:updateWidget(name, self.images.greenImage)
    self._rewardToDeliver = self.staircase:correctReward()
  end

  function env:incorrectResponseCallback(name, mousePos, hoverTime, userData)
    self.currentTrial.response = name
    self.currentTrial.correct = 0

    self.pac:updateWidget(name, self.images.redImage)
    self._rewardToDeliver = kwargs.incorrectReward
  end

  function env:endStudyPhaseCallback(name, mousePos, hoverTime, userData)
    local buttonImage = psychlab_helpers.scaleImageToScreenFraction(
      self.images.turquoiseImage,
      {width = kwargs.endStudyButtonSize, height = kwargs.endStudyButtonSize},
      self.screenSize
    )
    self.pac:updateWidget(name, buttonImage)
    self._rewardToDeliver = kwargs.advanceTrialReward
  end

  -- Place the response buttons on the screen
  function env:addResponseButtons(isNew)
    local buttonPosX = 0.5 - kwargs.buttonSize * 1.5
    local buttonSize = {kwargs.buttonSize, kwargs.buttonSize}

    local newCallback, oldCallback
    if isNew then
      newCallback = self.correctResponseCallback
      oldCallback = self.incorrectResponseCallback
    else
      newCallback = self.incorrectResponseCallback
      oldCallback = self.correctResponseCallback
    end

    -- "different" on left, "same" on right
    self.pac:addWidget{
        name = 'newButton',
        image = self.images.blackImage,
        pos = {buttonPosX, 1 - kwargs.buttonSize},
        size = buttonSize,
        mouseHoverCallback = newCallback,
        mouseHoverEndCallback = self.onHoverEnd,
    }
    self.pac:addWidget{
        name = 'oldButton',
        image = self.images.blackImage,
        pos = {1 - buttonPosX - kwargs.buttonSize, 1 - kwargs.buttonSize},
        size = buttonSize,
        mouseHoverCallback = oldCallback,
        mouseHoverEndCallback = self.onHoverEnd,
    }
  end

  -- Place the button to end the study phase and advance the trial on the screen
  function env:addEndStudyPhaseButton(buttonLoc)
    local buttonImage = psychlab_helpers.scaleImageToScreenFraction(
      self.images.turquoiseBox,
      {width = kwargs.endStudyButtonSize, height = kwargs.endStudyButtonSize},
      self.screenSize
    )
    self.pac:addWidget{
        name = 'endStudyPhaseButton',
        image = buttonImage,
        pos = {buttonLoc[1], buttonLoc[2]},
        size = {kwargs.endStudyButtonSize, kwargs.endStudyButtonSize},
        imageLayer = 2,
        mouseHoverCallback = self.endStudyPhaseCallback,
        mouseHoverEndCallback = function(...) return self.delayPhase(self) end,
    }
  end

  function env:getDomain(domainType)
    if domainType == 'E_ALL' then
      return {
          optotypes = {'E'},
          colors = kwargs.colors,
          colorIds = psychlab_helpers.range(1, #kwargs.colors),
          orientations = ORIENTATIONS
      }
    elseif domainType == 'E_COLOR' then
      local fixedOrientation, _ = psychlab_helpers.randomFrom(ORIENTATIONS)
      return {
          optotypes = {'E'},
          colors = kwargs.colors,
          colorIds = psychlab_helpers.range(1, #kwargs.colors),
          orientations = {fixedOrientation}
      }
    elseif domainType == 'E_ORIENTATION' then
      local fixedColor, fixedColorId = psychlab_helpers.randomFrom(
        kwargs.colors)
      return {
          optotypes = {'E'},
          colors = {fixedColor},
          colorIds = {fixedColorId},
          orientations = ORIENTATIONS
      }
    elseif domainType == 'SQUARE_COLOR' then
      return {
          optotypes = {'Square'},
          colors = kwargs.colors,
          colorIds = psychlab_helpers.range(1, #kwargs.colors),
          orientations = ORIENTATIONS, -- orientation does nothing for a square
      }
    elseif domainType == 'ALL' then
      return {
          optotypes = ALL_OPTO_TYPES,
          colors = kwargs.colors,
          colorIds = psychlab_helpers.range(1, #kwargs.colors),
          orientations = ORIENTATIONS
      }
    end
  end

  -- Return a table with the properties of each study object to draw.
  function env:getStudyArrayData()
    self.currentTrial.difficultyLevel = self.staircase:getDifficultyLevel()
    self.currentTrial.setSize = self.staircase:parameter()
    self.currentTrial.domainType = psychlab_helpers.randomFrom(self.domains)
    local domain = self:getDomain(self.currentTrial.domainType)

    local studyArrayData = {
        location = {},
        color = {},
        colorId = {},
        optotype = {},
        orientation = {}
    }

    -- iterate over objects in the study array
    self._currentStudyLocationsSet = {}
    for i = 1, self.currentTrial.setSize do
      -- generate random location, color, optotype, and orientation
      local location = getRandomCoordinates(self._gridLimit, self._gridStep)
      local color, index = psychlab_helpers.randomFrom(domain.colors)
      local colorId = domain.colorIds[index]
      local optotype = psychlab_helpers.randomFrom(domain.optotypes)
      local orientation = psychlab_helpers.randomFrom(domain.orientations)

      -- make sure the random location was not already used
      while self._currentStudyLocationsSet[helpers.tostring(location)] do
        location = getRandomCoordinates(self._gridLimit, self._gridStep)
      end
      self._currentStudyLocationsSet[helpers.tostring(location)] = true

      table.insert(studyArrayData.location, location)
      table.insert(studyArrayData.color, color)
      table.insert(studyArrayData.colorId, colorId)
      table.insert(studyArrayData.optotype, optotype)
      table.insert(studyArrayData.orientation, orientation)
    end
    return studyArrayData
  end

  -- Get set of legal transformations of a given  domain type and optotype.
  function env:getLegalTransforms(domainType, optotype)
    local legalTransforms
    if domainType == 'E_ALL' then
      legalTransforms = {'COLOR', 'ORIENTATION'}
    elseif domainType == 'E_COLOR' then
      legalTransforms = {'COLOR'}
    elseif domainType == 'E_ORIENTATION' then
      legalTransforms = {'ORIENTATION'}
    elseif domainType == 'SQUARE_COLOR' then
      legalTransforms = {'COLOR'}
    elseif domainType == 'ALL' then
      if optotype == 'E' then
        legalTransforms = {'OPTOTYPE', 'COLOR', 'ORIENTATION'}
      elseif optotype == 'Square' then
        legalTransforms = {'OPTOTYPE', 'COLOR'}
      end
    end
    if kwargs.allowTranslation then
      table.insert(legalTransforms, 'TRANSLATION')
    end
    return legalTransforms
  end

  -- Return all data needed to draw the test array and response buttons.
  function env:getTestArrayData(studyArrayData)
    local testArrayData = helpers.deepCopy(studyArrayData)
    local isNew = random:uniformReal(0, 1) > .5
    if not isNew then
      self.currentTrial.transform = 'NONE'
    else
      -- select an object to change
      local changedObjectIndex = random:uniformInt(1, #testArrayData.location)

      -- select a transformation to apply
      local legalTransforms = self:getLegalTransforms(
          self.currentTrial.domainType,
          testArrayData.optotype[changedObjectIndex]
      )
      self.currentTrial.transform, _ = psychlab_helpers.randomFrom(
        legalTransforms)

      -- apply the transformation
      if self.currentTrial.transform == 'COLOR' then
        -- make sure the random color is not the one that was already used
        local newColor, newColorId = psychlab_helpers.randomFrom(kwargs.colors)
        while newColor == studyArrayData.color[changedObjectIndex] do
          newColor, newColorId = psychlab_helpers.randomFrom(kwargs.colors)
        end
        testArrayData.color[changedObjectIndex] = newColor
        testArrayData.colorId[changedObjectIndex] = newColorId
      elseif self.currentTrial.transform == 'ORIENTATION' then
        -- make sure the random orientation is not the one that was already used
        local newOrientation, _ = psychlab_helpers.randomFrom(ORIENTATIONS)
        while newOrientation ==
            studyArrayData.orientation[changedObjectIndex] do
          newOrientation, _ = psychlab_helpers.randomFrom(ORIENTATIONS)
        end
        testArrayData.orientation[changedObjectIndex] = newOrientation
      elseif self.currentTrial.transform == 'OPTOTYPE' then
        if testArrayData.optotype[changedObjectIndex] == 'E' then
          testArrayData.optotype[changedObjectIndex] = 'Square'
        elseif testArrayData.optotype[changedObjectIndex] == 'Square' then
          testArrayData.optotype[changedObjectIndex] = 'E'
        else
          error('Unrecognized optotype: ' ..
              testArrayData.optotype[changedObjectIndex])
        end
      elseif self.currentTrial.transform == 'TRANSLATION' then
        -- make sure the random location is not on top of another object
        local newLocation = getRandomCoordinates(self._gridLimit,
          self._gridStep)
        while self._currentStudyLocationsSet[helpers.tostring(newLocation)] do
          newLocation = getRandomCoordinates(self._gridLimit, self._gridStep)
        end
        testArrayData.location[changedObjectIndex] = newLocation
      end
    end
    return testArrayData, isNew
  end

  function env:_drawSquare(location, color)
    for i = 1, #color do
      self._array:narrow(1, location.top, location.bottom - location.top):
                  narrow(2, location.left, location.right - location.left):
                  select(3, i):fill(color[i])
    end
  end

  function env:_drawE(location, color, orientation)
    local height = location.bottom - location.top
    local width = location.right - location.left

    local twentyPercentY = math.floor(.5 + 0.2 * height) + location.top
    local fortyPercentY = math.floor(.5 + 0.4 * height) + location.top
    local sixtyPercentY = math.floor(.5 + 0.6 * height) + location.top
    local eightyPercentY = math.floor(.5 + 0.8 * height) + location.top

    local twentyPercentX = math.floor(.5 + 0.2 * width) + location.left
    local fortyPercentX = math.floor(.5 + 0.4 * width) + location.left
    local sixtyPercentX = math.floor(.5 + 0.6 * width) + location.left
    local eightyPercentX = math.floor(.5 + 0.8 * width) + location.left

    -- fill with solid color
    self:_drawSquare(location, color)

    -- block out notches in the E by filling with background color
    if orientation == 'right' then
      self._array:narrow(1, twentyPercentY, fortyPercentY - twentyPercentY):
                  narrow(2, fortyPercentX, location.right - fortyPercentX):
                  fill(kwargs.bgColor)
      self._array:narrow(1, sixtyPercentY, eightyPercentY - sixtyPercentY):
                  narrow(2, fortyPercentX, location.right - fortyPercentX):
                  fill(kwargs.bgColor)
    elseif orientation == 'left' then
      self._array:narrow(1, twentyPercentY, fortyPercentY - twentyPercentY):
                  narrow(2, location.left, sixtyPercentX - location.left):
                  fill(kwargs.bgColor)
      self._array:narrow(1, sixtyPercentY, eightyPercentY - sixtyPercentY):
                  narrow(2, location.left, sixtyPercentX - location.left):
                  fill(kwargs.bgColor)
    elseif orientation == 'up' then
      self._array:narrow(1, location.top, sixtyPercentY - location.top):
                  narrow(2, twentyPercentX, fortyPercentX - twentyPercentX):
                  fill(kwargs.bgColor)
      self._array:narrow(1, location.top, sixtyPercentY - location.top):
                  narrow(2, sixtyPercentX, eightyPercentX - sixtyPercentX):
                  fill(kwargs.bgColor)
    elseif orientation == 'down' then
      self._array:narrow(1, fortyPercentY, location.bottom - fortyPercentY):
                  narrow(2, twentyPercentX, fortyPercentX - twentyPercentX):
                  fill(kwargs.bgColor)
      self._array:narrow(1, fortyPercentY, location.bottom - fortyPercentY):
                  narrow(2, sixtyPercentX, eightyPercentX - sixtyPercentX):
                  fill(kwargs.bgColor)
    else
      error('Unrecognized orientation: ' .. orientation)
    end
  end

  --[[ Draw object of specified type

  Keyword arguments:

  *   `optotype`, (string in {'Square', 'E'}) Type of object to draw.
  *   `location`, (table) {top = ..., bottom = ..., left = ..., right = ...}
      Coords.
  *   `color`, (table) RGB values for the color to draw the object.
  *   `orientation`, (string in {'left', 'right', up', 'down'}) Orientation of
      E.
  ]]
  function env:drawObject(opt)
    assert(type(opt.optotype) == 'string')
    assert(type(opt.location) == 'table')
    assert(type(opt.color) == 'table')
    assert(type(opt.orientation) == 'string')
    if opt.optotype == 'Square' then
      self:_drawSquare(opt.location, opt.color)
    elseif opt.optotype == 'E' then
      self:_drawE(opt.location, opt.color, opt.orientation)
    else
      error('Unrecognized object type: ' .. opt.type)
    end
  end

  -- Create the image tensor to display
  function env:renderArray(arrayData)
    if not self._array then
      self._array = tensor.ByteTensor(self.targetPixels.height,
                                      self.targetPixels.width,
                                      3)
    end
    self._array:fill(kwargs.bgColor)

    -- draw objects
    for i = 1, #arrayData.location do
      local location = {
          left = arrayData.location[i][1] * self._hFactor,
          right = (arrayData.location[i][1] + kwargs.grid.step) * self._hFactor,
          top = arrayData.location[i][2] * self._vFactor,
          bottom = (arrayData.location[i][2] + kwargs.grid.step) *
            self._vFactor,
      }

      self:drawObject{location = location,
                      color = arrayData.color[i],
                      optotype = arrayData.optotype[i],
                      orientation = arrayData.orientation[i]}
    end

    return self._array
  end

  -- Display the study array for kwargs.studyTime frames
  function env:studyPhase()
    self.currentTrial.studyArrayData = self:getStudyArrayData()
    psychlab_helpers.addTargetImage(self,
                           self:renderArray(self.currentTrial.studyArrayData),
                           kwargs.targetSize)

    if not kwargs.selfPaced then
      self.pac:addTimer{
          name = 'study_timer',
          timeout = kwargs.studyTime,
          callback = function(...) return self.delayPhase(self) end
      }
    else
      self.currentTrial.endStudyPhaseButtonLoc, _ =
        psychlab_helpers.randomFrom(self._endStudyButtonLocs)
      self:addEndStudyPhaseButton(self.currentTrial.endStudyPhaseButtonLoc)
    end
  end

  -- Remove the study array and display a blank screen (background color) for a
  -- random number of frames sampled from kwargs.delayTimes
  function env:delayPhase()
    -- in self-paced mode, give a reward for getting this far
    self.pac:addReward(self._rewardToDeliver)

    self.currentTrial.delayTime, _ = psychlab_helpers.randomFrom(
      kwargs.delayTimes)
    if kwargs.selfPaced then
      self.pac:removeWidget('endStudyPhaseButton')
    end
    self.pac:removeWidget('target')
    self.pac:addTimer{
        name = 'delay_timer',
        timeout = self.currentTrial.delayTime,
        callback = function(...) return self.preresponsePhase(self) end
    }
  end

  function env:preresponsePhase()
    if kwargs.requireAtCenterPreresponse then
      self.pac:addWidget{
          name = 'preresponse_button',
          image = self.images.preresponseButton,
          posAbs = psychlab_helpers.getUpperLeftFromCenter(
              {self.screenSize.width / 2, self.screenSize.height / 2},
              kwargs.preresponseButtonDiameter
          ),
          sizeAbs = {kwargs.preresponseButtonDiameter,
                     kwargs.preresponseButtonDiameter},
          mouseHoverCallback = function(...) return self.testPhase(self) end,
        }
    else
      self:testPhase()
    end
  end

  -- Display the test array and wait for the subject to respond
  function env:testPhase()
    if kwargs.requireAtCenterPreresponse then
      self.pac:removeWidget('preresponse_button')
    end
    self.currentTrial.testArrayData, self.currentTrial.isNew =
      self:getTestArrayData(self.currentTrial.studyArrayData)
    psychlab_helpers.addTargetImage(self,
                           self:renderArray(self.currentTrial.testArrayData),
                           kwargs.targetSize)
    self.pac:addTimer{
        name = 'preresponse_timer',
        timeout = kwargs.preresponseTime,
        callback = function(...) return
            self.addResponseButtons(self, self.currentTrial.isNew) end
    }

    -- Measure time till response in microseconds
    self._responseStartTime = game:episodeTimeSeconds()
    self.currentTrial.responseSteps = 0
  end

  -- Remove the test array
  function env:removeArray()
    -- remove the image and response buttons
    self.pac:removeWidget('target')
    self.pac:removeWidget('newButton')
    self.pac:removeWidget('oldButton')
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
