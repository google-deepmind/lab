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

local game = require 'dmlab.system.game'
local log = require 'common.log'
local psychlab_factory = require 'factories.psychlab.factory'
local psychlab_helpers = require 'factories.psychlab.helpers'
local helpers = require 'common.helpers'
local image = require 'dmlab.system.image'
local point_and_click = require 'factories.psychlab.point_and_click'
local random = require 'common.random'
local tensor = require 'dmlab.system.tensor'
local defaults = require 'levels.contributed.psychlab.visuospatial_suite.factories.defaults'

--[[ A visual match task.

The subject is presented with a small number (2, 3, or 4) of pairs of shapes.
In one pair, the shapes are the same, in the others, they are different.
The subject must indicate which pair are the same using a button.

The intention of the task is that in order to solve it, the agent needs
to look closely at the shapes. Hence it is a task that requires gaze control,
but not by directly rewarding it.
]]


local SHAPE_SIZES = {5, 6, 7, 8, 9}


local factory = {}

function factory.createLevelApi(kwargs)
  kwargs.episodeLengthSeconds = kwargs.episodeLengthSeconds or
      defaults.EPISODE_LENGTH_SECONDS
  kwargs.trialsPerEpisodeCap = kwargs.trialsPerEpisodeCap or
      defaults.TRIALS_PER_EPISODE_CAP
  kwargs.bgColor = kwargs.bgColor or defaults.BG_COLOR
  kwargs.colors = kwargs.colors or defaults.COLORS
  kwargs.shapeSizes = kwargs.shapeSizes or SHAPE_SIZES
  kwargs.incorrectReward = kwargs.incorrectReward or defaults.INCORRECT_REWARD
  kwargs.correctReward = kwargs.correctReward or defaults.CORRECT_REWARD
  kwargs.maxStepsOffScreen = kwargs.maxStepsOffScreen or
      defaults.MAX_STEPS_OFF_SCREEN

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
    psychlab_helpers.addFixation(self, defaults.FIXATION_SIZE)

    self.currentTrial = {}

    psychlab_helpers.setTrialsPerEpisodeCap(self, kwargs.trialsPerEpisodeCap)

    -- blockId groups together all rows written during the same episode
    self.blockId = random:uniformInt(1, 2 ^ 32)
    self.trialId = 1
  end

  -- Creates image tensor for fixation.
  function env:setupImages()
    self.images = {}

    self.images.fixation = psychlab_helpers.getFixationImage(
      self.screenSize, kwargs.bgColor, defaults.FIXATION_COLOR,
      defaults.FIXATION_SIZE)

    local h = defaults.BUTTON_SIZE * self.screenSize.height
    local w = defaults.BUTTON_SIZE * self.screenSize.width
    self.images.greenImage = tensor.ByteTensor(h, w, 3):fill{100, 255, 100}
    self.images.redImage = tensor.ByteTensor(h, w, 3):fill{255, 100, 100}
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
      self.pac:removeWidget('fixation')
      self.pac:removeWidget('center_of_fixation')
      self.currentTrial.reward = 0
      self.currentTrial.trialId = self.trialId
      self.trialId = self.trialId + 1
      self:addArray()
      self.pac:addTimer{
          name = 'trial_timeout',
          timeout = defaults.TRIAL_TIMEOUT,
          callback = function(...) return self:trialTimeoutCallback() end
      }
      -- Measure reaction time since the trial started.
      self._currentTrialStartTime = game:episodeTimeSeconds()
      self.pac:resetSteps()
    end
  end

  function env:onHoverEndCorrect(name, mousePos, hoverTime, userData)
    -- Reward if this is the first "hoverEnd" event for this trial.
    self.currentTrial.response = name
    self.currentTrial.correct = 1
    self.currentTrial.reward = kwargs.correctReward
    self.pac:addReward(self.currentTrial.reward)
    self:finishTrial(defaults.FAST_INTER_TRIAL_INTERVAL)
  end

  function env:onHoverEndIncorrect(name, mousePos, hoverTime, userData)
    -- Reward if this is the first "hoverEnd" event for this trial.
    self.currentTrial.response = name
    self.currentTrial.correct = 0
    self.currentTrial.reward = kwargs.incorrectReward
    self.pac:addReward(self.currentTrial.reward)
    self:finishTrial(defaults.FAST_INTER_TRIAL_INTERVAL)
  end

  function env:correctResponseCallback(name, mousePos, hoverTime, userData)
    self.pac:updateWidget(name, self.images.greenImage)
  end

  function env:incorrectResponseCallback(name, mousePos, hoverTime, userData)
    self.pac:updateWidget(name, self.images.redImage)
  end

  function env:trialTimeoutCallback()
    -- Trial times out if agent does not fixate on odd one out.
    self.currentTrial.correct = 0
    self.currentTrial.reward = kwargs.incorrectReward
    self.pac:addReward(self.currentTrial.reward)
    self:finishTrial(defaults.FAST_INTER_TRIAL_INTERVAL)
  end

  function env:addArray()
    local numPairs = psychlab_helpers.randomFrom{2, 3, 4}
    local shapeSize = psychlab_helpers.randomFrom(kwargs.shapeSizes)
    local setSize = 2 * numPairs

    local objectHeight = defaults.SCREEN_SIZE.height * 0.125
    local objectWidth = objectHeight

    self.currentTrial.numPairs = numPairs
    self.currentTrial.shapeSize = shapeSize

    local size = {objectWidth / defaults.SCREEN_SIZE.width,
                  objectHeight / defaults.SCREEN_SIZE.height}

    local shapes = self:createShapes(setSize, shapeSize, 0.75)

    local radius = 0.5 -- random:uniformReal(0.5, 0.75)
    local radiansOffset = -2 * math.pi / setSize / 2
    local shuffleGen = random:shuffledIndexGenerator(setSize - 2)
    local correctPair = random:uniformInt(1, numPairs)
    for i = 1, setSize do
      local pair = math.ceil(i / 2)
      local radians = radiansOffset + 2 * math.pi * i / setSize
      local pos = {
          0.5 * (1 + radius * math.sin(radians)),
          0.5 * (1 - radius * math.cos(radians))
      }
      pos = psychlab_helpers.getUpperLeftFromCenter(pos, size)

      local j
      if pair == correctPair then
        j = setSize - 1
      else
        j = shuffleGen()
      end
      local shape = shapes[j]
      local color = kwargs.colors[pair]
      local image = self:createImage(
          shape, color, objectHeight, objectWidth)
      self.pac:addWidget{
          name = 'image_' .. i,
          pos = pos,
          size = size,
          image = image,
      }
    end

    -- Add buttons
    local h = defaults.BUTTON_SIZE * self.screenSize.height
    local w = defaults.BUTTON_SIZE * self.screenSize.width
    for i = 1, numPairs do
      local color = kwargs.colors[i]
      local buttonImage = tensor.ByteTensor(h, w, 3):fill(color)
      local buttonPosX = 0.5 +
                         (0.5 * numPairs - i) * defaults.BUTTON_SIZE * 1.5 +
                         defaults.BUTTON_SIZE * 0.25
      local responseCallback, hoverEndCallback
      if i == correctPair then
        responseCallback = self.correctResponseCallback
        hoverEndCallback = self.onHoverEndCorrect
      else
        responseCallback = self.incorrectResponseCallback
        hoverEndCallback = self.onHoverEndIncorrect
      end
      self.pac:addWidget{
          name = 'button_' .. tostring(i),
          image = buttonImage,
          pos = {buttonPosX, 1 - defaults.BUTTON_SIZE},
          size = {defaults.BUTTON_SIZE, defaults.BUTTON_SIZE},
          mouseHoverCallback = responseCallback,
          mouseHoverEndCallback = hoverEndCallback,
      }
    end
  end

  function env:removeArray()
    self.pac:clearWidgets()
  end

  function env:createShapes(numShapes, shapeSize, p)
    local shapes = {}
    for n = 1, numShapes do
      local grid = {}
      for i = 1, shapeSize do
        local row = {}
        for j = 1, shapeSize do
          local value = random:uniformReal(0, 1) < p and 1 or 0
          table.insert(row, value)
        end
        table.insert(grid, row)
      end
      table.insert(shapes, grid)
    end
    return shapes
  end

  function env:createImage(shape, color, objectHeight, objectWidth)
    local shapeSize = #shape
    local unscaled = tensor.ByteTensor(shapeSize, shapeSize, 3)
    for k = 1, 3 do
      local plane = unscaled:select(3, k)
      plane:val(shape)
      plane:mul(color[k])
    end
    local scaled = image.scale(
        unscaled, objectHeight, objectWidth, 'nearest')
    return scaled
  end

  local screenSize = defaults.SCREEN_SIZE
  return psychlab_factory.createLevelApi{
      env = point_and_click,
      envOpts = {environment = env, screenSize = screenSize,
                 maxStepsOffScreen = kwargs.maxStepsOffScreen},
      episodeLengthSeconds = kwargs.episodeLengthSeconds
  }
end

return factory
