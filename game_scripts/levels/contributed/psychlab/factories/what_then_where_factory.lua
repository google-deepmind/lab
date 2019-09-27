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

local mnist = require 'datasets.mnist'
local game = require 'dmlab.system.game'
local helpers = require 'common.helpers'
local psychlab_factory = require 'factories.psychlab.factory'
local psychlab_helpers = require 'factories.psychlab.helpers'
local image = require 'dmlab.system.image'
local point_and_click = require 'factories.psychlab.point_and_click'
local random = require 'common.random'
local tensor = require 'dmlab.system.tensor'
local set = require 'common.set'
local psychlab_staircase = require 'levels.contributed.psychlab.factories.staircase'
local log = require 'common.log'

--[[ Each trial consists of a study-what, a study-where and a test phase
separated by two brief delay intervals. The agent must remember what target was
displayed during the study-what period, see where it is located during the
study-where period, and then respond by looking to that location in the test
phase.

This task is based on:

Rao, S. Chenchal, Gregor Rainer, and Earl K. Miller.
"Integration of what and where in the primate prefrontal cortex."
Science 276.5313 (1997): 821-824.
]]

-- setup constant parameters of the task
local TIME_TO_FIXATE_CROSS = 1 -- in frames
local FAST_INTER_TRIAL_INTERVAL = 1 -- in frames
local SCREEN_SIZE = {width = 256, height = 256}
local BG_COLOR = {0, 0, 0}
local EPISODE_LENGTH_SECONDS = 180
local TRIALS_PER_EPISODE_CAP = math.huge

local TARGET_SIZE = 0.25 -- fraction of screen to fill for study what target
local SEARCH_ARRAY_SIZE = 0.95 -- fraction of screen to fill for where array

local STUDY_WHAT_TIME = 90 -- in frames
local STUDY_WHERE_TIME = 90 -- in frames

local CONSTANT_IMAGE_PER_CATEGORY = false

local CATEGORIES_10 = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}

local SEQUENCE = {
    {categories = CATEGORIES_10, delayTimes = {8}},
    {categories = CATEGORIES_10, delayTimes = {16}},
    {categories = CATEGORIES_10, delayTimes = {32}},
    {categories = CATEGORIES_10, delayTimes = {64}},
    {categories = CATEGORIES_10, delayTimes = {128}},
    {categories = CATEGORIES_10, delayTimes = {256}}
}

local ADVANCE_TRIAL_REWARD = 0
local CORRECT_REWARD_SEQUENCE = 1
local INCORRECT_REWARD = 0
local FIXATION_SIZE = 0.1
local FIXATION_COLOR = {255, 0, 0} -- RGB
local CENTER = {0.5, 0.5}
local BUTTON_SIZE = 0.1

-- If REQUIRE_AT_CENTER_PRERESPONSE set to true, then require agent to look to
-- a circle in the center of the screen just before the target. This prevents
-- immediately looking to the target location at start of the studyWhere phase.
local REQUIRE_AT_CENTER_PRERESPONSE = false
local PRERESPONSE_BUTTON_DIAMETER = 21
local PRERESPONSE_BUTTON_COLOR = {255, 255, 255}

-- Staircase parameters
local FRACTION_TO_PROMOTE = 1.0
local FRACTION_TO_DEMOTE = 0.75
local PROBE_PROBABILITY = 0.5

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

local MAX_STEPS_OFF_SCREEN = 300  -- 5 seconds

local factory = {}

