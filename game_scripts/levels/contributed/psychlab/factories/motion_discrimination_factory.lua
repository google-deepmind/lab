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

--[[ This is a random dot motion discrimination task.
In each trial the agent is presented with a field of moving dots, most of which
are moving in the same direction. The agent must select this direction.
]]

local game = require 'dmlab.system.game'
local log = require 'common.log'
local helpers = require 'common.helpers'
local image = require 'dmlab.system.image'
local point_and_click = require 'factories.psychlab.point_and_click'
local psychlab_factory = require 'factories.psychlab.factory'
local psychlab_helpers = require 'factories.psychlab.helpers'
local psychlab_staircase = require 'levels.contributed.psychlab.factories.staircase'
local random = require 'common.random'
local set = require 'common.set'
local tensor = require 'dmlab.system.tensor'
local log = require 'common.log'

-- setup constant parameters of the task
local TIME_TO_FIXATE_CROSS = 1 -- in frames
local TIME_TO_FIXATE_TARGET = 1 -- in frames
local INTER_TRIAL_INTERVAL = 1 -- in frames
local SCREEN_SIZE = {width = 512, height = 512}
local ANIMATION_SIZE_AS_FRACTION_OF_SCREEN = {0.8, 0.8}
local BG_COLOR = 0
local EPISODE_LENGTH_SECONDS = 150
local TRIALS_PER_EPISODE_CAP = math.huge

local FIXATION_SIZE = 0.1
local FIXATION_COLOR = {255, 0, 0} -- RGB
local CENTER = {.5, .5}
local BUTTON_SIZE = 0.1

local FIXATION_REWARD = 0
local INCORRECT_REWARD = 0
local CORRECT_REWARD_SEQUENCE = 1

local INTERFRAME_INTERVAL = 4 -- in REAL (Labyrinth) frames

local VIDEO_LENGTH = 120 -- multiply by INTERFRAME_INTERVAL to get real frames
local PRERESPONSE_DELAY = 1

local DOT_DIAMETER = 6
local DOT_COLOR = {255, 255, 255}
local NUM_DOTS = 350
local SPEED = 28

local CANVAS_SIZE = 2048
local PRE_RENDER_MASK_SIZE = 1224
local COHERENCES = {1.0, 0.9, 0.75, 0.5, 0.35, 0.2, 0.1, 0.05}
-- TODO(jzl): Consider adding an option not to use the Gaussian mask.
local GAUSSIAN_MASK_SIGMA = 0.10  -- nice big size

-- Staircase parameters
local FRACTION_TO_PROMOTE = 0.75
local FRACTION_TO_DEMOTE = 0.5
local PROBE_PROBABILITY = 0.1

-- Setup response button arrows
local ARROWS_DIR = game:runFiles() ..
    '/baselab/game_scripts/levels/contributed/psychlab/data'
local ARROW_SIZE = .075
local RADIUS = .4

local MAX_STEPS_OFF_SCREEN = 300  -- 5 seconds

local factory = {}

