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

--[[ This is a multi-object tracking task.
As in e.g.
Pylyshyn, Z.W. and R.W. Storm, Spatial Vision, 1988. 3(3): p. 1-19.
]]

local game = require 'dmlab.system.game'
local log = require 'common.log'
local helpers = require 'common.helpers'
local point_and_click = require 'factories.psychlab.point_and_click'
local psychlab_helpers = require 'factories.psychlab.helpers'
local psychlab_factory = require 'factories.psychlab.factory'
local psychlab_staircase = require 'levels.contributed.psychlab.factories.staircase'
local random = require 'common.random'
local tensor = require 'dmlab.system.tensor'


-- setup constant parameters of the task
local TIME_TO_FIXATE_CROSS = 1 -- in frames
local FAST_INTERTRIAL_INTERVAL = 1 -- in frames
local SCREEN_SIZE = {width = 512, height = 512}
local ANIMATION_SIZE_AS_FRACTION_OF_SCREEN = {0.75, 0.75}
local BG_COLOR = 255

local FIXATION_SIZE = 0.1
local FIXATION_COLOR = {255, 0, 0} -- RGB
local CENTER = {.5, .5}
local BUTTON_SIZE = 0.1

local FIXATION_REWARD = 0
local CORRECT_REWARD = 1
local INCORRECT_REWARD = 0

local MAX_IDLE_STEPS = 400

local STUDY_INTERVAL = 60

local INTERFRAME_INTERVAL = 4 -- in REAL (Labyrinth) frames

local VIDEO_LENGTH = 60 -- multiply by INTERFRAME_INTERVAL to get real frames
local NUM_TO_TRACK_SEQUENCE = {1, 1, 2, 3, 4, 5, 6, 7}
local MOTION_SPEEDS = {6, 9, 12, 15, 18}
local BALL_DIAMETER = 40
local ALLOWABLE_DISTANCE_TO_WALL = 21
local MIN_DISTANCE_BETWEEN_BALLS = 61
local BALL_TEST_COLOR = {0, 0, 0} -- 0
local BALL_STUDY_COLOR = {0, 200, 0} -- 127
local BALL_RESPONSE_COLOR = {0, 0, 200} -- 100

local TRIALS_PER_EPISODE_CAP = 100

-- Staircase parameters
local PROBE_PROBABILITY = 0.1
local FRACTION_TO_PROMOTE = 1.0
local FRACTION_TO_DEMOTE = 0.5

local factory = {}

