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
local psychlab_staircase = require 'levels.contributed.psychlab.factories.staircase'
local helpers = require 'common.helpers'
local image = require 'dmlab.system.image'
local point_and_click = require 'factories.psychlab.point_and_click'
local random = require 'common.random'
local tensor = require 'dmlab.system.tensor'

--[[ This task is a variant on visual search in which the subject must find
the odd one out among an array of objects.

The objects, apart from the odd one out, are either identical, or at least
locally similar - for example, there might be a colour or size gradient
in operation, but objects are at least similar in colour and size to their
neighbours. The odd one out, however, is different, either in colour, size
or shape.

The goal of the subject is to look to the odd one out object.
]]

local SHAPE_HEIGHT, SHAPE_WIDTH = 5, 5
-- The shapes come in complementary pairs.
local SHAPES = {
    -- First pair: hollow shape and pointy shape
    {
        {1, 1, 1, 1, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
        {1, 1, 1, 1, 1},
    },
    {
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {1, 1, 1, 1, 1},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
    },
    -- Second pair: vertical shape and horizontal shape
    {
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 1, 0, 0},
    },
    {
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {1, 1, 1, 1, 1},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
    },
}

local COLORS = {
    {255, 0, 0},    -- Red
    {255, 255, 0},  -- Yellow
    {0, 255, 0},    -- Green
    {0, 255, 255},  -- Cyan
    {0, 0, 255},    -- Blue
    {255, 0, 255},  -- Magenta
}

local TIME_TO_FIXATE_CROSS = 1 -- in frames
local FAST_INTER_TRIAL_INTERVAL = 1 -- in frames
local TRIAL_TIMEOUT = 300 -- in frames
local SCREEN_HEIGHT, SCREEN_WIDTH = 512, 512

local EPISODE_LENGTH_SECONDS = 150
local TRIALS_PER_EPISODE_CAP = 125

-- How often to update the animation
local INTERFRAME_INTERVAL = 4

local BG_COLOR = {0, 0, 0}
local WHITE_BUTTON_COLOR = {255, 255, 255}
local GREEN_BUTTON_COLOR = {100, 255, 100}
local RED_BUTTON_COLOR = {255, 100, 100}
local TRIAL_TYPES = {'color', 'shape', 'orientation', 'motion'}
local SET_SIZES = {3, 5, 7, 9, 11, 13, 15}

local FIXATION_SIZE = 0.1
local FIXATION_COLOR = {255, 0, 0} -- RGB

local FIXATION_REWARD = 0
local INCORRECT_REWARD = 0
local CORRECT_REWARD_SEQUENCE = 1

-- Staircase parameters
local PROBE_PROBABILITY = 0.1
local FRACTION_TO_PROMOTE = 1.0
local FRACTION_TO_DEMOTE = 0.5

local MAX_STEPS_OFF_SCREEN = 300  -- 5 seconds

-- When choiceMode is 'binary', we use invisible widgets in front of the
-- shape widgets
local BACK_LAYER = 1
local FRONT_LAYER = 2

local factory = {}

