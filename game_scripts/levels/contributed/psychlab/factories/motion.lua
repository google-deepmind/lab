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

local random = require 'common.random'
local tensor = require 'dmlab.system.tensor'

local motion = {}

--[[ Create an animation of balls moving around randomly.

The balls maintain a minimum distance from one another and from the walls. They
should never overlap.

Keyword Arguments:

*   `screenSize`:  {width = (int), height = (int)} in absolute coordinates
*   `allowableDistanceToWall`: (int) in absolute coordinates
*   `minDistanceBetweenBallsSq`: (number) in absolute coordinates (squared)
*   `sizeAsFractionOfScreen`: {(number), (number) size as fraction of screen
]]
function motion.createRandomBallMotion(opt)

  local motionData = {
      screenSize = {width = opt.screenSize.width,
                    height = opt.screenSize.height},
      _allowableDistanceToWall = opt.allowableDistanceToWall,
      _minDistanceBetweenBallsSq = opt.minDistanceBetweenBallsSq,
  }

  -- Preinitialize tensors to be used for rendering an animation.
  local imageWidth = opt.sizeAsFractionOfScreen[1] * opt.screenSize.width
  local imageHeight = opt.sizeAsFractionOfScreen[2] * opt.screenSize.height
  motionData.animation = {
      currentFrame = tensor.ByteTensor(imageHeight, imageWidth, 3):fill(0),
      nextFrame = tensor.ByteTensor(imageHeight, imageWidth, 3):fill(0),
      imageSize = imageHeight,  -- Assume height and width are the same
  }

  function motionData:ballNearEdge(tentativePosition)
    local maxDist = self.animation.imageSize - self._allowableDistanceToWall
    for ii = 1, tentativePosition:shape()[1] do
      local val = tentativePosition(ii):val()
      if val <= self._allowableDistanceToWall or val >= maxDist then
        return true
      end
    end
    return false
  end

  local function ballNearBall(ballA, ballB)
    return ballA:clone():csub(ballB):lengthSquared() <
        motionData._minDistanceBetweenBallsSq
  end

  function motionData:illegal(tentativePosition, nextPositions)
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

  local function generateRandomPosition(positionTensor, rangeFactor,
                                        excludeStartingNearCenter)
    if excludeStartingNearCenter then
      local exclusionFractionWidth = math.sqrt(
          motionData._minDistanceBetweenBallsSq) / motionData.screenSize.width
      local exclusionFractionHeight = math.sqrt(
          motionData._minDistanceBetweenBallsSq) / motionData.screenSize.height
      -- Equiprobably place left or right of center
      local xLowerBound, xUpperBound
      if random:uniformReal(0, 1) > 0.5 then
        xLowerBound = 0
        xUpperBound = 0.5 - exclusionFractionWidth
      else
        xLowerBound = 0.5 + exclusionFractionWidth
        xUpperBound = 1.0 - exclusionFractionWidth
      end
      -- equiprobably place above or below center
      local yLowerBound, yUpperBound
      if random:uniformReal(0, 1) > 0.5 then
        yLowerBound = 0
        yUpperBound = 0.5 - exclusionFractionHeight
      else
        yLowerBound = 0.5 + exclusionFractionHeight
        yUpperBound = 1.0 - exclusionFractionHeight
      end
      positionTensor(1):val(random:uniformReal(xLowerBound, xUpperBound))
      positionTensor(2):val(random:uniformReal(yLowerBound, yUpperBound))
    else
      positionTensor(1):val(random:uniformReal(0, 1))
      positionTensor(2):val(random:uniformReal(0, 1))
    end
    positionTensor:mul(rangeFactor):add(motionData._allowableDistanceToWall):
      round()
    return positionTensor
  end

  function motionData:generateInitialConditions(videoLength, numBalls,
                                                excludeStartingNearCenter)
    -- Frame, ball, position
    local positions = tensor.DoubleTensor(videoLength, numBalls, 2)
    -- Directions are angles in [-pi, pi].
    -- Balls begin moving in different directions from one another.
    local deltaDir = 2 * math.pi / numBalls
    local directions = tensor.DoubleTensor{
        range = {deltaDir, 2 * math.pi, deltaDir}}
    assert(directions:shape()[1] == numBalls)

    -- Tensor big enough to hold directions per ball per frame.  Initialize
    -- first frame directions only.  generateVideoData sets remaining values.
    local allDirections = tensor.DoubleTensor(videoLength,
                                              numBalls)
    allDirections:select(1, 1):copy(directions)

    local rangeFactor = self.animation.imageSize -
                        2 * self._allowableDistanceToWall
    local tentativePosition = tensor.DoubleTensor(2)
    for i = 1, numBalls do
      generateRandomPosition(tentativePosition, rangeFactor,
                             excludeStartingNearCenter)
      if i > 1 then
        -- Ensure that no positions are too close to one another
        for attempt = 1, 10000 do
         if self:illegal(tentativePosition, positions(1):
                         narrow(1, 1, i - 1)) then
           generateRandomPosition(tentativePosition, rangeFactor,
                                  excludeStartingNearCenter)
         else
           break
         end
        end
      end
      positions(1, i):copy(tentativePosition)
    end
    return positions, allDirections
  end

  function motionData:nextFrameFromPrevious(positions, directions, motionSpeed)
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
          directions(i):val(random:uniformReal(-math.pi, math.pi))
        tentativePosition = getNewPosition(positions(i), directions(i))
      end
      -- for all balls after first, must check not near edge or other balls.
      if i > 1 then
        local safeguardCounter = 1
        local illegal = self:illegal(tentativePosition,
                                     nextPositions:narrow(1, 1, i - 1))
        while illegal do
            directions(i):val(random:uniformReal(-math.pi, math.pi))
          tentativePosition = getNewPosition(positions(i), directions(i))
          safeguardCounter = safeguardCounter + 1
          if safeguardCounter == 500 then
            tentativePosition = oldPositions(i)
            if type(illegal) == 'table' then
              for _, ii in ipairs(illegal) do
                -- stop all balls in collision and give each a new direction to
                -- try to move in on the next frame
                nextPositions(ii):copy(oldPositions(ii))
                directions(ii):val(random:uniformReal(-math.pi, math.pi))
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

  function motionData:generateVideoData(videoLength, numBalls, motionSpeed,
                                        excludeStartingNearCenter)
    local excludeStartingNearCenter = excludeStartingNearCenter or false
    local positions, directions = self:generateInitialConditions(
        videoLength,
        numBalls,
        excludeStartingNearCenter
    )
    for i = 2, videoLength do
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

  return motionData
end

return motion
