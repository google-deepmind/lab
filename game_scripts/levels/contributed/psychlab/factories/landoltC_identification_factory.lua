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

--[[ Landolt C identification task.
In each trial the agent must choose the orientation of a landolt C optotype.
The optotypes are presented at a range of scales and contrasts.
]]

local game = require 'dmlab.system.game'
local log = require 'common.log'
local helpers = require 'common.helpers'
local image = require 'dmlab.system.image'
local point_and_click = require 'factories.psychlab.point_and_click'
local psychlab_factory = require 'factories.psychlab.factory'
local psychlab_helpers = require 'factories.psychlab.helpers'
local random = require 'common.random'
local set = require 'common.set'
local tensor = require 'dmlab.system.tensor'

-- arrows and landolt C in same directory for now, but may differ in future
local ARROWS_DIR = game:runFiles() ..
    '/baselab/game_scripts/levels/contributed/psychlab/data'
local LANDOLT_C_DIR = game:runFiles() ..
    '/baselab/game_scripts/levels/contributed/psychlab/data'

-- setup constant parameters of the task
local TIME_TO_FIXATE_CROSS = 1 -- in frames
local TIME_TO_FIXATE_TARGET = 1 -- in frames
local FAST_INTERTRIAL_INTERVAL = 1 -- in frames
local SLOW_INTERTRIAL_INTERVAL = 1 -- in frames
local SCREEN_SIZE = {width = 512, height = 512}
local BG_COLOR = 0

-- Set the maximum stimulus size in [0, 1] coordinates and its fractions to test
local MAX_SIZE = .47

local TEST_SCALES = {1, 1 / 2, 1 / 4, 1 / 8, 1 / 16, 1 / 32}
local TEST_CONTRASTS = {1, 1 / 2, 1 / 4, 1 / 8, 1 / 16, 1 / 32}

local FIXATION_REWARD = 0
local CORRECT_REWARD = 1
local INCORRECT_REWARD = 0

-- local ARROW_SIZE = .1 -- this works fine
local ARROW_SIZE = .075 -- switched to this for jitter experiments and future
local FIXATION_SIZE = .1
local FIXATION_COLOR = {255, 0, 0} -- RGB
local CENTER = {.5, .5}
local RADIUS = .4

-- Staircase parameters
local PROBE_PROBABILITY = 0.1
local FRACTION_TO_ADVANCE = 0.75
local FRACTION_TO_REMAIN = 0.5

local MAX_STEPS_OFF_SCREEN = 300  -- 5 seconds

-- for target jittering experiments, this is the max x,y perturbation
local MAX_JITTER = .015

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

--[[ Scale the image contrast. Assumes source is a white figure on a black
background
* `source` image to modify
* `contrast` (0 < contrast < 1), if 1, then has no effect
]]
local function applyContrast(source, contrast)
  return source:div(math.floor(1 / contrast))
end

local factory = {}