function factory.createLevelApi(kwargs)
  kwargs.episodeLengthSeconds = kwargs.episodeLengthSeconds or
      EPISODE_LENGTH_SECONDS
  kwargs.timeToFixateCross = kwargs.timeToFixateCross or TIME_TO_FIXATE_CROSS
  kwargs.fastInterTrialInterval = kwargs.fastInterTrialInterval or
      FAST_INTER_TRIAL_INTERVAL
  kwargs.screenSize = kwargs.screenSize or SCREEN_SIZE
  kwargs.bgColor = kwargs.bgColor or BG_COLOR
  kwargs.trialsPerEpisodeCap = kwargs.trialsPerEpisodeCap or
      TRIALS_PER_EPISODE_CAP
  kwargs.targetSize = kwargs.targetSize or TARGET_SIZE
  kwargs.searchArraySize = kwargs.searchArraySize or SEARCH_ARRAY_SIZE
  kwargs.studyWhatTime = kwargs.studyWhatTime or STUDY_WHAT_TIME
  kwargs.studyWhereTime = kwargs.studyWhereTime or STUDY_WHERE_TIME
  kwargs.correctRewardSequence = kwargs.correctRewardSequence or
      CORRECT_REWARD_SEQUENCE
  kwargs.sequence = kwargs.sequence or SEQUENCE
  kwargs.advanceTrialReward = kwargs.advanceTrialReward or ADVANCE_TRIAL_REWARD
  kwargs.incorrectReward = kwargs.incorrectReward or INCORRECT_REWARD
  kwargs.fixationSize = kwargs.fixationSize or FIXATION_SIZE
  kwargs.fixationColor = kwargs.fixationColor or FIXATION_COLOR
  kwargs.center = kwargs.center or CENTER
  kwargs.buttonSize = kwargs.buttonSize or BUTTON_SIZE
  kwargs.buttons = kwargs.buttons or BUTTONS
  kwargs.buttonPositions = kwargs.buttonPositions or BUTTON_POSITIONS
  kwargs.requireAtCenterPreresponse = kwargs.requireAtCenterPreresponse or
      REQUIRE_AT_CENTER_PRERESPONSE
  kwargs.preresponseButtonDiameter = kwargs.preresponseButtonDiameter or
      PRERESPONSE_BUTTON_DIAMETER
  kwargs.preresponseButtonColor = kwargs.preresponseButtonColor or
      PRERESPONSE_BUTTON_COLOR
  kwargs.fractionToPromote = kwargs.fractionToPromote or FRACTION_TO_PROMOTE
  kwargs.fractionToDemote = kwargs.fractionToDemote or FRACTION_TO_DEMOTE
  kwargs.probeProbability = kwargs.probeProbability or PROBE_PROBABILITY
  kwargs.fixedTestLength = kwargs.fixedTestLength or false
  kwargs.initialDifficultyLevel = kwargs.initialDifficultyLevel or 1
  kwargs.maxStepsOffScreen = kwargs.maxStepsOffScreen or MAX_STEPS_OFF_SCREEN
  kwargs.constantImagePerCategory = kwargs.constantImagePerCategory or
      CONSTANT_IMAGE_PER_CATEGORY

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

    if self.dataset == nil then
      self.dataset = mnist(opts)
    end
    self:setupImages()
    self:wrapDataset()
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

    -- initialize the adaptive staircase procedure
    self.staircase = psychlab_staircase.createStaircase1D{
        sequence = kwargs.sequence,
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
    local wherePixels = psychlab_helpers.getSizeInPixels(self.screenSize,
                                                         kwargs.searchArraySize)
    self.whereTensor = tensor.ByteTensor(wherePixels.height,
                                         wherePixels.width,
                                         3)

    self.images.preresponseButton = self:drawCircle(
        kwargs.preresponseButtonDiameter,
        kwargs.preresponseButtonColor
    )
  end

  function env:getImageOfCategory(category)
    local imageId = random:uniformInt(1, self.dataset:getSize())
    local label = self.dataset:getLabel(imageId)
    local safeLoopCounter = 1
    while label ~= category do
      imageId = random:uniformInt(1, self.dataset:getSize())
      label = self.dataset:getLabel(imageId)
      safeLoopCounter = safeLoopCounter + 1
      if safeLoopCounter == 1000 then
        error("Raise error to avoid infinite loop.")
      end
    end
    return imageId, self.dataset:getImage(imageId):clone()
  end

  function env:getConstantImageOfCategory(category)
    local imageId = kwargs.constantImagePerCategory[category + 1]
    local label = self.dataset:getLabel(imageId)
    return imageId, self.dataset:getImage(imageId):clone()
  end

  function env:getImage(category)
    local id, img
    if kwargs.constantImagePerCategory then
      id, img = self:getConstantImageOfCategory(category)
    else
      id, img = self:getImageOfCategory(category)
    end
    return id, img
  end

  function env:wrapDataset()
    self.wrappedDataset = {}

    function self.wrappedDataset.getNextTarget(self, categories)
      local targetCategory = psychlab_helpers.randomFrom(
          set.toList(categories))
      local targetId, targetImg = self:getImage(targetCategory)
      return targetCategory, targetId, targetImg
    end

    function self.wrappedDataset.getLocations(self, targetCategory, categories)
      local correctLocation = psychlab_helpers.randomFrom(kwargs.buttons)
      local mapObjects = {}
      local categoriesAlreadyPicked = {}
      for _, loc in ipairs(kwargs.buttons) do
        local objectData = {}
        if loc == correctLocation then
          -- add correct object data to map
          objectData.category = targetCategory
        else
          -- add incorrect object data to map
          -- sample categories without replacement
          local availableCategories = set.difference(
              categories,
              set.Set(categoriesAlreadyPicked)
          )
          availableCategories[targetCategory] = nil
          objectData.category = psychlab_helpers.randomFrom(
            set.toList(availableCategories))
          table.insert(categoriesAlreadyPicked, objectData.category)
        end
        objectData.id, objectData.img = self:getImage(objectData.category)
        table.insert(mapObjects, objectData)
      end
      return mapObjects, correctLocation
    end
  end

  function env:finishTrial(delay)
    self.currentTrial.blockId = self.blockId
    self.currentTrial.reactionTime =
      game:episodeTimeSeconds() - self._currentTrialStartTime
    self.currentTrial.responseTime =
      game:episodeTimeSeconds() - self._responseStartTime

    self.staircase:step(self.currentTrial.correct == 1)

    self.currentTrial.stepCount = self.pac:elapsedSteps()
    self.currentTrial.mapObjects = nil  -- avoid logging images
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
      self:studyWhatPhase()
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

  function env:renderArray(mapObjects)
    local h = kwargs.searchArraySize * self.screenSize.height
    local w = kwargs.searchArraySize * self.screenSize.width
    -- Assume h and w are the same
    local scaledObjSize = kwargs.targetSize * h

    local canvas = tensor.ByteTensor(h, w, 3):fill(kwargs.bgColor)

    local upperLefts = {}
    for i, direction in ipairs(kwargs.buttons) do
      if direction == 'north' then
        upperLefts[direction] = {1,
                                 (w / 2) - (scaledObjSize / 2) + 1}
      elseif direction == 'south' then
        upperLefts[direction] = {h - scaledObjSize + 1,
                                 (w / 2) - (scaledObjSize / 2) + 1}
      elseif direction == 'east' then
        upperLefts[direction] = {(h / 2) - (scaledObjSize / 2) + 1,
                                 1}
      elseif direction == 'west' then
        upperLefts[direction] = {(h / 2) - (scaledObjSize / 2) + 1,
                                 w - scaledObjSize + 1}
      end
    end

    for i, direction in ipairs(kwargs.buttons) do
      local upperLeftCoords = upperLefts[direction]
      local dest = canvas:narrow(1, upperLeftCoords[1], scaledObjSize):
                          narrow(2, upperLeftCoords[2], scaledObjSize)
      local scaledObj = psychlab_helpers.scaleImage(mapObjects[i].img,
                                                    scaledObjSize,
                                                    scaledObjSize)
      dest:copy(scaledObj)
    end
    return canvas
  end

  -- Display the target object for kwargs.studyWhatTime frames
  function env:studyWhatPhase()
    local trialData = self.staircase:parameter()
    assert(#trialData.categories >= 4, "Must have at least 4 categories.")
    self._categoriesThisTrial = set.Set(trialData.categories)
    self.currentTrial.delayWhatTime = psychlab_helpers.randomFrom(
        trialData.delayTimes)
    self.currentTrial.delayWhereTime = psychlab_helpers.randomFrom(
        trialData.delayTimes)
    self.currentTrial.difficultyLevel = self.staircase:getDifficultyLevel()
    local targetCategory, targetId, targetImg =
      self.wrappedDataset.getNextTarget(self, self._categoriesThisTrial)
    self.currentTrial.targetCategory = targetCategory
    self.currentTrial.targetId = targetId
    -- self.target gets used by psychlab_helpers.addTargetImage
    -- TODO(jzl): refactor this horrible global state in the helpers
    self.target = self.whatTensor
    psychlab_helpers.addTargetImage(
        self,
        psychlab_helpers.scaleImageToScreenFraction(
            targetImg,
            {height = kwargs.targetSize,
             width = kwargs.targetSize},
            self.screenSize
        ),
        kwargs.targetSize)
    self.pac:addTimer{
        name = 'study_what_timer',
        timeout = kwargs.studyWhatTime,
        callback = function(...) return self.delayWhatPhase(self) end
    }
  end

  -- Remove the study what image and display a blank screen (background color)
  -- for some number of frames.
  function env:delayWhatPhase()
    self.pac:removeWidget('target')
    self.pac:addTimer{
        name = 'delay_what_timer',
        timeout = self.currentTrial.delayWhatTime,
        callback = function(...) return self.studyWherePhase(self) end
    }
  end

  -- Display the search array for kwargs.studyWhereTime frames
  function env:studyWherePhase()
    -- mapObjects is a table indexed by location. Each location maps to a table
    -- containing the data (id, category) for the object at that location.
    local mapObjects, correctLocation = self.wrappedDataset.getLocations(self,
      self.currentTrial.targetCategory, self._categoriesThisTrial)
    self.currentTrial.mapObjects = mapObjects
    self.currentTrial.correctLocation = correctLocation
    -- self.target gets used by psychlab_helpers.addTargetImage
    -- TODO(jzl): refactor this horrible global state in the helpers
    self.target = self.whereTensor
    psychlab_helpers.addTargetImage(
        self,
        self:renderArray(self.currentTrial.mapObjects),
        kwargs.searchArraySize)
    self.pac:addTimer{
        name = 'study_where_timer',
        timeout = kwargs.studyWhereTime,
        callback = function(...) return self.delayWherePhase(self) end
    }
  end

  -- Remove the study where image and display a blank screen (background color)
  -- for some number of frames.
  function env:delayWherePhase()
    self.pac:removeWidget('target')
    self.pac:addTimer{
        name = 'delay_where_timer',
        timeout = self.currentTrial.delayWhereTime,
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
    self:addResponseButtons(self.currentTrial.correctLocation)
    -- Measure time till response in microseconds
    self._responseStartTime = game:episodeTimeSeconds()
    self.currentTrial.responseSteps = 0
  end

  function env:addResponseButtons(correctLocation)
    for _, button in ipairs(kwargs.buttons) do
      local callback
      if button == correctLocation then
        callback = self.correctResponseCallback
      else
        callback = self.incorrectResponseCallback
      end
      self.pac:addWidget{
          name = button,
          image = self.images.whiteImage,
          pos = kwargs.buttonPositions[button],
          size = {kwargs.buttonSize, kwargs.buttonSize},
          mouseHoverCallback = callback,
          mouseHoverEndCallback = self.onHoverEnd,
      }
    end
  end

  -- Remove the test array
  function env:removeArray()
    -- remove the response buttons
    self.pac:removeWidget('north')
    self.pac:removeWidget('south')
    self.pac:removeWidget('east')
    self.pac:removeWidget('west')
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
