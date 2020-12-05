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
local log = require 'common.log'
local psychlab_factory = require 'factories.psychlab.factory'
local psychlab_helpers = require 'factories.psychlab.helpers'
local helpers = require 'common.helpers'
local image = require 'dmlab.system.image'
local point_and_click = require 'factories.psychlab.point_and_click'
local random = require 'common.random'
local tensor = require 'dmlab.system.tensor'
local psychlab_staircase = require 'levels.contributed.psychlab.factories.staircase'

--[[ This is a visual search task with an adaptive staircase procedure that
increases or decreases the task difficulty level as appropriate for the
tested agent.

The target is always a magenta 'T'.
The goal of the task is to look to the right button if the target is present on
the screen, or look to the left button if the target is not visble. The screen
contains some randomly-placed objects, one of which may be the target 'T'. The
other objects are all distractors.

Arguments:

`searchMode`:

---- `shape`: Distractors are non-T red shapes chosen at random.
---- `color`: Distractors are non-red T shapes chosen at random.
---- `conjunction`: Distractors are non(red T) shapes.
---- `interleaved`: Randomly pick `shape`, `color` or `conjunction` each trial.
]]

-- The first shape is the target shape.
local SHAPES = {
    {
        {1, 1, 1, 0},
        {0, 1, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 0, 0}
    },  -- T
    {
        {0, 1, 0, 0},
        {0, 1, 0, 0},
        {1, 1, 1, 0},
        {0, 0, 0, 0}
    },  -- reverse T
    {
        {1, 0, 0, 0},
        {1, 0, 0, 0},
        {1, 1, 1, 0},
        {0, 0, 0, 0}
    },  -- L
    {
        {0, 0, 1, 0},
        {0, 0, 1, 0},
        {1, 1, 1, 0},
        {0, 0, 0, 0}
    },  -- reverse L
    {
        {1, 1, 1, 0},
        {1, 0, 0, 0},
        {1, 0, 0, 0},
        {0, 0, 0, 0}
    },  -- R
    {
        {1, 1, 1, 0},
        {0, 0, 1, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 0}
    },  -- reverse R
    {
        {0, 1, 1, 0},
        {0, 1, 0, 0},
        {1, 1, 0, 0},
        {0, 0, 0, 0}
    },  -- S
    {
        {1, 1, 0, 0},
        {0, 1, 0, 0},
        {0, 1, 1, 0},
        {0, 0, 0, 0}
    },  -- Z
}

-- These are RGB values, they have a fixed luminance in HSL space.
-- The first color is the target color.
local COLORS = {
    {255, 0, 191},  -- magenta
    {255, 191, 0},  -- sunflower yellow
    {0, 255, 255},  -- light blue
    {0, 63, 255},   -- dark blue
    {127, 0, 255},  -- purple
}

local DEFAULT_SEARCH_MODE = 'interleaved'

local TIME_TO_FIXATE_CROSS = 1 -- in frames
local FAST_INTER_TRIAL_INTERVAL = 1 -- in frames
local SCREEN_SIZE = {width = 512, height = 512}
local BG_COLOR = {255, 255, 255}
local EPISODE_LENGTH_SECONDS = 150
local TRIALS_PER_EPISODE_CAP = math.huge

local GRID_CELL_SIZE = 32  -- size of one grid cell, in pixels
local BUTTON_SIZE = 0.1
local SET_SIZES = {1, 1, 1, 2, 2, 4, 8, 16, 24, 32}
local TARGET_SIZE = 56

local FIXATION_SIZE = 0.1
local FIXATION_COLOR = {255, 0, 0} -- RGB

local FIXATION_REWARD = 0
local INCORRECT_REWARD = 0
local CORRECT_REWARD_SEQUENCE = 1

-- Staircase parameters
local FRACTION_TO_PROMOTE = 1.0
local FRACTION_TO_DEMOTE = 0.75
local PROBE_PROBABILITY = 0.1

local MAX_STEPS_OFF_SCREEN = 300  -- 5 seconds

local ARG = {}

local function parseArgs(arg)
  if arg == nil then
    return arg
  end
  return arg
end

local factory = {}

