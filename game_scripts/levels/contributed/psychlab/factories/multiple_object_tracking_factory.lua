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
local psychlab_motion = require 'levels.contributed.psychlab.factories.motion'
local psychlab_staircase = require 'levels.contributed.psychlab.factories.staircase'
local random = require 'common.random'
local tensor = require 'dmlab.system.tensor'


-- setup constant parameters of the task
local TIME_TO_FIXATE_CROSS = 1 -- in frames
local FAST_INTERTRIAL_INTERVAL = 1 -- in frames
local SCREEN_SIZE = {width = 512, height = 512}
local ANIMATION_SIZE_AS_FRACTION_OF_SCREEN = {0.75, 0.75}
local BG_COLOR = {255, 255, 255}
local EPISODE_LENGTH_SECONDS = 180
local RESPONSE_TIME_CAP_FRAMES = 300 -- end trial after this many frames

local FIXATION_SIZE = 0.1
local FIXATION_COLOR = {255, 0, 0} -- RGB
local CENTER = {.5, .5}
local BUTTON_SIZE = 0.1

local FIXATION_REWARD = 0
local CORRECT_REWARD_SEQUENCE = 1
local INCORRECT_REWARD = 0

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

local MAX_STEPS_OFF_SCREEN = 300  -- 5 seconds

local factory = {}

