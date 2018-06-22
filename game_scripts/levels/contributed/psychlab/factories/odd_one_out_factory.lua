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

-- How often to update the animation
local INTERFRAME_INTERVAL = 4

local BG_COLOR = {0, 0, 0}
local TRIALS_PER_EPISODE_CAP = math.huge

local SET_SIZES = {3, 5, 7, 9, 11, 13, 15}

local FIXATION_SIZE = 0.1
local FIXATION_COLOR = {255, 0, 0} -- RGB

local FIXATION_REWARD = 0
local CORRECT_REWARD = 1
local INCORRECT_REWARD = 0

-- Staircase parameters
local PROBE_PROBABILITY = 0.1
local FRACTION_TO_PROMOTE = 1.0
local FRACTION_TO_DEMOTE = 0.5

local factory = {}

function factory.createLevelApi(kwargs)
  kwargs.bgColor = kwargs.bgColor or BG_COLOR
  kwargs.setSizes = kwargs.setSizes or SET_SIZES

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

    psychlab_helpers.setTrialsPerEpisodeCap(self, TRIALS_PER_EPISODE_CAP)

    -- initialize the adaptive staircase procedure
    self.staircase = psychlab_staircase.createStaircase1D{
        sequence = kwargs.setSizes,
        fractionToPromote = FRACTION_TO_PROMOTE,
        fractionToDemote = FRACTION_TO_DEMOTE,
        probeProbability = PROBE_PROBABILITY,
    }
  end

  -- Creates image tensor for fixation.
  function env:setupImages()
    self.images = {}

    self.images.fixation = psychlab_helpers.getFixationImage(
      self.screenSize, kwargs.bgColor, FIXATION_COLOR,
      FIXATION_SIZE)
  end

  function env:finishTrial(delay)
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
      self:addArray()
      self.pac:addTimer{
        name = 'trial_timeout',
        timeout = TRIAL_TIMEOUT,
        callback = function(...) return self:trialTimeoutCallback() end
      }
      -- Measure reaction time since the trial started.
      self._currentTrialStartTime = game:episodeTimeSeconds()
      self.pac:resetSteps()
    end
  end

  function env:correctResponseCallback(name, mousePos, hoverTime, userData)
    -- Reward if this is the first "hoverEnd" event for this trial.
    self.currentTrial.response = name
    self.currentTrial.correct = 1
    self.pac:addReward(CORRECT_REWARD)
    self:finishTrial(FAST_INTER_TRIAL_INTERVAL)
  end

  function env:trialTimeoutCallback()
    -- Trial times out if agent does not fixate on odd one out.
    self.currentTrial.correct = 0
    self.pac:addReward(INCORRECT_REWARD)
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
    local cellHeight = math.floor(SCREEN_HEIGHT / gridHeight)
    local cellWidth = math.floor(SCREEN_WIDTH / gridWidth)
    local vPad, hPad = math.floor(cellHeight / 4), math.floor(cellWidth / 4)
    local objectHeight, objectWidth = cellHeight - vPad, cellWidth - hPad

    local trialType = psychlab_helpers.randomFrom{
        'color', 'shape', 'orientation', 'motion'}
    self.currentTrial.trialType = trialType
    self.currentTrial.setSize = setSize

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
    mainColor = random:uniformInt(1, #COLORS)
    oddColor = mainColor
    if trialType == 'color' then
      mainShape = random:uniformInt(1, #SHAPES)
      oddShape = mainShape
      repeat
        oddColor = random:uniformInt(1, #COLORS)
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
        SHAPES[mainShape], COLORS[mainColor], objectHeight, objectWidth)
    local oddImage = self:createImage(
        SHAPES[oddShape], COLORS[oddColor], objectHeight, objectWidth)

    for i = 1, gridHeight do
      for j = 1, gridWidth do
        local pos = {
            (j - 1) / gridWidth +
            random:uniformInt(1, hPad - 1) / SCREEN_WIDTH,
            (i - 1) / gridHeight +
            random:uniformInt(1, vPad - 1) / SCREEN_HEIGHT
        }
        -- The following two lines are the rest of the workaround
        -- for rounding errors. See comment above.
        pos[1] = pos[1] - 1 / (4 * SCREEN_WIDTH)
        pos[2] = pos[2] - 1 / (4 * SCREEN_HEIGHT)

        local image, callback = mainImage, nil
        if i == iCorrect and j == jCorrect then
          image, callback = oddImage, self.correctResponseCallback
        end
        self.pac:addWidget{
          name = 'image_' .. i .. '_' .. j,
          pos = pos,
          size = size,
          image = image,
          mouseHoverCallback = callback,
        }
      end
    end

    if trialType == 'motion' then
      self.correctWidget = 'image_' .. iCorrect .. '_' .. jCorrect
      self.animationFrames = {}
      local color = COLORS[mainColor]
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
      envOpts = {environment = env, screenSize = screenSize},
      episodeLengthSeconds = kwargs.episodeLengthSeconds or 150
  }
end

return factory