function factory.createLevelApi(kwargs)
  -- Currently there are no configurable parameters for this environment,
  -- so kwargs are ignored.

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

  --[[ Function to define a two-dimensional adaptive staircase procedure
  (a 'class'). This procedure promotes from difficulty level (K, L) by testing
  num_to_test trials at level (K + 1, L) and (K, L + 1), both interleaved, also
  interleave (K, L). If successful (> fractionToAdvance * num_to_test correct)
  then increment either K or L or both. If < fractionToRemain * num_to_test
  correct on the (K, L) tests, then demote to (K - 1, L -1). In addition, there
  are also 'probe' trials. The probe trials may show any stimuli
  from (1:K, 1:L).

  At level (K, L), the number of trials to test is mean(K, L).
  ]]
  local function initStaircase(opt)
    assert(type(opt.probeProbability) == 'number')
    assert(type(opt.fractionToAdvance) == 'number')
    assert(type(opt.fractionToRemain) == 'number')
    assert(opt.contrastRatios ~= nil)
    assert(opt.scaleRatios ~= nil)

    local staircase = {
        difficultyLevel = {contrast = 1, scale = 1},
        numTrialsPerTest = 1,
        probeProbability = opt.probeProbability,
        fractionToAdvance = opt.fractionToAdvance,
        fractionToRemain = opt.fractionToRemain,
        contrastRatios = opt.contrastRatios,
        scaleRatios = opt.scaleRatios,
        _testTrialTypes = {'base', 'advance_contrast', 'advance_scale'},
    }

    function staircase._resetTestDomain(self)
      -- Set the number of trials before changing level to floor((K + L)/2)
      self.numTrialsPerTest = math.floor(
          (self.difficultyLevel.contrast + self.difficultyLevel.scale) / 2)

      local advance_contrast_level, advance_scale_level
      if self.difficultyLevel.contrast < self.contrastRatios:size() then
        advance_contrast_level = self.difficultyLevel.contrast + 1
      else
        advance_contrast_level = self.difficultyLevel.contrast
      end
      if self.difficultyLevel.scale < self.scaleRatios:size() then
        advance_scale_level = self.difficultyLevel.scale + 1
      else
        advance_scale_level = self.difficultyLevel.scale
      end
      self._testDomain = {
          base = self.difficultyLevel,
          advance_contrast = {contrast = advance_contrast_level,
                              scale = self.difficultyLevel.scale},
          advance_scale = {contrast = self.difficultyLevel.contrast,
                           scale = advance_scale_level}
      }
      local randperm = random:shuffle({1, 2, 3})
      self._testOrder = tensor.Tensor(self.numTrialsPerTest, 3)
          :fill(randperm)
          :reshape{self.numTrialsPerTest * 3}
      self._testIndex = 1
      self._scores = {base = 0, advance_contrast = 0, advance_scale = 0}
    end

    function staircase._updateLevel(self)
      local scoreToAdvance = self.fractionToAdvance * self.numTrialsPerTest
      local scoreToRemain = self.fractionToRemain * self.numTrialsPerTest
      if self._scores['advance_contrast'] >= scoreToAdvance then
        if self.difficultyLevel.contrast < self.contrastRatios:size() then
          self.difficultyLevel.contrast = self.difficultyLevel.contrast + 1
        end
      end
      if self._scores['advance_scale'] >= scoreToAdvance then
        if self.difficultyLevel.scale < self.scaleRatios:size() then
          self.difficultyLevel.scale = self.difficultyLevel.scale + 1
        end
      end
      if self._scores['base'] < scoreToRemain then
        if self.difficultyLevel.contrast > 1 then
          self.difficultyLevel.contrast = self.difficultyLevel.contrast - 1
        end
        if self.difficultyLevel.scale > 1 then
          self.difficultyLevel.scale = self.difficultyLevel.scale - 1
        end
      end
    end

    function staircase._getNextTestTrial(self)
      local trialType = self._testTrialTypes[
          self._testOrder(self._testIndex):val()]
      local contrast = self.contrastRatios(
          self._testDomain[trialType].contrast):val()
      local scale = self.scaleRatios(self._testDomain[trialType].scale):val()
      self._testIndex = self._testIndex + 1
      return {contrast = contrast, scale = scale, trialType = trialType}
    end

    function staircase._getNextProbeTrial(self)
      local contrast = self.contrastRatios:narrow(
          1, 1, self.difficultyLevel.contrast
      )(random:uniformInt(1, self.difficultyLevel.contrast)):val()
      local scale = self.scaleRatios:narrow(
          1, 1, self.difficultyLevel.scale)(
          random:uniformInt(1, self.difficultyLevel.scale)):val()
      return {contrast = contrast, scale = scale, trialType = 'probe'}
    end

    function staircase.getNextTrial(self)
      if random:uniformReal(0, 1) < self.probeProbability then
        return self:_getNextProbeTrial()
      else
        return self:_getNextTestTrial()
      end
    end

    -- 'staircase.step' is called at the end of each trial.
    function staircase.step(self, trialType, correct)
      if trialType ~= 'probe' then
        self._scores[trialType] = self._scores[trialType] + correct
      end
      if self._testIndex > self._testOrder:size() then
        self:_updateLevel()
        self:_resetTestDomain()
      end
    end

    staircase:_resetTestDomain()
    return staircase
  end

  -- init gets called at the start of each episode
  function env:_init(pac, opts)
    self.screenSize = opts.screenSize
    log.info('opts passed to _init:\n' .. helpers.tostring(opts))

    -- use the screenSize to compute the actual size in pixels for each image
    self.sizeInPixels = {
        arrowHeight = ARROW_SIZE * self.screenSize.height,
        arrowWidth = ARROW_SIZE * self.screenSize.width,
        fixationHeight = FIXATION_SIZE * self.screenSize.height,
        fixationWidth = FIXATION_SIZE * self.screenSize.width
    }

    self.arrowIDs = {7, 8, 9, 4, 6, 1, 2, 3}
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

    -- if self.jitter then randomly perturb the target location on each trial
    self.jitter = nil
    if self.jitter then
      log.info('Jitter target location')
      self._jitteredCenter = {}
    end

    self.scaleRatios = TEST_SCALES
    self.contrastRatios = TEST_CONTRASTS

    -- load and preprocess the images once when the experiment starts
    self:setupImages()
    -- handle to the point and click api
    self.pac = pac

    self.pac:clearWidgets()
    psychlab_helpers.addFixation(self, FIXATION_SIZE)

    self.reward = 0

    self.currentTrial = {}

    self.staircase = initStaircase{
        probeProbability = PROBE_PROBABILITY,
        fractionToAdvance = FRACTION_TO_ADVANCE,
        fractionToRemain = FRACTION_TO_REMAIN,
        contrastRatios = tensor.Tensor(TEST_CONTRASTS),
        scaleRatios = tensor.Tensor(TEST_SCALES)
    }
  end

  -- reset is called after init. It is called only once per episode.
  -- Note: the episodeId passed to this function may not be correct if the job
  -- has resumed from a checkpoint after preemption.
  function env:reset(episodeId, seed, ...)
    random:seed(seed + episodeId)
    -- blockId groups together all rows written during the same episode
    self.blockId = seed  -- currently unused?
  end

  function env:setupImages()
    self.images = {}
    self.images.arrows = {}
    self.images.landoltCs = {}
    self.images.fixation = psychlab_helpers.getFixationImage(
        self.screenSize, BG_COLOR, FIXATION_COLOR, FIXATION_SIZE)
    for _, i in pairs(self.arrowIDs) do
      local arrow = image.load(ARROWS_DIR .. '/arrow_' .. i .. '.png')
      arrow = image.scale(
          arrow, self.sizeInPixels.arrowHeight, self.sizeInPixels.arrowWidth)
      local arrowRGB = tensor.ByteTensor(
          self.sizeInPixels.arrowHeight, self.sizeInPixels.arrowWidth, 3)
      arrowRGB:select(3, 1):copy(arrow)
      arrowRGB:select(3, 2):copy(arrow)
      arrowRGB:select(3, 3):copy(arrow)
      self.images.arrows[i] = arrowRGB

      local landoltC = image.load(LANDOLT_C_DIR .. '/landoltC_' .. i .. '.png')
      self.images.landoltCs[i] = landoltC
    end
  end

  function env:finishTrial(delay)
    self.currentTrial.blockId = self.blockId
    self.currentTrial.reactionTime =
        game:episodeTimeSeconds() - self._currentTrialStartTime
    self.staircase:step(self.currentTrial.trialType, self.currentTrial.correct)

    psychlab_helpers.publishTrialData(self.currentTrial, kwargs.schema)
    psychlab_helpers.finishTrialCommon(self, delay, FIXATION_SIZE)
  end

  function env:fixationCallback(name, mousePos, hoverTime, userData)
    if hoverTime == TIME_TO_FIXATE_CROSS then
      self.pac:addReward(FIXATION_REWARD)
      self.pac:removeWidget('fixation')
      self.pac:removeWidget('center_of_fixation')
      self:addArray()

      -- Measure reaction time from trial initiation
      self._currentTrialStartTime = game:episodeTimeSeconds()
      self.currentTrial.stepCount = 0
    end
  end

  function env:correctResponseCallback(name, mousePos, hoverTime, userData)
    if hoverTime == TIME_TO_FIXATE_TARGET then
      self.currentTrial.response = name
      self.currentTrial.correct = 1

      self.pac:addReward(CORRECT_REWARD)
      self:finishTrial(FAST_INTERTRIAL_INTERVAL)
    end
  end

  function env:incorrectResponseCallback(name, mousePos, hoverTime, userData)
    if hoverTime == TIME_TO_FIXATE_TARGET then
      self.currentTrial.response = name
      self.currentTrial.correct = 0

      self.pac:addReward(INCORRECT_REWARD)
      self:finishTrial(SLOW_INTERTRIAL_INTERVAL)
    end
  end

  function env:formatScale(factor)
    local zeroOne = factor * MAX_SIZE
    return {
        factor = factor,
        zeroOne = zeroOne,
        pixels = {width = zeroOne * self.screenSize.width,
                  height = zeroOne * self.screenSize.height}
    }
  end

  local function getJitter(maxJitter)
    return random:uniformReal(-maxJitter, maxJitter)
  end

  function env:_randomShift(center)
    self._jitteredCenter[1] = center[1] + getJitter(MAX_JITTER)
    self._jitteredCenter[2] = center[2] + getJitter(MAX_JITTER)
    return self._jitteredCenter
  end

  function env:getTargetCenter(center)
    if self.jitter then
      return self:_randomShift(center)
    else
      return center
    end
  end

  function env:addLandoltC(contrast)
    self.currentTrial.targetId = psychlab_helpers.randomFrom(self.arrowIDs)
    local landoltC = self.images.landoltCs[self.currentTrial.targetId]

    self.pac:addWidget{
        name = 'target',
        image = applyContrast(
            image.scale(landoltC,
                        self.currentTrial.scalePixelsFormat.height,
                        self.currentTrial.scalePixelsFormat.width),
            contrast),
        pos = psychlab_helpers.getUpperLeftFromCenter(
            self:getTargetCenter(CENTER), self.currentTrial.scaleZeroOneFormat),
        size = {self.currentTrial.scaleZeroOneFormat,
                self.currentTrial.scaleZeroOneFormat},
    }

    return self.currentTrial.targetId
  end

  function env:addArrows(correctID)
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
      }
    end
  end

  function env:addArray()
    local trialData = self.staircase:getNextTrial()
    local scaleFormats = self:formatScale(trialData.scale)
    self.currentTrial.scaleFactor = scaleFormats.factor
    self.currentTrial.scaleZeroOneFormat = scaleFormats.zeroOne
    self.currentTrial.scalePixelsFormat = scaleFormats.pixels
    self.currentTrial.contrast = trialData.contrast
    self.currentTrial.trialType = trialData.trialType
    local correctID = self:addLandoltC(self.currentTrial.contrast)
    self:addArrows(correctID)
  end

  function env:removeArray()
    -- remove the landoltC
    self.pac:removeWidget('target')

    -- remove the arrows
    for _, i in pairs(self.arrowIDs) do
      self.pac:removeWidget(i)
    end
  end

  function env:step(lookingAtScreen)
    if self.currentTrial.stepCount ~= nil then
      self.currentTrial.stepCount = self.currentTrial.stepCount + 1
    end
  end

  return psychlab_factory.createLevelApi{
      env = point_and_click,
      envOpts = {environment = env, screenSize = SCREEN_SIZE,
                 maxStepsOffScreen = MAX_STEPS_OFF_SCREEN},
      episodeLengthSeconds = 150
  }
end

return factory