function factory.createLevelApi(kwargs)
  kwargs.episodeLengthSeconds = kwargs.episodeLengthSeconds or
      EPISODE_LENGTH_SECONDS
  kwargs.timeToFixateCross = kwargs.timeToFixateCross or TIME_TO_FIXATE_CROSS
  kwargs.fastIntertrialInterval = kwargs.fastIntertrialInterval or
    FAST_INTERTRIAL_INTERVAL
  kwargs.screenSize = kwargs.screenSize or SCREEN_SIZE
  kwargs.responseTimeCapFrames = kwargs.responseTimeCapFrames or
    RESPONSE_TIME_CAP_FRAMES
  kwargs.animationSizeAsFractionOfScreen =
    kwargs.animationSizeAsFractionOfScreen or
    ANIMATION_SIZE_AS_FRACTION_OF_SCREEN
  kwargs.bgColor = kwargs.bgColor or BG_COLOR
  kwargs.fixationSize = kwargs.fixationSize or FIXATION_SIZE
  kwargs.fixationColor = kwargs.fixationColor or FIXATION_COLOR
  kwargs.center = kwargs.center or CENTER
  kwargs.buttonSize = kwargs.buttonSize or BUTTON_SIZE
  kwargs.fixationReward = kwargs.fixationReward or FIXATION_REWARD
  kwargs.correctRewardSequence = kwargs.correctRewardSequence or
      CORRECT_REWARD_SEQUENCE
  kwargs.incorrectReward = kwargs.incorrectReward or INCORRECT_REWARD
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
  kwargs.maxStepsOffScreen = kwargs.maxStepsOffScreen or MAX_STEPS_OFF_SCREEN
  kwargs.fractionToPromote = kwargs.fractionToPromote or FRACTION_TO_PROMOTE
  kwargs.fractionToDemote = kwargs.fractionToDemote or FRACTION_TO_DEMOTE
  kwargs.probeProbability = kwargs.probeProbability or PROBE_PROBABILITY
  kwargs.fixedTestLength = kwargs.fixedTestLength or false
  kwargs.initialDifficultyLevel = kwargs.initialDifficultyLevel or 1

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

    -- handle to the point and click api
    self.pac = pac

    self.staircase = psychlab_staircase.createStaircase1D{
        sequence = kwargs.numToTrackSequence,
        correctRewardSequence = kwargs.correctRewardSequence,
        fractionToPromote = kwargs.fractionToPromote,
        fractionToDemote = kwargs.fractionToDemote,
        probeProbability = kwargs.probeProbability,
        fixedTestLength = kwargs.fixedTestLength,
        initialDifficultyLevel = kwargs.initialDifficultyLevel,
    }

    -- initialize the motion generation procedure
    self._motion = psychlab_motion.createRandomBallMotion{
        screenSize = self.screenSize,
        allowableDistanceToWall = kwargs.allowableDistanceToWall,
        minDistanceBetweenBallsSq = kwargs.minDistanceBetweenBallsSq,
        sizeAsFractionOfScreen = kwargs.animationSizeAsFractionOfScreen
    }
  end

  -- reset is called after init. It is called only once per episode.
  -- Note: the episodeId passed to this function may not be correct if the job
  -- has resumed from a checkpoint after preemption.
  function env:reset(episodeId, seed)
    random:seed(seed)

    self.pac:setBackgroundColor(kwargs.bgColor)
    self.pac:clearWidgets()
    psychlab_helpers.addFixation(self, kwargs.fixationSize)

    self.currentTrial = {}

    psychlab_helpers.setTrialsPerEpisodeCap(self, kwargs.trialsPerEpisodeCap)

    -- blockId groups together all rows written during the same episode
    self.blockId = random:uniformInt(0, MAX_INT)
    self.trialId = 1
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

    self.images.testCircle = psychlab_helpers.makeFilledCircle(
        kwargs.ballDiameter,
        kwargs.ballTestColor,
        kwargs.bgColor
    )
    self.images.studyCircle = psychlab_helpers.makeFilledCircle(
        kwargs.ballDiameter,
        kwargs.ballStudyColor,
        kwargs.bgColor
    )
    self.images.responseCircle = psychlab_helpers.makeFilledCircle(
        kwargs.ballDiameter,
        kwargs.ballResponseColor,
        kwargs.bgColor
    )
  end

  function env:finishTrial(delay)
    self.currentTrial.blockId = self.blockId
    self.currentTrial.reactionTime =
        game:episodeTimeSeconds() - self._currentTrialStartTime

    self.staircase:step(self.currentTrial.correct == 1)

    psychlab_helpers.publishTrialData(self.currentTrial, kwargs.schema)
    psychlab_helpers.finishTrialCommon(self, delay, kwargs.fixationSize)
  end

  function env:fixationCallback(name, mousePos, hoverTime, userData)
    if hoverTime == kwargs.timeToFixateCross then
      self.pac:addReward(kwargs.fixationReward)
      self.pac:removeWidget('fixation')
      self.pac:removeWidget('center_of_fixation')

      -- Measure reaction time from trial initiation
      self._currentTrialStartTime = game:episodeTimeSeconds()
      self.currentTrial.stepCount = 0
      self.currentTrial.trialId = self.trialId
      self.trialId = self.trialId + 1

      -- go to the study phase
      self:studyPhase()
    end
  end

  function env:onHoverEndCorrect(name, mousePos, hoverTime, userData)
    -- Reward if this is the first "hoverEnd" event for this trial.
    self.currentTrial.response = name
    self.currentTrial.correct = 1
    self.currentTrial.reward = self.staircase:correctReward()
    self.pac:addReward(self.currentTrial.reward)
    self:finishTrial(kwargs.fastIntertrialInterval)
  end

  function env:onHoverEndIncorrect(name, mousePos, hoverTime, userData)
    -- Reward if this is the first "hoverEnd" event for this trial.
    self.currentTrial.response = name
    self.currentTrial.correct = 0
    self.currentTrial.reward = kwargs.incorrectReward
    self.pac:addReward(self.currentTrial.reward)
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
    local frame = tensor.ByteTensor(
      unpack(self._motion.animation.nextFrame:shape())):fill(kwargs.bgColor)

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
      self.pac:updateWidget('main_image', self._motion.animation.currentFrame)
      -- recursively call this function after the interframe interval
      self.pac:addTimer{
          name = 'interframe_interval',
          timeout = kwargs.interframeInterval,
          callback = function(...) return self.displayFrame(self,
                                                            videoCoords,
                                                            index) end
      }
      -- render the next frame
      self._motion.animation.nextFrame = self:renderFrame(videoCoords(index))
      -- update the reference called currentFrame to point to the next tensor
      self._motion.animation.currentFrame = self._motion.animation.nextFrame
    end
  end

  function env:displayAnimation(videoCoords)
    -- videoCoords is a tensor of size [framesPerVideo, numOjects, 2] describing
    -- the location of each object on every frame.
    -- displayFrame calls itself recursively until all frames have been shown
    local index = 0
    self:displayFrame(videoCoords, index)
  end

  -- Permutes the sequence p, p + 1, ..., q.
  local function permute(p, q)
    return tensor.Int64Tensor{range = {p, q}}:shuffle(random:generator())
  end

  function env:studyPhase()
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
    -- videoCoords is size [videoLength,numBalls,2] where 2 is x,y coordinates.
    local videoCoords, _ = self._motion:generateVideoData(
        kwargs.videoLength,
        self.currentTrial.numBalls,
        self.currentTrial.motionSpeed
    )
    -- create the widget and display the first frame
    local upperLeftPosition = psychlab_helpers.getUpperLeftFromCenter(
        kwargs.center,
        kwargs.animationSizeAsFractionOfScreen[1]
    )
    -- Display the first frame for the duration of the study interval
    self._motion.animation.currentFrame = self:renderFrame(
        videoCoords(1),
        indicesToTrack,
        self.images.studyCircle
    )
    self.pac:addWidget{
        name = 'main_image',
        image = self._motion.animation.currentFrame,
        pos = upperLeftPosition,
        size = kwargs.animationSizeAsFractionOfScreen,
    }
    self.pac:updateWidget('main_image', self._motion.animation.currentFrame)
    self.pac:addTimer{
        name = 'study_interval',
        timeout = kwargs.studyInterval,
        callback = function(...) return self.trackingPhase(self,
                                                           videoCoords) end
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
    -- Display the last frame of the animation for the entire response phase.
    local queryIndex = {}
    if self.currentTrial.queryIsTarget then
      -- Since the balls are randomly ordered it is fine to always use the
      -- first.
      queryIndex[self.currentTrial.indicesToTrack[1]] = true
    else
      queryIndex[self.currentTrial.indicesNotToTrack[1]] = true
    end
    self._motion.animation.currentFrame = self:renderFrame(
        positions(positions:shape()[1]),
        queryIndex,
        self.images.responseCircle
    )
    self.pac:updateWidget('main_image', self._motion.animation.currentFrame)
    self:addResponseButtons(self.currentTrial.queryIsTarget)

    self.pac:addTimer{
        name = 'trial_timeout',
        timeout = kwargs.responseTimeCapFrames,
        callback = function(...) return self:trialTimeoutCallback() end
      }
  end

  function env:trialTimeoutCallback()
    self.currentTrial.correct = 0
    self.currentTrial.reward = kwargs.incorrectReward
    self.pac:addReward(self.currentTrial.reward)
    self:finishTrial(kwargs.fastIntertrialInterval)
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
    if self.currentTrial.stepCount ~= nil then
      self.currentTrial.stepCount = self.currentTrial.stepCount + 1
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