function factory.createLevelApi(kwargs)
  kwargs.episodeLengthSeconds = kwargs.episodeLengthSeconds or
      EPISODE_LENGTH_SECONDS
  kwargs.trialsPerEpisodeCap = kwargs.trialsPerEpisodeCap or
      TRIALS_PER_EPISODE_CAP
  kwargs.shapes = kwargs.shapes or SHAPES
  kwargs.colors = kwargs.colors or COLORS
  kwargs.timeToFixateCross = kwargs.timeToFixateCross or TIME_TO_FIXATE_CROSS
  kwargs.fastInterTrialInterval = kwargs.fastInterTrialInterval or
      FAST_INTER_TRIAL_INTERVAL
  kwargs.screenSize = kwargs.screenSize or SCREEN_SIZE
  kwargs.bgColor = kwargs.bgColor or BG_COLOR
  kwargs.gridCellSize = kwargs.gridCellSize or GRID_CELL_SIZE
  kwargs.buttonSize = kwargs.buttonSize or BUTTON_SIZE
  kwargs.gridHeight = math.floor(
      kwargs.screenSize.height * (1 - 2 * kwargs.buttonSize))
  kwargs.gridWidth = math.floor(
      kwargs.screenSize.width * (1 - 2 * kwargs.buttonSize))
  kwargs.setSizes = kwargs.setSizes or SET_SIZES
  kwargs.correctRewardSequence = kwargs.correctRewardSequence or
      CORRECT_REWARD_SEQUENCE
  kwargs.targetSize = kwargs.targetSize or TARGET_SIZE
  kwargs.fixationSize = kwargs.fixationSize or FIXATION_SIZE
  kwargs.fixationColor = kwargs.fixationColor or FIXATION_COLOR
  kwargs.fixationReward = kwargs.fixationReward or FIXATION_REWARD
  kwargs.incorrectReward = kwargs.incorrectReward or INCORRECT_REWARD
  kwargs.fractionToPromote = kwargs.fractionToPromote or FRACTION_TO_PROMOTE
  kwargs.fractionToDemote = kwargs.fractionToDemote or FRACTION_TO_DEMOTE
  kwargs.probeProbability = kwargs.probeProbability or PROBE_PROBABILITY
  kwargs.fixedTestLength = kwargs.fixedTestLength or false
  kwargs.initialDifficultyLevel = kwargs.initialDifficultyLevel or 1
  kwargs.maxStepsOffScreen = kwargs.maxStepsOffScreen or MAX_STEPS_OFF_SCREEN
  kwargs.searchMode = kwargs.searchMode or DEFAULT_SEARCH_MODE

  local validSearchModes = {
      shape = true,
      color = true,
      conjunction = true,
      interleaved = true
  }
  assert(validSearchModes[kwargs.searchMode],
         'searchMode should be shape, color, conjunction, or interleaved'
  )

  -- Class definition for visual_search psychlab environment.
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
    ARG.jitter = opts.jitter or false

    log.info('ARGs in _init:\n' .. helpers.tostring(ARG))

    ARG = parseArgs(ARG)
    log.info('visual_search args parsed correctly')

    self.screenSize = opts.screenSize

    -- Use 'screenSize' to compute the actual size in pixels for each image.
    self.sizeInPixels = {
        fixationHeight = kwargs.fixationSize * self.screenSize.height,
        fixationWidth = kwargs.fixationSize * self.screenSize.width
    }

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

  -- Creates image Tensors for red/green/white/black buttons and fixation.
  function env:setupImages()
    self.images = {}

    self.images.fixation = psychlab_helpers.getFixationImage(
      self.screenSize, kwargs.bgColor, kwargs.fixationColor,
      kwargs.fixationSize)

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

    self.images.whiteImage = tensor.ByteTensor(h, w, 3):fill(255)
    self.images.blackImage = tensor.ByteTensor(h, w, 3)

    self.target = tensor.ByteTensor(256, 256, 3)
  end

  function env:finishTrial(delay)
    self.currentTrial.blockId = self.blockId
    self.currentTrial.reactionTime =
      game:episodeTimeSeconds() - self._currentTrialStartTime
    self.staircase:step(self.currentTrial.correct == 1)

    self.currentTrial.stepCount = self.pac:elapsedSteps()
    psychlab_helpers.publishTrialData(self.currentTrial, kwargs.schema)
    psychlab_helpers.finishTrialCommon(self, delay, kwargs.fixationSize)
  end

  function env:fixationCallback(name, mousePos, hoverTime, userData)
    if hoverTime == kwargs.timeToFixateCross then
      self.pac:addReward(kwargs.fixationReward)
      self.pac:removeWidget('fixation')
      self.pac:removeWidget('center_of_fixation')
      self.currentTrial.reward = 0
      self.currentTrial.trialId = self.trialId
      self.trialId = self.trialId + 1
      self:addArray()

      -- Measure reaction time since the trial started.
      self._currentTrialStartTime = game:episodeTimeSeconds()
      self.pac:resetSteps()
    end
  end

  function env:onHoverEndCorrect(name, mousePos, hoverTime, userData)
    -- Reward if this is the first "hoverEnd" event for this trial.
    self.currentTrial.response = name
    self.currentTrial.correct = 1
    self.currentTrial.reward = self.staircase:correctReward()
    self.pac:addReward(self.currentTrial.reward)
    self:finishTrial(kwargs.fastInterTrialInterval)
  end

  function env:onHoverEndIncorrect(name, mousePos, hoverTime, userData)
    -- Reward if this is the first "hoverEnd" event for this trial.
    self.currentTrial.response = name
    self.currentTrial.correct = 0
    self.currentTrial.reward = kwargs.incorrectReward
    self.pac:addReward(self.currentTrial.reward)
    self:finishTrial(kwargs.fastInterTrialInterval)
  end

  function env:correctResponseCallback(name, mousePos, hoverTime, userData)
    self.pac:updateWidget(name, self.images.greenImage)
  end

  function env:incorrectResponseCallback(name, mousePos, hoverTime, userData)
    self.pac:updateWidget(name, self.images.redImage)
  end

  function env:addResponseButtons(targetPresent)
    local buttonPosX = 0.5 - kwargs.buttonSize * 1.5
    local buttonSize = {kwargs.buttonSize, kwargs.buttonSize}

    local targetPresentCallback, targetAbsentCallback
    local hoverEndPresent, hoverEndAbsent
    if targetPresent then
      targetPresentCallback = self.correctResponseCallback
      hoverEndPresent = self.onHoverEndCorrect
      targetAbsentCallback = self.incorrectResponseCallback
      hoverEndAbsent = self.onHoverEndIncorrect
    else
      targetPresentCallback = self.incorrectResponseCallback
      hoverEndPresent = self.onHoverEndIncorrect
      targetAbsentCallback = self.correctResponseCallback
      hoverEndAbsent = self.onHoverEndCorrect
    end

    self.pac:addWidget{
        name = 'targetAbsent',
        image = self.images.blackImage,
        pos = {buttonPosX, 1 - kwargs.buttonSize},
        size = buttonSize,
        mouseHoverCallback = targetAbsentCallback,
        mouseHoverEndCallback = hoverEndAbsent,
    }
    self.pac:addWidget{
        name = 'targetPresent',
        image = self.images.blackImage,
        pos = {1 - buttonPosX - kwargs.buttonSize, 1 - kwargs.buttonSize},
        size = buttonSize,
        mouseHoverCallback = targetPresentCallback,
        mouseHoverEndCallback = hoverEndPresent,
    }
  end

  local function getTarget()
    return 1, 1
  end

  local function getRandomDistractor(searchMode)

    local shapeIndex = 1
    local colorIndex = 1

    if searchMode == 'color' then
      colorIndex = random:uniformInt(2, #kwargs.colors)
    elseif searchMode == 'shape' then
      shapeIndex = random:uniformInt(2, #kwargs.shapes)
    elseif searchMode == 'conjunction' then
      -- Randomly sample shapeIndex and colorIndex, with both not == 1.
      local randomIndex = random:uniformInt(1,
                                            #kwargs.shapes * #kwargs.colors - 1)
      shapeIndex = (randomIndex % #kwargs.shapes) + 1
      colorIndex = math.floor(randomIndex / #kwargs.shapes) + 1
    end

    return shapeIndex, colorIndex
  end

  --[[ Given a shape and color, scale the coloured shape and copy it to the
  patch.
  Shape is assumed to be a [4,4] table.
  Patch is assumed to be a [H,W,3] tensor.
  ]]
  local function printToPatch(patch, shape, color)
    local coloredShape = tensor.ByteTensor(4, 4, 3)
    local maskForeground = tensor.ByteTensor(shape)
    local maskBackground = tensor.ByteTensor(shape):fill(1)
    maskBackground:csub(maskForeground)
    for i = 1, 3 do
      local op0 = maskForeground:clone():mul(color[i])
      local op1 = maskBackground:clone():mul(kwargs.bgColor[i])
      op0:cadd(op1)
      coloredShape:select(3, i):copy(op0)
    end
    local shape = patch:shape()
    local coloredShapeScale = image.scale(coloredShape, shape[1], shape[2],
                                          'nearest')
    patch:copy(coloredShapeScale)
  end

  --[[ Returns the coordinate in a 0-based grid of size numRows by numColumns.
  'index' is assumed to be 1-index based (as opposed to 0-index).
  ]]
  local function indexToUpperLeftCoordinates(index, numRows, numColumns,
      rowOffset, colOffset, targetSize)
    local rowIndex = math.floor((index - 1) / numColumns)
    local columnIndex = (index - 1) % numColumns
    local x = rowOffset + rowIndex * targetSize
    local y = colOffset + columnIndex * targetSize
    return x, y
  end

  function env:getSearchImage(targetPresent, setSize, searchMode, targetSize)
    local shapeIndices = {}
    local colorIndices = {}

    local numRows = math.floor(kwargs.gridHeight / targetSize)
    local numColumns = math.floor(kwargs.gridWidth / targetSize)

    -- When gridCellSize does not evenly divide gridHeight there is leftover
    -- width, so shift the grid by rowOffset to distribute leftover space
    -- evenly.)
    local rowOffset = math.floor((kwargs.gridHeight % targetSize) / 2)
    local colOffset = math.floor((kwargs.gridWidth % targetSize) / 2)

    local numCells = numRows * numColumns

    -- If the target is present, it will be the first item; otherwise, the
    -- shape will be a distractor.
    local shapeIndex, colorIndex = getTarget()

    if not targetPresent then
      shapeIndex, colorIndex = getRandomDistractor(searchMode)
    end
    shapeIndices[1] = shapeIndex
    colorIndices[1] = colorIndex

    -- All shapes after the first one will always be distractors
    for i = 2, setSize do
      shapeIndex, colorIndex = getRandomDistractor(searchMode)
      shapeIndices[#shapeIndices + 1] = shapeIndex
      colorIndices[#colorIndices + 1] = colorIndex
    end

    -- Render all of the shapes to their appropriate patches on the grid.
    self.currentTrial.numTs = 0
    self.currentTrial.numReds = 0
    local objectSize = {targetSize, targetSize}
    local grid = tensor.ByteTensor(kwargs.gridHeight, kwargs.gridWidth, 3):fill(
      kwargs.bgColor)
    local filledIndices = tensor.Int64Tensor{range = {numCells}}:shuffle(
      random:generator()):narrow(1, 1, setSize)
    for i = 1, setSize do
      local x, y = indexToUpperLeftCoordinates(filledIndices(i):val(), numRows,
        numColumns, rowOffset, colOffset, targetSize)
      local patch = grid:narrow(2, x + 1, objectSize[1]):narrow(1, y + 1,
        objectSize[2])
      printToPatch(patch, kwargs.shapes[shapeIndices[i]],
        kwargs.colors[colorIndices[i]])
      -- Log the number of red and T-shaped objects.
      if shapeIndices[i] == 1 then
        self.currentTrial.numTs = self.currentTrial.numTs + 1
      end
      if colorIndices[i] == 1 then
        self.currentTrial.numReds = self.currentTrial.numReds + 1
      end
    end

    return grid
  end

  function env:addArray()
    self.currentTrial.targetPresent = psychlab_helpers.randomFrom{true, false}

    if kwargs.searchMode == 'interleaved' then
      self.currentTrial.searchMode =
        psychlab_helpers.randomFrom{'shape', 'color', 'conjunction'}
    else
      self.currentTrial.searchMode = kwargs.searchMode
    end

    self.currentTrial.difficultyLevel = self.staircase:getDifficultyLevel()
    self.currentTrial.setSize = self.staircase:parameter()
    self.currentTrial.targetSize = kwargs.targetSize

    local myImage = self:getSearchImage(self.currentTrial.targetPresent,
      self.currentTrial.setSize, self.currentTrial.searchMode,
      self.currentTrial.targetSize)

    -- Center the search image taking into account the buttons
    self.pac:addWidget{
        name = 'image',
        image = myImage,
        pos = {kwargs.buttonSize, kwargs.buttonSize},
        size = {1 - 2 * kwargs.buttonSize, 1 - 2 * kwargs.buttonSize},
    }

    self:addResponseButtons(self.currentTrial.targetPresent)
  end

  function env:removeArray()
    self.pac:removeWidget('image')
    self.pac:removeWidget('targetPresent')
    self.pac:removeWidget('targetAbsent')
  end

  return psychlab_factory.createLevelApi{
      env = point_and_click,
      envOpts = {environment = env, screenSize = kwargs.screenSize,
                 maxStepsOffScreen = kwargs.maxStepsOffScreen},
      episodeLengthSeconds = kwargs.episodeLengthSeconds
  }
end

return factory