function factory.createLevelApi(kwargs)
  kwargs.timeToFixateCross = kwargs.timeToFixateCross or TIME_TO_FIXATE_CROSS
  kwargs.fastIntertrialInterval = kwargs.fastIntertrialInterval or
    FAST_INTERTRIAL_INTERVAL
  kwargs.screenSize = kwargs.screenSize or SCREEN_SIZE
  kwargs.animationSizeAsFractionOfScreen =
    kwargs.animationSizeAsFractionOfScreen or
    ANIMATION_SIZE_AS_FRACTION_OF_SCREEN
  kwargs.bgColor = kwargs.bgColor or BG_COLOR
  kwargs.fixationSize = kwargs.fixationSize or FIXATION_SIZE
  kwargs.fixationColor = kwargs.fixationColor or FIXATION_COLOR
  kwargs.center = kwargs.center or CENTER
  kwargs.buttonSize = kwargs.buttonSize or BUTTON_SIZE
  kwargs.fixationReward = kwargs.fixationReward or FIXATION_REWARD
  kwargs.correctReward = kwargs.correctReward or CORRECT_REWARD
  kwargs.incorrectReward = kwargs.incorrectReward or INCORRECT_REWARD
  kwargs.maxIdleSteps = kwargs.maxIdleSteps or MAX_IDLE_STEPS
  kwargs.studyInterval = kwargs.studyInterval or STUDY_INTERVAL
  kwargs.interframeInterval = kwargs.interframeInterval or INTERFRAME_INTERVAL
  kwargs.videoLength = kwargs.videoLength or VIDEO_LENGTH
  kwargs.numToTrackSequence = kwargs.numToTrackSequence or NUM_TO_TRACK_SEQUENCE
  kwargs.motionSpeeds = kwargs.motionSpeeds or MOTION_SPEEDS
  kwargs.ballDiameter = kwargs.ballDiameter or BALL_DIAMETER
  kwargs.ballRadius = kwargs.ballDiameter / 2
  kwargs.allowableDistanceToWall = kwargs.allowableDistanceToWall or
    ALLOWABLE_DISTANCE_TO_WALL
  kwargs.minDistanceBetweenBalls = kwargs.minDistanceBetweenBalls or
    MIN_DISTANCE_BETWEEN_BALLS
  kwargs.minDistanceBetweenBallsSq = kwargs.minDistanceBetweenBalls ^ 2
  kwargs.ballTestColor = kwargs.ballTestColor or BALL_TEST_COLOR
  kwargs.ballStudyColor = kwargs.ballStudyColor or BALL_STUDY_COLOR
  kwargs.ballResponseColor = kwargs.ballResponseColor or BALL_RESPONSE_COLOR
  kwargs.trialsPerEpisodeCap = kwargs.trialsPerEpisodeCap or
    TRIALS_PER_EPISODE_CAP

  -- Class definition for multiobject_tracking psychlab environment.
  local env = {}
  env.__index = env

  setmetatable(env, {
    __call = function (cls, ...)
      local self = setmetatable({}, cls)
      self:_init(...)
      return self
    end
  })

  local MAX_INT = math.pow(2, 32) - 1

  -- init gets called at the start of each episode
  function env:_init(pac, opts)
    log.info('opts passed to _init:\n' .. helpers.tostring(opts))

    self.screenSize = opts.screenSize

    -- setup images
    self:setupImages()
    self._timeoutIfIdle = true
    self._stepsSinceInteraction = 0

    -- handle to the point and click api
    self.pac = pac

    self.staircase = psychlab_staircase.createStaircase1D{
        sequence = kwargs.numToTrackSequence,
        fractionToPromote = FRACTION_TO_PROMOTE,
        fractionToDemote = FRACTION_TO_DEMOTE,
        probeProbability = PROBE_PROBABILITY,
    }
  end

  -- reset is called after init. It is called only once per episode.
  -- Note: the episodeId passed to this function may not be correct if the job
  -- has resumed from a checkpoint after preemption.
  function env:reset(episodeId, seed)
    random:seed(seed)

    self.pac:setBackgroundColor{kwargs.bgColor, kwargs.bgColor, kwargs.bgColor}
    self.pac:clearWidgets()
    psychlab_helpers.addFixation(self, kwargs.fixationSize)

    self:initAnimation(kwargs.animationSizeAsFractionOfScreen)
    self.currentTrial = {}

    psychlab_helpers.setTrialsPerEpisodeCap(self, kwargs.trialsPerEpisodeCap)

    -- blockId groups together all rows written during the same episode
    self.blockId = random:uniformInt(0, MAX_INT)
  end

  function env:drawCircle(size, ballColor)
    local radius = math.floor(size / 2)
    local radiusSquared = radius ^ 2
    local ballTensor = tensor.ByteTensor(size, size, 3):fill(kwargs.bgColor)
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

  -- Creates image tensors for buttons, objects-to-track and the fixation cross.
  function env:setupImages()
    self.images = {}

    self.images.fixation = psychlab_helpers.getFixationImage(self.screenSize,
      kwargs.bgColor, kwargs.fixationColor, kwargs.fixationSize)
    local h = kwargs.buttonSize * self.screenSize.height
    local w = kwargs.buttonSize * self.screenSize.width

    self.images.greenImage = tensor.ByteTensor(h, w, 3):fill{100, 255, 100}
    self.images.redImage = tensor.ByteTensor(h, w, 3):fill{255, 100, 100}
    self.images.blackImage = tensor.ByteTensor(h, w, 3)

    self.images.testCircle = self:drawCircle(kwargs.ballDiameter,
                                             kwargs.ballTestColor)
    self.images.studyCircle = self:drawCircle(kwargs.ballDiameter,
                                              kwargs.ballStudyColor)
    self.images.responseCircle = self:drawCircle(kwargs.ballDiameter,
                                                 kwargs.ballResponseColor)
  end

  function env:initAnimation(sizeAsFractionOfScreen)
    local imageHeight = sizeAsFractionOfScreen[1] * self.screenSize.height
    local imageWidth = sizeAsFractionOfScreen[2] * self.screenSize.width
    self.animation = {
        currentFrame = tensor.ByteTensor(imageHeight, imageWidth, 3):fill(0),
        nextFrame = tensor.ByteTensor(imageHeight, imageWidth, 3):fill(0),
        imageSize = imageHeight,  -- Assume height and width are the same
    }
  end

  function env:finishTrial(delay)
    self._stepsSinceInteraction = 0
    self.currentTrial.blockId = self.blockId
    self.currentTrial.reactionTime =
        game:episodeTimeSeconds() - self._currentTrialStartTime

    self.staircase:step(self.currentTrial.correct == 1)

    psychlab_helpers.publishTrialData(self.currentTrial, kwargs.schema)
    psychlab_helpers.finishTrialCommon(self, delay, kwargs.fixationSize)
  end

  function env:fixationCallback(name, mousePos, hoverTime, userData)
    if hoverTime == kwargs.timeToFixateCross then
      self._stepsSinceInteraction = 0
      self.pac:addReward(kwargs.fixationReward)
      self.pac:removeWidget('fixation')
      self.pac:removeWidget('center_of_fixation')

      -- Measure reaction time from trial initiation
      self._currentTrialStartTime = game:episodeTimeSeconds()
      self.currentTrial.stepCount = 0

      -- go to the study phase
      self:studyPhase()
    end
  end

  function env:onHoverEndCorrect(name, mousePos, hoverTime, userData)
    -- Reward if this is the first "hoverEnd" event for this trial.
    self.currentTrial.response = name
    self.currentTrial.correct = 1
    self.pac:addReward(kwargs.correctReward)
    self:finishTrial(kwargs.fastIntertrialInterval)
  end

  function env:onHoverEndIncorrect(name, mousePos, hoverTime, userData)
    -- Reward if this is the first "hoverEnd" event for this trial.
    self.currentTrial.response = name
    self.currentTrial.correct = 0
    self.pac:addReward(kwargs.incorrectReward)
    self:finishTrial(kwargs.fastIntertrialInterval)
  end

  function env:correctResponseCallback(name, mousePos, hoverTime, userData)
    self.pac:updateWidget(name, self.images.greenImage)
  end

  function env:incorrectResponseCallback(name, mousePos, hoverTime, userData)
    self.pac:updateWidget(name, self.images.redImage)
  end

  function env:addResponseButtons(queryIsTarget)
    local buttonPosX = 0.5 - kwargs.buttonSize * 1.5
    local buttonSize = {kwargs.buttonSize, kwargs.buttonSize}

    local callbackQueryIsDistractor, callbackQueryIsTarget
    local hoverEndQueryIsDistractor, hoverEndQueryIsTarget
    if queryIsTarget then
      -- correct response is on the right
      callbackQueryIsTarget = self.correctResponseCallback
      hoverEndQueryIsTarget = self.onHoverEndCorrect
      callbackQueryIsDistractor = self.incorrectResponseCallback
      hoverEndQueryIsDistractor = self.onHoverEndIncorrect
    else
      -- correct response is on the left
      callbackQueryIsTarget = self.incorrectResponseCallback
      hoverEndQueryIsTarget = self.onHoverEndIncorrect
      callbackQueryIsDistractor = self.correctResponseCallback
      hoverEndQueryIsDistractor = self.onHoverEndCorrect
    end

    self.pac:addWidget{
        name = 'respond_right',
        image = self.images.blackImage,
        pos = {1 - buttonPosX - kwargs.buttonSize, 1 - kwargs.buttonSize},
        size = buttonSize,
        mouseHoverCallback = callbackQueryIsTarget,
        mouseHoverEndCallback = hoverEndQueryIsTarget,
    }
    self.pac:addWidget{
        name = 'respond_left',
        image = self.images.blackImage,
        pos = {buttonPosX, 1 - kwargs.buttonSize},
        size = buttonSize,
        mouseHoverCallback = callbackQueryIsDistractor,
        mouseHoverEndCallback = hoverEndQueryIsDistractor,
    }
  end

  function env:renderFrame(coords, indicesToColor, specialCircle)
    -- coords is a tensor of size [numObjects, 2] describing the coordinates of
    -- each object in the next frame to be displayed after the current frame.
    local indicesToColor = indicesToColor or {}
    local frame = tensor.ByteTensor(unpack(self.animation.nextFrame:shape())):
        fill(kwargs.bgColor)

    for i = 1, coords:shape()[1] do
      local circle = indicesToColor[i] and specialCircle or
        self.images.testCircle
      local topLeft = coords(i):clone():round():sub(kwargs.ballRadius):val()
      frame:
          narrow(1, topLeft[1], kwargs.ballDiameter):
          narrow(2, topLeft[2], kwargs.ballDiameter):
          copy(circle)
    end
    return frame
  end

  function env:displayFrame(videoCoords, index)
    if index < videoCoords:shape()[1] then
      index = index + 1
      -- show the current frame
      self.pac:updateWidget('main_image', self.animation.currentFrame)
      -- recursively call this function after the interframe interval
      self.pac:addTimer{
          name = 'interframe_interval',
          timeout = kwargs.interframeInterval,
          callback = function(...) return self.displayFrame(self,
                                                            videoCoords,
                                                            index) end
      }
      -- render the next frame
      self.animation.nextFrame = self:renderFrame(videoCoords(index))
      -- update the reference called currentFrame to point to the next tensor
      self.animation.currentFrame = self.animation.nextFrame
    end
  end

  function env:displayAnimation(videoCoords)
    -- videoCoords is a tensor of size [framesPerVideo, numOjects, 2] describing
    -- the location of each object on every frame.
    -- displayFrame calls itself recursively until all frames have been shown
    local index = 0
    self:displayFrame(videoCoords, index)
  end

  function env:ballNearEdge(tentativePosition)
    local maxDist = self.animation.imageSize - kwargs.allowableDistanceToWall
    for ii = 1, tentativePosition:shape()[1] do
      local val = tentativePosition(ii):val()
      if val <= kwargs.allowableDistanceToWall or val >= maxDist then
        return true
      end
    end
    return false
  end

  local function ballNearBall(ballA, ballB)
    local distanceSq = 0
    local temp = ballA:clone():csub(ballB)
    temp:cmul(temp)
    temp:apply(function (v) distanceSq = distanceSq + v end)
    return distanceSq < kwargs.minDistanceBetweenBallsSq
  end

  function env:illegal(tentativePosition, nextPositions)
    local ballsTooClose = {}
    if self:ballNearEdge(tentativePosition) then
      return true
    end
    for i = 1, nextPositions:shape()[1] do
      if ballNearBall(tentativePosition, nextPositions(i)) then
        table.insert(ballsTooClose, i)
      end
    end
    return #ballsTooClose > 0 and ballsTooClose or false
  end

  local function generateRandomPosition(positionTensor, rangeFactor)
      positionTensor(1):val(random:uniformReal(0, 1))
      positionTensor(2):val(random:uniformReal(0, 1))
      positionTensor:mul(rangeFactor):add(kwargs.allowableDistanceToWall):
        round()
      return positionTensor
  end

  function env:generateInitialConditions(numBalls)
    -- Frame, ball, position
    local positions = tensor.DoubleTensor(kwargs.videoLength, numBalls, 2)
    -- Directions are angles in [-pi, pi].
    -- Balls begin moving in different directions from one another.
    local deltaDir = 2 * math.pi / numBalls
    local directions = tensor.DoubleTensor{
        range = {deltaDir, 2 * math.pi, deltaDir}}
    assert(directions:shape()[1] == numBalls)

    -- Tensor big enough to hold directions per ball per frame.  Initialize
    -- first frame directions only.  generateVideoData sets remaining values.
    local allDirections = tensor.DoubleTensor(kwargs.videoLength,
                                              numBalls)
    allDirections:select(1, 1):copy(directions)

    local rangeFactor = self.animation.imageSize -
                        2 * kwargs.allowableDistanceToWall
    local tentativePosition = tensor.DoubleTensor(2)
    for i = 1, numBalls do
      generateRandomPosition(tentativePosition, rangeFactor)
      if i > 1 then
        -- Ensure that no positions are too close to one another
        while self:illegal(tentativePosition, positions(1):
            narrow(1, 1, i - 1)) do
          generateRandomPosition(tentativePosition, rangeFactor)
        end
      end
      positions(1, i):copy(tentativePosition)
    end
    return positions, allDirections
  end

  function env:nextFrameFromPrevious(positions, directions, motionSpeed)
    local oldPositions = positions:clone()
    local function getNewPosition(position, direction)
      direction = direction:val()
      local displacement = tensor.DoubleTensor{
          motionSpeed * math.cos(direction),
          motionSpeed * math.sin(direction)
      }
      return position:clone():cadd(displacement)
    end
    -- Compute the next position of each ball by following its current
    -- direction.
    local nextPositions = positions:clone()
    for i = 1, positions:shape()[1] do
      -- only need to check not near edge for first ball
      local tentativePosition = getNewPosition(positions(i), directions(i))
      while self:ballNearEdge(tentativePosition) do
        directions(i):val(random:uniformReal(0, 1) * 2 * math.pi - math.pi)
        tentativePosition = getNewPosition(positions(i), directions(i))
      end
      -- for all balls after first, must check not near edge or other balls.
      if i > 1 then
        local safeguardCounter = 1
        local illegal = self:illegal(tentativePosition,
                                     nextPositions:narrow(1, 1, i - 1))
        while illegal do
          --directions[i] = torch.rand(1) * 2 * math.pi - math.pi
          directions(i):val(random:uniformReal(0, 1) * 2 * math.pi - math.pi)
          tentativePosition = getNewPosition(positions(i), directions(i))
          safeguardCounter = safeguardCounter + 1
          if safeguardCounter == 500 then
            tentativePosition = oldPositions(i)
            if type(illegal) == 'table' then
              for _, ii in ipairs(illegal) do
                -- stop all balls in collision and give each a new direction to
                -- try to move in on the next frame
                nextPositions(ii):copy(oldPositions(ii))
                directions(ii):val(random:uniformReal(0, 1) * 2 * math.pi -
                                   math.pi)
              end
            end
            break
          end
        end
      end
      nextPositions(i):copy(tentativePosition)
    end
    return nextPositions, directions
  end

  function env:generateVideoData(numBalls, motionSpeed)
    local positions, directions = self:generateInitialConditions(numBalls)
    for i = 2, kwargs.videoLength do
      local nextPositions, nextDirections = self:nextFrameFromPrevious(
          positions(i - 1),
          directions(i - 1),
          motionSpeed
      )
      positions(i):copy(nextPositions)
      directions(i):copy(nextDirections)
    end
    return positions, directions
  end


  -- Permutes the sequence p, p + 1, ..., q.
  local function permute(p, q)
    return tensor.Int64Tensor{range = {p, q}}:shuffle(random:generator())
  end


  function env:studyPhase()
    self._stepsSinceInteraction = 0
    self.currentTrial.numToTrack = self.staircase:parameter()
    self.currentTrial.numBalls = self.currentTrial.numToTrack * 2
    self.currentTrial.motionSpeed = psychlab_helpers.randomFrom(
        kwargs.motionSpeeds)
    local indicesToTrackTensor = permute(1, self.currentTrial.numBalls)
    local indicesToTrack = {}
    self.currentTrial.indicesToTrack = {}
    for i = 1, self.currentTrial.numToTrack do
      local j = indicesToTrackTensor(i):val()
      indicesToTrack[j] = true
      table.insert(self.currentTrial.indicesToTrack, j)
    end
    self.currentTrial.indicesNotToTrack = {}
    for i = self.currentTrial.numToTrack + 1, self.currentTrial.numBalls do
      local j = indicesToTrackTensor(i):val()
      table.insert(self.currentTrial.indicesNotToTrack, j)
    end
    self.currentTrial.queryIsTarget = psychlab_helpers.randomFrom{true, false}
    local positions, _ = self:generateVideoData(self.currentTrial.numBalls,
                                                self.currentTrial.motionSpeed)
    -- create the widget and display the first frame
    local upperLeftPosition = psychlab_helpers.getUpperLeftFromCenter(
        kwargs.center,
        kwargs.animationSizeAsFractionOfScreen[1]
    )
    -- Display the first frame for the duration of the study interval
    self.animation.currentFrame = self:renderFrame(
        positions(1),
        indicesToTrack,
        self.images.studyCircle
    )
    self.pac:addWidget{
        name = 'main_image',
        image = self.animation.currentFrame,
        pos = upperLeftPosition,
        size = kwargs.animationSizeAsFractionOfScreen,
    }
    self.pac:updateWidget('main_image', self.animation.currentFrame)
    self.pac:addTimer{
        name = 'study_interval',
        timeout = kwargs.studyInterval,
        callback = function(...) return self.trackingPhase(self,
                                                           positions) end
    }
  end

  function env:trackingPhase(positions)
    self:displayAnimation(positions)
    self.pac:addTimer{
        name = 'tracking_interval',
        timeout = kwargs.videoLength * kwargs.interframeInterval,
        callback = function(...) return self.responsePhase(self,
                                                           positions) end
    }
  end

  function env:responsePhase(positions)
    self._stepsSinceInteraction = 0
    -- Display the last frame of the animation for the entire response phase.
    local queryIndex = {}
    if self.currentTrial.queryIsTarget then
      -- Since the balls are randomly ordered it is fine to always use the
      -- first.
      queryIndex[self.currentTrial.indicesToTrack[1]] = true
    else
      queryIndex[self.currentTrial.indicesNotToTrack[1]] = true
    end
    self.animation.currentFrame = self:renderFrame(
        positions(positions:shape()[1]),
        queryIndex,
        self.images.responseCircle
    )
    self.pac:updateWidget('main_image', self.animation.currentFrame)
    self:addResponseButtons(self.currentTrial.queryIsTarget)
  end

  -- Remove the test array
  function env:removeArray()
    -- remove the image and response buttons
    self.pac:removeWidget('main_image')
    self.pac:removeWidget('respond_right')
    self.pac:removeWidget('respond_left')
    self.pac:clearTimers()
  end

  -- Increment counter to allow measurement of reaction times in steps
  -- This function is automatically called at each tick.
  function env:step(lookingAtScreen)
    if self.currentTrial.stepCount == nil then self:fixationCallback() end

    if self.currentTrial.stepCount ~= nil then
      self.currentTrial.stepCount = self.currentTrial.stepCount + 1
    end

    -- If too long since interaction with any buttons, then end episode. This
    -- should speed up the early stages of training, since it causes less time
    -- to be spent looking away from the screen.
    self._stepsSinceInteraction = self._stepsSinceInteraction + 1
    if self._timeoutIfIdle and
        self._stepsSinceInteraction > kwargs.maxIdleSteps then
      self.pac:endEpisode()
    end
  end

  return psychlab_factory.createLevelApi{
      env = point_and_click,
      envOpts = {environment = env, screenSize = kwargs.screenSize},
      episodeLengthSeconds = 180
  }
end

return factory
