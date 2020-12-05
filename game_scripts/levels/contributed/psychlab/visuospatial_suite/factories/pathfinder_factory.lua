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

--[[ This is a pathfinder task.

The agent's task is to decide whether the two white circles are joined
by a continuous path or not.

The implementation here is based on the description in Linsley et al,
Learning long-range spatial dependencies with horizontal gated-recurrent units
https://arxiv.org/pdf/1805.08315.pdf
although some parameters have been modified for the psychlab environment.
]]


local FG_COLOR = {255, 255, 255}

local LINE_LENGTHS = {3, 6, 9, 12, 15}

local SEGMENT_RADIUS = 10
local SEGMENT_RADIUS_SQUARED = SEGMENT_RADIUS * SEGMENT_RADIUS
local SEGMENT_DIAMETER_SQUARED = 4 * SEGMENT_RADIUS_SQUARED
local PADDLE_MARGIN = 2
local PADDLE_RADIUS = SEGMENT_RADIUS - PADDLE_MARGIN
local PADDLE_RADIUS_SQUARED = PADDLE_RADIUS * PADDLE_RADIUS
local PADDLE_THICKNESS = 3
local MAX_DELTA_THETA = 0.5
local NUM_PADDLES = 100  -- Total number of paddles in a challenge image


local factory = {}