function factory.createLevelApi(kwargs)
  kwargs.episodeLengthSeconds = kwargs.episodeLengthSeconds or
      EPISODE_LENGTH_SECONDS
  kwargs.trialsPerEpisodeCap = kwargs.trialsPerEpisodeCap or
      TRIALS_PER_EPISODE_CAP
  kwargs.fractionToPromote = kwargs.fractionToPromote or FRACTION_TO_PROMOTE
  kwargs.fractionToDemote = kwargs.fractionToDemote or FRACTION_TO_DEMOTE
  kwargs.probeProbability = kwargs.probeProbability or PROBE_PROBABILITY
  kwargs.fixedTestLength = kwargs.fixedTestLength or false
  kwargs.initialDifficultyLevel = kwargs.initialDifficultyLevel or 1
  kwargs.correctRewardSequence = kwargs.correctRewardSequence or
      CORRECT_REWARD_SEQUENCE
  kwargs.coherenceValues = kwargs.coherenceValues or COHERENCES
  kwargs.maxStepsOffScreen = kwargs.maxStepsOffScreen or MAX_STEPS_OFF_SCREEN

  local function xLoc(angle)
    return CENTER[1] + (RADIUS * math.cos(angle)) - (ARROW_SIZE / 2)
  end
  local function yLoc(angle)
    return 1 - (CENTER[2] + (RADIUS * math.sin(angle)) + (ARROW_SIZE / 2))
  end
  local ARROW_POS = {
      topLeft = {xLoc(3 * math.pi / 4), yLoc(3 * math.pi / 4)},
      topMiddle = {xLoc(math.pi / 2), yLoc(math.pi / 2)},
      topRight = {xLoc(math.pi / 4), yLoc(math.pi / 4)},

      middleLeft = {xLoc(math.pi), yLoc(math.pi)},
      middleRight = {xLoc(0), yLoc(0)},

      bottomLeft = {xLoc(5 * math.pi / 4), yLoc(5 * math.pi / 4)},
      bottomMiddle = {xLoc(3 * math.pi / 2), yLoc(3 * math.pi / 2)},
      bottomRight = {xLoc(7 * math.pi / 4), yLoc(7 * math.pi / 4)}
  }

  -- Class definition for motion discrimination psychlab environment.
  local env = {}
  env.__index = env

  setmetatable(env, {
      __call = function (cls, ...)
        local self = setmetatable({}, cls)
        self:_init(...)
        return self
      end
  })

  -- init is called at the start of each episode.
  function env:_init(pac, opts)
    self.screenSize = opts.screenSize
    log.info('opts passed to _init:\n' .. helpers.tostring(opts))

    self:setupImages()
    self:setupCoordinateBounds(CANVAS_SIZE, PRE_RENDER_MASK_SIZE)
    self.motionDirections = {}
    for i = 1, 8 do
      self.motionDirections[i] = (i - 1) * math.pi / 4
    end

    -- handle to the point and click api
    self.pac = pac
  end

  -- reset is called after init. It is called only once per episode.
  -- Note: the episodeId passed to this function may not be correct if the job
  -- has resumed from a checkpoint after preemption.
  function env:reset(episodeId, seed, ...)
    random:seed(seed)

    self.pac:setBackgroundColor{BG_COLOR, BG_COLOR, BG_COLOR}
    self.pac:clearWidgets()
    psychlab_helpers.addFixation(self, FIXATION_SIZE)
    self.reward = 0
    self:initAnimation(ANIMATION_SIZE_AS_FRACTION_OF_SCREEN)

    self.currentTrial = {}

    psychlab_helpers.setTrialsPerEpisodeCap(self, kwargs.trialsPerEpisodeCap)

    -- setup the adaptive staircase procedure
    self.staircase = psychlab_staircase.createStaircase1D{
        sequence = kwargs.coherenceValues,
        correctRewardSequence = kwargs.correctRewardSequence,
        fractionToPromote = kwargs.fractionToPromote,
        fractionToDemote = kwargs.fractionToDemote,
        probeProbability = kwargs.probeProbability,
        fixedTestLength = kwargs.fixedTestLength,
        initialDifficultyLevel = kwargs.initialDifficultyLevel,
    }

    -- blockId groups together all rows written during the same episode
    self.blockId = seed
    self.trialId = 1
  end

  function env:fillCircle(dest)
    -- Dest is (size, size, 3)
    local size = dest:shape()[1]
    local center = (size + 1) / 2
    local radiusSq = (size / 2) ^ 2
    for i = 1, size do
      for j = 1, size do
        if (i - center) ^ 2 + (j - center) ^ 2 < radiusSq then
          dest(i, j):val(DOT_COLOR)
        end
      end
    end
  end

  -- Creates image tensors for buttons, objects-to-track and the fixation cross.
  function env:setupImages()
    self.images = {}

    self.images.fixation = psychlab_helpers.getFixationImage(self.screenSize,
                                                             BG_COLOR,
                                                             FIXATION_COLOR,
                                                             FIXATION_SIZE)

    local h = BUTTON_SIZE * self.screenSize.height
    local w = BUTTON_SIZE * self.screenSize.width

    self.images.greenImage = tensor.ByteTensor(h, w, 3):fill{100, 255, 100}
    self.images.redImage = tensor.ByteTensor(h, w, 3):fill{255, 100, 100}
    self.images.blackImage = tensor.ByteTensor(h, w, 3)

    self.arrowIDs = {3, 2, 1, 6, 4, 9, 8, 7}
    self.arrowPositions = {
        ARROW_POS.topLeft,
        ARROW_POS.topMiddle,
        ARROW_POS.topRight,
        ARROW_POS.middleLeft,
        ARROW_POS.middleRight,
        ARROW_POS.bottomLeft,
        ARROW_POS.bottomMiddle,
        ARROW_POS.bottomRight
    }
    -- map orientations from 0 to 2pi to arrowIds
    self.mapAngleIdsToArrows = {4, 1, 2, 3, 6, 9, 8, 7}
    self.arrowSize = {
        height = ARROW_SIZE * self.screenSize.height,
        width = ARROW_SIZE * self.screenSize.width,
    }

    self.images.arrows = {}
    for _, i in pairs(self.arrowIDs) do
      local arrow = image.load(ARROWS_DIR .. '/arrow_' .. i .. '.png')
      arrow = image.scale(arrow, self.arrowSize.height, self.arrowSize.width)
      local arrowRGB = tensor.ByteTensor(
          self.arrowSize.height, self.arrowSize.width, 3)
      arrowRGB:select(3, 1):copy(arrow)
      arrowRGB:select(3, 2):copy(arrow)
      arrowRGB:select(3, 3):copy(arrow)
      self.images.arrows[i] = arrowRGB
    end
  end

  function env:setupCoordinateBounds(canvasSize, preRenderMaskSize)
    local preRenderLowerBound = math.floor((canvasSize - preRenderMaskSize) / 2)
    local preRenderUpperBound = canvasSize - preRenderLowerBound
    local function isAboveLowerBound(coord)
      return coord > preRenderLowerBound
    end
    local function isBelowUpperBound(coord)
      return coord < preRenderUpperBound
    end
    self.isInBounds = function (coord)
      return isAboveLowerBound(coord) and isBelowUpperBound(coord)
    end
  end

  function env:initAnimation(sizeAsFractionOfScreen)
    local imageHeight = sizeAsFractionOfScreen[1] * self.screenSize.height
    local imageWidth = sizeAsFractionOfScreen[2] * self.screenSize.width
    self.animation = {
        currentFrame = tensor.ByteTensor(imageHeight, imageWidth, 3):fill(0),
        nextFrame = tensor.ByteTensor(imageHeight, imageWidth, 3):fill(0),
        imageSize = imageHeight,  -- Assume height and width are the same
    }
    local sigma = GAUSSIAN_MASK_SIGMA
    -- Make gaussian by hand
    local gaussian = tensor.Tensor(imageHeight, imageWidth)
    local cx = 0.5 * imageWidth + 0.5
    local cy = 0.5 * imageHeight + 0.5
    gaussian:applyIndexed(function(_, index)
        local y, x = unpack(index)
        return math.exp(-math.pow((x - cx) / (sigma * imageWidth), 2) / 2
                        -math.pow((y - cy) / (sigma * imageHeight), 2) / 2)
    end)
    self.gaussianMask = gaussian
  end

  function env:finishTrial(delay)
    self.currentTrial.blockId = self.blockId
    self.currentTrial.reactionTime =
        game:episodeTimeSeconds() - self._currentTrialStartTime
    self.staircase:step(self.currentTrial.correct == 1)

    -- Convert radians to degrees before logging
    self.currentTrial.motionDirection =
      math.floor(self.currentTrial.motionDirection * (180 / math.pi) + 0.5)
    psychlab_helpers.publishTrialData(self.currentTrial, kwargs.schema)
    psychlab_helpers.finishTrialCommon(self, delay, FIXATION_SIZE)
  end

  function env:fixationCallback(name, mousePos, hoverTime, userData)
    if hoverTime == TIME_TO_FIXATE_CROSS then
      self.pac:addReward(FIXATION_REWARD)
      self.pac:removeWidget('fixation')
      self.pac:removeWidget('center_of_fixation')

      -- Sample all trial-unique data
      self.currentTrial.difficultyLevel = self.staircase:getDifficultyLevel()
      self.currentTrial.coherence = self.staircase:parameter()
      local angleId
      self.currentTrial.motionDirection, angleId = psychlab_helpers.randomFrom(
        self.motionDirections)
      self.currentTrial.correctArrow = self.mapAngleIdsToArrows[angleId]
      self.currentTrial.reward = 0

      self.pac:addTimer{
          name = 'preresponse_delay',
          timeout = PRERESPONSE_DELAY,
          callback = function(...)
            return self.addArrows(self, self.currentTrial.correctArrow) end
      }

      self.currentTrial.trialId = self.trialId
      self.trialId = self.trialId + 1

      self:displayDotMotion()
    end
  end

  function env:correctResponseCallback(name, mousePos, hoverTime, userData)
    if hoverTime == TIME_TO_FIXATE_TARGET then
      self.currentTrial.response = name
      self.currentTrial.correct = 1
      self.currentTrial.reward = self.staircase:correctReward()
      self.pac:addReward(self.currentTrial.reward)
      self:finishTrial(INTER_TRIAL_INTERVAL)
    end
  end

  function env:incorrectResponseCallback(name, mousePos, hoverTime, userData)
    if hoverTime == TIME_TO_FIXATE_TARGET then
      self.currentTrial.response = name
      self.currentTrial.correct = 0
      self.currentTrial.reward = INCORRECT_REWARD
      self.pac:addReward(self.currentTrial.reward)
      self:finishTrial(INTER_TRIAL_INTERVAL)
    end
  end

  function env:addArrows(correctID)
    -- Once the arrows appear the agent can break fixation with no penalty.
    self.pac:removeWidget('hold_fixation')

    for k, i in pairs(self.arrowIDs) do
      local arrow = self.images.arrows[i]

      local callback
      if i == correctID then
        callback = self.correctResponseCallback
      else
        callback = self.incorrectResponseCallback
      end

      self.pac:addWidget{
          name = i,
          image = arrow,
          pos = self.arrowPositions[k],
          size = {ARROW_SIZE, ARROW_SIZE},
          mouseHoverCallback = callback,
          imageLayer = 3,
      }
    end
  end

  function env:transformToFrameCoords(coords)
    self._frameCoords:val({
        math.floor((coords(1):val() / CANVAS_SIZE) * self._frameSize[1] + 0.5),
        math.floor((coords(2):val() / CANVAS_SIZE) * self._frameSize[2] + 0.5)
    })
    return self._frameCoords
  end

  function env:renderFrame(coords)
    -- coords is a tensor of size [numObjects, 2] describing the coordinates of
    -- each object in the next frame to be displayed after the current frame.
    local frame = tensor.Tensor(unpack(self.animation.nextFrame:shape()))
        :fill(BG_COLOR)

    -- Setup stored size and frameCoords buffer the first time this is called.
    if not self._frameSize then
      self._frameCoords = coords(1):clone()
      self._frameSize = frame:shape()
    end

    for i = 1, coords:shape()[1] do
      if self.isInBounds(coords(i, 1):val()) and
         self.isInBounds(coords(i, 2):val()) then
        -- Transform from canvas coordinate system to frame coordinate system
        self._frameCoords = self:transformToFrameCoords(coords(i))
        local cx, cy = self._frameCoords(2):val(), self._frameCoords(1):val()
        local dest = frame:narrow(1, cy - DOT_DIAMETER / 2, DOT_DIAMETER)
                          :narrow(2, cx - DOT_DIAMETER / 2, DOT_DIAMETER)
        self:fillCircle(dest)
      end
    end

    -- Apply gaussian aperture to R, G, B channels separately
    local gaussianMask = self.gaussianMask

    frame:select(3, 1):cmul(gaussianMask)
    frame:select(3, 2):cmul(gaussianMask)
    frame:select(3, 3):cmul(gaussianMask)
    return frame:byte()
  end

  function env:displayFrame(videoCoords, index)
    -- show the current frame
    self.pac:updateWidget('main_image', self.animation.currentFrame)
    -- recursively call this function after the interframe interval
    self.pac:addTimer{
        name = 'interframe_interval',
        timeout = INTERFRAME_INTERVAL,
        callback = function(...) self:displayFrame() end
    }
    self.animation.transition() -- updates state
    -- render the next frame
    self.animation.nextFrame = self:renderFrame(self.animation.state)
    -- update the reference called currentFrame to point to the next tensor
    self.animation.currentFrame = self.animation.nextFrame
  end

  function env:generateMotionData(motionDirection)
    local positions = tensor.Tensor(NUM_DOTS, 2)
    local randomCoordFn = function(_)
      return random:uniformInt(0, CANVAS_SIZE)
    end
    positions:apply(randomCoordFn)
    local displacement = tensor.Tensor{
        (-1) * SPEED * math.sin(motionDirection),
        SPEED * math.cos(motionDirection),
    }
    local numCoherentDots = math.floor(self.currentTrial.coherence * NUM_DOTS)
    local numIncoherentDots = NUM_DOTS - numCoherentDots
    local transition = function()
      for dot = 1, numCoherentDots do
        local position = positions(dot)
        position:cadd(displacement)
        position:apply(function(val)
          return val % CANVAS_SIZE
        end)
      end
      if numIncoherentDots > 0 then
        positions:narrow(1, numCoherentDots + 1, numIncoherentDots)
            :apply(randomCoordFn)
      end
    end
    return positions, transition
  end

  function env:displayDotMotion()
    -- Measure reaction time from start of dot motion
    self._currentTrialStartTime = game:episodeTimeSeconds()
    self.currentTrial.stepCount = 0

    self.animation.state, self.animation.transition = self:generateMotionData(
        self.currentTrial.motionDirection)

    -- Create the widget and display the first frame
    local upperLeftPosition = psychlab_helpers.getUpperLeftFromCenter(
        CENTER,
        ANIMATION_SIZE_AS_FRACTION_OF_SCREEN[1]
    )
    self.animation.currentFrame = self:renderFrame(self.animation.state)
    self.pac:addWidget{
        name = 'main_image',
        image = self.animation.currentFrame,
        pos = upperLeftPosition,
        size = ANIMATION_SIZE_AS_FRACTION_OF_SCREEN,
        imageLayer = 2,
    }
    self:displayFrame() -- starts animation
  end

  -- Remove the test array
  function env:removeArray()
    -- remove the image and response buttons
    self.pac:removeWidget('main_image')
    -- remove the arrows
    for _, i in pairs(self.arrowIDs) do
      self.pac:removeWidget(i)
    end
    self.pac:clearTimers()
  end

  -- Increment counter to allow measurement of reaction times in steps
  -- This function is automatically called at each tick.
  function env:step(lookingAtScreen)
    if self.currentTrial.stepCount ~= nil then
      self.currentTrial.stepCount = self.currentTrial.stepCount + 1
    end
  end

  return psychlab_factory.createLevelApi{
      env = point_and_click,
      envOpts = {environment = env, screenSize = SCREEN_SIZE,
                 maxStepsOffScreen = kwargs.maxStepsOffScreen},
      episodeLengthSeconds = kwargs.episodeLengthSeconds
  }
end

return factory