function factory.createLevelApi(kwargs)
  kwargs.episodeLengthSeconds = kwargs.episodeLengthSeconds or
      EPISODE_LENGTH_SECONDS
  kwargs.trialsPerEpisodeCap = kwargs.trialsPerEpisodeCap or
      TRIALS_PER_EPISODE_CAP
  kwargs.bgColor = kwargs.bgColor or BG_COLOR
  kwargs.fixationColor = kwargs.fixationColor or FIXATION_COLOR
  kwargs.colors = kwargs.colors or COLORS
  kwargs.setSizes = kwargs.setSizes or SET_SIZES
  kwargs.shapeSize = kwargs.shapeSize or nil
  kwargs.trialTypes = kwargs.trialTypes or TRIAL_TYPES
  kwargs.incorrectReward = kwargs.incorrectReward or INCORRECT_REWARD
  kwargs.correctRewardSequence = kwargs.correctRewardSequence or
      CORRECT_REWARD_SEQUENCE
  kwargs.fractionToPromote = kwargs.fractionToPromote or FRACTION_TO_PROMOTE
  kwargs.fractionToDemote = kwargs.fractionToDemote or FRACTION_TO_DEMOTE
  kwargs.probeProbability = kwargs.probeProbability or PROBE_PROBABILITY
  kwargs.fixedTestLength = kwargs.fixedTestLength or false
  kwargs.initialDifficultyLevel = kwargs.initialDifficultyLevel or 1
  kwargs.maxStepsOffScreen = kwargs.maxStepsOffScreen or MAX_STEPS_OFF_SCREEN
  -- When twin is false, the screen shows a single array of objects.
  -- When twin is true, there are separate arrays on the left and right
  -- sides of the screen, with only one containing an odd one out.
  kwargs.twin = kwargs.twin or false
  -- When choiceMode is 'saccade', the agent should saccade to the odd one out.
  -- When choiceMode is 'binary'. the agent only needs to indicate, by looking
  -- left or right, which side has the odd one out.
  kwargs.choiceMode = kwargs.choiceMode or 'saccade'

  -- Class definition for odd_one_out psychlab environment.
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
    psychlab_helpers.addFixation(self, FIXATION_SIZE)

    self.currentTrial = {}

    psychlab_helpers.setTrialsPerEpisodeCap(self, kwargs.trialsPerEpisodeCap)

    -- initialize the adaptive staircase procedure
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

  -- Creates image tensor for fixation.
  function env:setupImages()
    self.images = {}

    self.images.fixation = psychlab_helpers.getFixationImage(
      self.screenSize, kwargs.bgColor, kwargs.fixationColor,
      FIXATION_SIZE)

    if kwargs.twin and
        (kwargs.choiceMode == 'button' or kwargs.choiceMode == 'sidebar') then
      local h, w
      if kwargs.choiceMode == 'button' then
        h = 0.1 * self.screenSize.height
        w = 0.1 * self.screenSize.width
      elseif kwargs.choiceMode == 'sidebar' then
        h = self.screenSize.height
        w = 0.05 * self.screenSize.width
      end
      self.images.whiteButton = tensor.ByteTensor(h, w, 3):fill(
          WHITE_BUTTON_COLOR)
      self.images.greenButton = tensor.ByteTensor(h, w, 3):fill(
          GREEN_BUTTON_COLOR)
      self.images.redButton = tensor.ByteTensor(h, w, 3):fill(
          RED_BUTTON_COLOR)
    end
  end

  function env:finishTrial(delay)
    self.currentTrial.blockId = self.blockId
    self.currentTrial.reactionTime =
      game:episodeTimeSeconds() - self._currentTrialStartTime
    self.staircase:step(self.currentTrial.correct == 1)

    self.currentTrial.stepCount = self.pac:elapsedSteps()
    psychlab_helpers.publishTrialData(self.currentTrial, kwargs.schema)
    self.pac:clearTimers()
    psychlab_helpers.finishTrialCommon(self, delay, FIXATION_SIZE)
  end

  function env:fixationCallback(name, mousePos, hoverTime, userData)
    if hoverTime == TIME_TO_FIXATE_CROSS then
      self.pac:addReward(FIXATION_REWARD)
      self.pac:removeWidget('fixation')
      self.pac:removeWidget('center_of_fixation')
      self.currentTrial.reward = 0
      self:addArray()
      self.pac:addTimer{
          name = 'trial_timeout',
          timeout = TRIAL_TIMEOUT,
          callback = function(...) return self:trialTimeoutCallback() end
      }

      self.currentTrial.trialId = self.trialId
      self.trialId = self.trialId + 1

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
    self:finishTrial(FAST_INTER_TRIAL_INTERVAL)
  end

  function env:onHoverEndIncorrect(name, mousePos, hoverTime, userData)
    -- Only used if choiceMode is in {'binary', 'button', 'forfeit', 'sidebar'}
    self.currentTrial.response = name
    self.currentTrial.correct = 0
    self.currentTrial.reward = kwargs.incorrectReward
    self.pac:addReward(self.currentTrial.reward)
    self:finishTrial(FAST_INTER_TRIAL_INTERVAL)
  end

  function env:correctResponseCallback(name, mousePos, hoverTime, userData)
    self.pac:updateWidget(name, self.images.greenButton)
  end

  function env:incorrectResponseCallback(name, mousePos, hoverTime, userData)
    self.pac:updateWidget(name, self.images.redButton)
  end

  function env:trialTimeoutCallback()
    -- Trial times out if agent does not fixate on odd one out.
    self.currentTrial.correct = 0
    self.currentTrial.reward = kwargs.incorrectReward
    self.pac:addReward(self.currentTrial.reward)
    self:finishTrial(FAST_INTER_TRIAL_INTERVAL)
  end

  function env:displayMotion()
    self.animationStep = (self.animationStep % #self.animationFrames) + 1

    self.pac:updateWidget(
        self.correctWidget, self.animationFrames[self.animationStep])
    self.pac:addTimer{
        name = 'animation_heartbeat',
        timeout = INTERFRAME_INTERVAL,
        callback = function(...) self:displayMotion() end
    }
  end

  function env:addArray()
    local setSize = self.staircase:parameter()
    local gridHeight, gridWidth = setSize, setSize
    local cellHeight, cellWidth
    if kwargs.shapeSize then
      cellHeight = math.floor(SCREEN_HEIGHT / kwargs.shapeSize)
      cellWidth = math.floor(SCREEN_WIDTH / kwargs.shapeSize)
    elseif kwargs.twin and
        (kwargs.choiceMode == 'button' or kwargs.choiceMode == 'sidebar') then
      cellHeight = math.floor(0.9 * SCREEN_HEIGHT / gridHeight)
      cellWidth = math.floor(0.9 * SCREEN_WIDTH / gridWidth)
    else
      cellHeight = math.floor(SCREEN_HEIGHT / gridHeight)
      cellWidth = math.floor(SCREEN_WIDTH / gridWidth)
    end
    local vPad, hPad = math.floor(cellHeight / 4), math.floor(cellWidth / 4)
    local objectHeight, objectWidth = cellHeight - vPad, cellWidth - hPad
    local vOffset, hOffset = 0, 0
    if kwargs.shapeSize then
      vOffset = (kwargs.shapeSize - gridHeight) * cellHeight / 2
      hOffset = (kwargs.shapeSize - gridWidth) * cellWidth / 2
    end

    local trialType = psychlab_helpers.randomFrom(kwargs.trialTypes)
    self.currentTrial.trialType = trialType
    self.currentTrial.setSize = setSize
    self.currentTrial.difficultyLevel = self.staircase:getDifficultyLevel()

    local size = {objectWidth / SCREEN_WIDTH, objectHeight / SCREEN_HEIGHT}
    -- Point and click works in fractional coordinates, but the images we pass
    -- in have definite integer dimensions. Because our grids are not
    -- powers of two, we occasionally get fatal rounding errors.
    -- The following two lines are part of a workaround for this.
    size[1] = size[1] + 1 / (4 * SCREEN_WIDTH)
    size[2] = size[2] + 1 / (4 * SCREEN_HEIGHT)
    local iCenter = math.ceil(gridHeight / 2)
    local jCenter = math.ceil(gridWidth / 2)
    -- Choose which cell is the odd one out
    local iCorrect, jCorrect
    repeat
      iCorrect = random:uniformInt(1, gridHeight)
      jCorrect = random:uniformInt(1, gridWidth)
    until iCorrect ~= iCenter and jCorrect ~= jCenter

    -- Choose colors and shapes of main objects and odd one out
    local mainShape, mainColor, oddShape, oddColor
    mainColor = random:uniformInt(1, #kwargs.colors)
    oddColor = mainColor
    if trialType == 'color' then
      mainShape = random:uniformInt(1, #SHAPES)
      oddShape = mainShape
      repeat
        oddColor = random:uniformInt(1, #kwargs.colors)
      until oddColor ~= mainColor
    end
    if trialType == 'shape' then
      mainShape = random:uniformInt(1, 2)
      oddShape = 3 - mainShape
    end
    if trialType == 'orientation' then
      mainShape = random:uniformInt(3, 4)
      oddShape = 7 - mainShape
    end
    if trialType == 'motion' then
      -- Motion trials use the orientation shapes
      mainShape = random:uniformInt(3, 4)
      oddShape = mainShape
    end

    local mainImage = self:createImage(
        SHAPES[mainShape], kwargs.colors[mainColor], objectHeight, objectWidth)
    local oddImage = self:createImage(
        SHAPES[oddShape], kwargs.colors[oddColor], objectHeight, objectWidth)

    local middle = math.ceil(gridWidth / 2)
    for i = 1, gridHeight do
      for j = 1, gridWidth do
        if not (kwargs.twin and j == middle) then
          local pos = {
              (hOffset + (j - 1) * cellWidth + random:uniformInt(1, hPad - 1)) /
                  SCREEN_WIDTH,
              (vOffset + (i - 1) * cellHeight +
               random:uniformInt(1, vPad - 1)) /
                  SCREEN_HEIGHT
          }
          if kwargs.twin then
            if kwargs.choiceMode == 'button' then
              pos[1] = pos[1] + 0.05
            elseif kwargs.choiceMode == 'sidebar' then
              pos[1] = pos[1] + 0.05
              pos[2] = pos[2] + 0.05
            end
          end
          -- The following two lines are the rest of the workaround
          -- for rounding errors. See comment above.
          pos[1] = pos[1] - 1 / (4 * SCREEN_WIDTH)
          pos[2] = pos[2] - 1 / (4 * SCREEN_HEIGHT)

          local image, callback = mainImage, nil
          if i == iCorrect and j == jCorrect then
            image, callback = oddImage, self.onHoverEndCorrect
          end
          if kwargs.twin and kwargs.choiceMode ~= 'saccade' and
              kwargs.choiceMode ~= 'forfeit' then
            -- For binary, button, or sidebar, saccade to odd one out
            -- doesn't end trial.
            callback = nil
          end
          self.pac:addWidget{
              name = 'image_' .. i .. '_' .. j,
              pos = pos,
              size = size,
              image = image,
              imageLayer = BACK_LAYER,
              mouseHoverCallback = callback,
          }
        end
      end
    end

    if trialType == 'motion' then
      self.correctWidget = 'image_' .. iCorrect .. '_' .. jCorrect
      self.animationFrames = {}
      local color = kwargs.colors[mainColor]
      local shape
      local offsets = {0, 1, 0, -1}
      for t = 1, 4 do
        if mainShape == 3 then
          -- vertical
          local row = {0, 0, 0, 0, 0}
          row[3 + offsets[t]] = 1
          shape = {row, row, row, row, row}
        else
          -- horizontal
          local row = {0, 0, 0, 0, 0}
          shape = {row, row, row, row, row}
          shape[3 + offsets[t]] = {1, 1, 1, 1, 1}
        end
        self.animationFrames[t] = self:createImage(
            shape, color, objectHeight, objectWidth)
      end
      self.animationStep = 0
      self:displayMotion()
    end

    local correctLeft = jCorrect < middle
    if kwargs.choiceMode == 'binary' then
      self:addInvisibleResponseWidget(true, correctLeft)  -- left side
      self:addInvisibleResponseWidget(false, not correctLeft)  -- right side
    end
    if kwargs.choiceMode == 'forfeit' then
      self:addInvisibleResponseWidget(not correctLeft, false)  -- wrong side
    end
    if kwargs.choiceMode == 'button' or kwargs.choiceMode == 'sidebar' then
      self:addResponseButtons(correctLeft)
    end
  end

  function env:addInvisibleResponseWidget(leftSide, correct)
    -- Add invisible widget covering one side of the array.
    -- Agent is required to respond by looking to the side containing
    -- the odd one out.
    local callback
    if correct then
      callback = self.onHoverEndCorrect
    else
      callback = self.onHoverEndIncorrect
    end
    if leftSide then
      self.pac:addWidget{
          name = 'respond_left',
          pos = {0.0, 0.0},
          size = {0.4, 1.0},
          imageLayer = FRONT_LAYER,
          mouseHoverCallback = callback,
      }
    else
      self.pac:addWidget{
          name = 'respond_right',
          pos = {0.6, 0.0},
          size = {0.4, 1.0},
          imageLayer = FRONT_LAYER,
          mouseHoverCallback = callback,
      }
    end
  end

  function env:addResponseButtons(isLeft)
    -- Add sidebar buttons at left and right.
    -- Agent is required to respond by looking to the sidebar button
    -- on the side containing the odd one out.
    local leftResponseCallback, rightResponseCallback
    local hoverEndLeft, hoverEndRight
    if isLeft then
      leftResponseCallback = self.correctResponseCallback
      rightResponseCallback = self.incorrectResponseCallback
      hoverEndLeft = self.onHoverEndCorrect
      hoverEndRight = self.onHoverEndIncorrect
    else
      leftResponseCallback = self.incorrectResponseCallback
      rightResponseCallback = self.correctResponseCallback
      hoverEndLeft = self.onHoverEndIncorrect
      hoverEndRight = self.onHoverEndCorrect
    end
    local leftPos, rightPos, size
    if kwargs.choiceMode == 'button' then
      leftPos = {0.35, 0.9}
      rightPos = {0.55, 0.9}
      size = {0.1, 0.1}
    elseif kwargs.choiceMode == 'sidebar' then
      leftPos = {0.0, 0.0}
      rightPos = {0.95, 0.0}
      size = {0.05, 1.0}
    end
    self.pac:addWidget{
        name = 'respond_left',
        pos = leftPos,
        size = size,
        image = self.images.whiteButton,
        imageLayer = FRONT_LAYER,
        mouseHoverCallback = leftResponseCallback,
        mouseHoverEndCallback = hoverEndLeft,
    }
    self.pac:addWidget{
        name = 'respond_right',
        pos = rightPos,
        size = size,
        image = self.images.whiteButton,
        imageLayer = FRONT_LAYER,
        mouseHoverCallback = rightResponseCallback,
        mouseHoverEndCallback = hoverEndRight,
    }
  end

  function env:removeArray()
    self.pac:clearWidgets()
  end

  function env:createImage(shape, color, objectHeight, objectWidth)
    local unscaled = tensor.ByteTensor(SHAPE_HEIGHT, SHAPE_WIDTH, 3)
    for k = 1, 3 do
      local plane = unscaled:select(3, k)
      plane:val(shape)
      plane:mul(color[k])
    end
    local scaled = image.scale(
        unscaled, objectHeight, objectWidth, 'nearest')
    return scaled
  end

  local screenSize = {width = SCREEN_WIDTH, height = SCREEN_HEIGHT}
  return psychlab_factory.createLevelApi{
      env = point_and_click,
      envOpts = {environment = env, screenSize = screenSize,
                 maxStepsOffScreen = kwargs.maxStepsOffScreen},
      episodeLengthSeconds = kwargs.episodeLengthSeconds
  }
end

return factory