function factory.createLevelApi(kwargs)
  kwargs.episodeLengthSeconds = kwargs.episodeLengthSeconds or
      defaults.EPISODE_LENGTH_SECONDS
  kwargs.trialsPerEpisodeCap = kwargs.trialsPerEpisodeCap or
      defaults.TRIALS_PER_EPISODE_CAP
  kwargs.timeToFixateCross = kwargs.timeToFixateCross or
      defaults.TIME_TO_FIXATE_CROSS
  kwargs.fastInterTrialInterval = kwargs.fastInterTrialInterval or
      defaults.FAST_INTER_TRIAL_INTERVAL
  kwargs.screenSize = kwargs.screenSize or defaults.SCREEN_SIZE
  kwargs.buttonSize = kwargs.buttonSize or defaults.BUTTON_SIZE
  kwargs.lineLengths = kwargs.lineLengths or LINE_LENGTHS
  kwargs.fixationSize = kwargs.fixationSize or defaults.FIXATION_SIZE
  kwargs.fixationColor = kwargs.fixationColor or defaults.FIXATION_COLOR
  kwargs.fixationReward = kwargs.fixationReward or defaults.FIXATION_REWARD
  kwargs.correctReward = kwargs.correctReward or defaults.CORRECT_REWARD
  kwargs.incorrectReward = kwargs.incorrectReward or defaults.INCORRECT_REWARD
  kwargs.maxStepsOffScreen = kwargs.maxStepsOffScreen or
      defaults.MAX_STEPS_OFF_SCREEN

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

    self.screenSize = opts.screenSize

    -- Use 'screenSize' to compute the actual size in pixels for each image.
    self.sizeInPixels = {
        fixationHeight = kwargs.fixationSize * self.screenSize.height,
        fixationWidth = kwargs.fixationSize * self.screenSize.width
    }

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

    self.pac:setBackgroundColor(defaults.BG_COLOR)
    self.pac:clearWidgets()
    psychlab_helpers.addFixation(self, kwargs.fixationSize)

    self.currentTrial = {}

    psychlab_helpers.setTrialsPerEpisodeCap(self, kwargs.trialsPerEpisodeCap)

    -- blockId groups together all rows written during the same episode
    self.blockId = random:uniformInt(1, 2 ^ 32)
    self.trialId = 1
  end

  -- Creates image Tensors for red/green/white/black buttons and fixation.
  function env:setupImages()
    self.images = {}

    self.images.fixation = psychlab_helpers.getFixationImage(
      self.screenSize, defaults.BG_COLOR, kwargs.fixationColor,
      kwargs.fixationSize)

    local h = kwargs.buttonSize * self.screenSize.height
    local w = kwargs.buttonSize * self.screenSize.width

    self.images.greenImage = tensor.ByteTensor(h, w, 3):fill(
        defaults.GREEN_BUTTON_COLOR)
    self.images.redImage = tensor.ByteTensor(h, w, 3):fill(
        defaults.RED_BUTTON_COLOR)
    self.images.whiteImage = tensor.ByteTensor(h, w, 3):fill(255)
    self.images.blackImage = tensor.ByteTensor(h, w, 3)

    self.canvasHeight = math.floor(
        (1 - 2 * kwargs.buttonSize) * self.screenSize.height)
    self.canvasWidth = math.floor(
        (1 - 2 * kwargs.buttonSize) * self.screenSize.width)
  end

  function env:finishTrial(delay)
    self.currentTrial.blockId = self.blockId
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
    self.currentTrial.reward = kwargs.correctReward
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

  function env:addResponseButtons(connected)
    local buttonPosX = 0.5 - kwargs.buttonSize * 1.5
    local buttonSize = {kwargs.buttonSize, kwargs.buttonSize}

    local connectedCallback, notConnectedCallback
    local hoverEndConnected, hoverEndNotConnected
    if connected then
      connectedCallback = self.correctResponseCallback
      hoverEndConnected = self.onHoverEndCorrect
      notConnectedCallback = self.incorrectResponseCallback
      hoverEndNotConnected = self.onHoverEndIncorrect
    else
      connectedCallback = self.incorrectResponseCallback
      hoverEndConnected = self.onHoverEndIncorrect
      notConnectedCallback = self.correctResponseCallback
      hoverEndNotConnected = self.onHoverEndCorrect
    end

    self.pac:addWidget{
        name = 'notConnected',
        image = self.images.whiteImage,
        pos = {buttonPosX, 1 - kwargs.buttonSize},
        size = buttonSize,
        mouseHoverCallback = notConnectedCallback,
        mouseHoverEndCallback = hoverEndNotConnected,
    }
    self.pac:addWidget{
        name = 'connected',
        image = self.images.whiteImage,
        pos = {1 - buttonPosX - kwargs.buttonSize, 1 - kwargs.buttonSize},
        size = buttonSize,
        mouseHoverCallback = connectedCallback,
        mouseHoverEndCallback = hoverEndConnected,
    }
  end

  function env:resetCollisionDetection()
    self._committedSegments = {}
    self._uncommittedSegments = {}
  end

  function env:addSegment(segment)
    table.insert(self._uncommittedSegments, segment)
  end

  function env:commitSegments()
    for _, segment in ipairs(self._uncommittedSegments) do
      table.insert(self._committedSegments, segment)
    end
    self._uncommittedSegments = {}
  end

  function env:rollbackSegments()
    self._uncommittedSegments = {}
  end

  function env:detectCollision(newSegment)
    if newSegment.cx <= SEGMENT_RADIUS or newSegment.cy <= SEGMENT_RADIUS
        or newSegment.cx >= self.canvasWidth - SEGMENT_RADIUS
        or newSegment.cy >= self.canvasHeight - SEGMENT_RADIUS then
      return true
    end
    for _, segment in ipairs(self._committedSegments) do
      local dx = newSegment.cx - segment.cx
      local dy = newSegment.cy - segment.cy
      if dx * dx + dy * dy < SEGMENT_DIAMETER_SQUARED then
        return true
      end
    end
    for _, segment in ipairs(self._uncommittedSegments) do
      local dx = newSegment.cx - segment.cx
      local dy = newSegment.cy - segment.cy
      if dx * dx + dy * dy < SEGMENT_DIAMETER_SQUARED then
        return true
      end
    end
    return false
  end

  function env:beginSegment()
    local cx = random:uniformInt(
        1 + SEGMENT_RADIUS, self.canvasWidth - SEGMENT_RADIUS)
    local cy = random:uniformInt(
        1 + SEGMENT_RADIUS, self.canvasHeight - SEGMENT_RADIUS)
    local theta = random:uniformReal(0, math.pi)
    local segment = {cx = cx, cy = cy, theta = theta}
    if not self:detectCollision(segment) then
      self:addSegment(segment)
      return segment
    end
  end

  function env:nextSegment(segment)
    local deltaTheta = random:uniformReal(-MAX_DELTA_THETA, MAX_DELTA_THETA)
    local theta = segment.theta + deltaTheta
    local midTheta = (segment.theta + theta) / 2
    local cx = math.floor(
        segment.cx + 2 * (SEGMENT_RADIUS + 1) * math.cos(midTheta))
    local cy = math.floor(
        segment.cy + 2 * (SEGMENT_RADIUS + 1) * math.sin(midTheta))
    local segment = {cx = cx, cy = cy, theta = theta}
    if not self:detectCollision(segment) then
      self:addSegment(segment)
      return segment
    end
  end

  function env:drawCircle(segment)
    local cx, cy = segment.cx, segment.cy
    for i = -SEGMENT_RADIUS, SEGMENT_RADIUS do
      for j = -SEGMENT_RADIUS, SEGMENT_RADIUS do
        if i * i + j * j <= SEGMENT_RADIUS_SQUARED then
          self._image(cy + i, cx + j):fill(FG_COLOR)
        end
      end
    end
  end

  function env:drawPaddle(segment)
    local cx, cy, theta = segment.cx, segment.cy, segment.theta
    local sinTheta = math.sin(theta)
    local cosTheta = math.cos(theta)
    for i = -PADDLE_RADIUS, PADDLE_RADIUS do
      for j = -PADDLE_RADIUS, PADDLE_RADIUS do
        if i * i + j * j <= PADDLE_RADIUS_SQUARED and
           math.abs(i * cosTheta - j * sinTheta) < PADDLE_THICKNESS then
          self._image(cy + i, cx + j):fill(FG_COLOR)
        end
      end
    end
  end

  function env:buildPathOrFail(length, withBegin, withEnd)
    local segments, beginSegment, endSegment = {}, nil, nil

    local radius = 10
    local segment = self:beginSegment(radius)
    if segment == nil then
      return nil
    end
    if withBegin then
      segment = self:nextSegment(segment)
      if segment == nil then
        return nil
      end
      beginSegment = segment
    end
    for i = 1, length do
      segment = self:nextSegment(segment)
      if segment == nil then
        return nil
      end
      segments[i] = segment
    end
    if withEnd then
      segment = self:nextSegment(segment)
      if segment == nil then
        return nil
      end
      endSegment = segment
    end
    return segments, beginSegment, endSegment
  end

  function env:drawPath(segments, beginSegment, endSegment)
    if beginSegment ~= nil then
      self:drawCircle(beginSegment)
    end
    for i = 1, #segments do
      self:drawPaddle(segments[i])
    end
    if endSegment ~= nil then
      self:drawCircle(endSegment)
    end
  end

  function env:buildPath(length, withBegin, withEnd)
    local segments, beginSegment, endSegment
    repeat
      self:rollbackSegments()
      segments, beginSegment, endSegment = self:buildPathOrFail(
          length, withBegin, withEnd)
    until segments ~= nil
    self:commitSegments()

    self:drawPath(segments, beginSegment, endSegment)
  end

  function env:getChallengeImage(lineLength, connected)
    if not self._image then
      self._image = tensor.ByteTensor(self.canvasHeight, self.canvasWidth, 3)
    end
    self._image:fill(defaults.BG_COLOR)

    self:resetCollisionDetection()

    if connected then
      self:buildPath(lineLength, true, true)
      self:buildPath(lineLength, false, false)
    else
      self:buildPath(lineLength, true, false)
      self:buildPath(lineLength, true, false)
    end

    local distractorLineLength = lineLength / 3
    -- Number of distractor paths is chosen so that total number of paddles
    -- is constant.
    local numDistractorPaths =
        (NUM_PADDLES - 2 * lineLength) / distractorLineLength
    for _ = 1, numDistractorPaths do
      self:buildPath(distractorLineLength, false, false)
    end

    return self._image
  end

  function env:addArray()
    self.currentTrial.lineLength = psychlab_helpers.randomFrom(
        kwargs.lineLengths)
    self.currentTrial.connected = psychlab_helpers.randomFrom{true, false}

    local challengeImage = self:getChallengeImage(
        self.currentTrial.lineLength, self.currentTrial.connected)

    -- Center the search image taking into account the buttons
    self.pac:addWidget{
        name = 'image',
        image = challengeImage,
        pos = {kwargs.buttonSize, 0.5 * kwargs.buttonSize},
        size = {1 - 2 * kwargs.buttonSize, 1 - 2 * kwargs.buttonSize},
    }

    self:addResponseButtons(self.currentTrial.connected)
  end

  function env:removeArray()
    self.pac:removeWidget('image')
    self.pac:removeWidget('connected')
    self.pac:removeWidget('notConnected')
  end

  return psychlab_factory.createLevelApi{
      env = point_and_click,
      envOpts = {environment = env, screenSize = kwargs.screenSize,
                 maxStepsOffScreen = kwargs.maxStepsOffScreen},
      episodeLengthSeconds = kwargs.episodeLengthSeconds
  }
end

return factory
