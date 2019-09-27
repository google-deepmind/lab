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

local log = require 'common.log'
local events = require 'dmlab.system.events'
local image = require 'dmlab.system.image'
local random = require 'common.random'
local tensor = require 'dmlab.system.tensor'

local helpers = {}

local CENTER = {0.5, 0.5}
local MAX_JITTER = 0.01

--[[ Scales the provided image and adds it as a widget to the environment in the
center of the screen.

'env' (table) Psychlab task environment class.
'targetImage' (tensor) RGB image to display on the target widget.
'targetSize' (number in (0, 1]) target size as fraction of the screen.
]]
function helpers.addTargetImage(env, targetImage, targetSize)
  assert(targetImage, 'targetImage must not be nil')
  assert(targetSize, 'targetSize must not be nil')
  assert(targetSize > 0 and targetSize <= 1, 'targetSize must in (0, 1]')

  env.target:copy(targetImage)
  local sizeInPixels = helpers.getSizeInPixels(env.screenSize, targetSize)
  local scaledImage = helpers.scaleImage(env.target,
                                         sizeInPixels.width,
                                         sizeInPixels.height)

  env.pac:addWidget{
      name = 'target',
      image = scaledImage,
      pos = helpers.getUpperLeftFromCenter(helpers.getTargetCenter(env, CENTER),
                                           targetSize),
      size = {targetSize, targetSize},
  }
end

--[[ Scales the provided image tensor so it corresponds to a given fraction of
the screen.

'source' (tensor) Image to scale, dimensions must be {3, height, width}.
'imageFraction' (table {width = x, height = y} where x, y = numbers in (0, 1])
output.
size as a fraction of the screen.
'screenPixels' (table {width = int, height = int} full size
of the screen in pixels.

returns the scaled tensor
]]
function helpers.scaleImageToScreenFraction(source, imageFraction, screenPixels)
  assert(imageFraction.width > 0 and imageFraction.width <= 1,
         'imageFraction.width must be in (0, 1]')
  assert(imageFraction.height > 0 and imageFraction.height <= 1,
         'imageFraction.height must be in (0, 1]')
  local sizeInPixels = helpers.getSizeInPixels(screenPixels,
                                               imageFraction.width,
                                               imageFraction.height)
  return helpers.scaleImage(source, sizeInPixels.width, sizeInPixels.height)
end

--[[ Sizes the input while preserving its aspect ratio.
Any unused area of the output is filled in with black.

'source' (tensor) Image to scale, dimensions must be {height, width, 3}.
'sizex' (int) Horizontal output dimension in pixels.
'sizey' (int) Vertical output dimension in pixels.

returns the scaled tensor
]]
function helpers.scaleImage(source, sizex, sizey)
  -- Scale input.
  local shape = source:shape()
  local scale = math.min(sizex / shape[2], sizey / shape[1])
  local width = math.floor(scale * shape[2])
  local height = math.floor(scale * shape[1])
  local scaledInput = image.scale(source, width, height)
  -- Create output.
  local result = tensor.ByteTensor(sizey, sizex, 3)
  local narrow = result:
      narrow(1, math.floor((sizey - height) / 2) + 1, height):
      narrow(2, math.floor((sizex - width) / 2) + 1, width)
  narrow:copy(scaledInput)
  return result
end

--[[ Returns the upper left pixel of an image of size `size` from its center.
'size' may be table or number, if number then it assumes equal height and width.
]]
function helpers.getUpperLeftFromCenter(center, size)
  if type(size) == 'number' then
    size = {size, size}
  end
  return {center[1] - size[1] / 2, center[2] - size[2] / 2}
end

--[[ Set the maximum number of trials per episode. An episode will
terminate after it either times out or completes this many trials.
'env' (table) Psychlab task environment class.
'trialsPerEpisodeCap' (int) episode to end after this many trials.
]]
function helpers.setTrialsPerEpisodeCap(env, trialsPerEpisodeCap)
  env._maxTrialsPerEpisode = trialsPerEpisodeCap
  env._currNumTrialsThisEpisode = 0
end

--[[ Publishes trial data using dmlab events.
'trialData' (table) All the data items to be published.
'schema' (optional string) Name of the experiment schema (normally the
same as the name of the level). This can be used by the event receiver
to separate trial data from different experiments in a setup where
an agent is doing multiple experiments at the same time.
]]
function helpers.publishTrialData(trialData, schema)
  trialData = helpers.tostring(trialData)
  if schema then
    events:add('xdata:psychlab', trialData, schema)
  else
    events:add('xdata:psychlab', trialData)
  end
end

--[[ Provides common logic for tasks finishing a trial in an episode.
'env' (table) Psychlab task environment class.
'delay' (float) delay time (in seconds)
'fixationSize' (float) size of fixation image [0,1] as proportion of screen
]]
function helpers.finishTrialCommon(env, delay, fixationSize)
  log.info('finished trial')
  env:removeArray()
  env.pac:addTimer{
      name = 'delay',
      timeout = delay,
      callback =
        function(...) return helpers.addFixation(env, fixationSize) end,
  }

  env.currentTrial = {}

  if env._maxTrialsPerEpisode then
    env._currNumTrialsThisEpisode = env._currNumTrialsThisEpisode + 1

    if env._currNumTrialsThisEpisode >= env._maxTrialsPerEpisode then
      log.info('Episode ended after ' ..
               tonumber(env._currNumTrialsThisEpisode) .. ' trials')
      env.pac:endEpisode()
    end
  end

  io.flush()
  collectgarbage(); collectgarbage()
end

--[[ Adds fixation widgets to the environment
'env' (table) Psychlab task environment class.
'fixationSize' (float) size of fixation image [0,1] as proportion of screen
]]
function helpers.addFixation(env, fixationSize)
  -- Add the image widget.
  env.pac:addWidget{
      name = 'fixation',
      image = env.images.fixation,
      pos = helpers.getUpperLeftFromCenter(CENTER, fixationSize),
      size = {fixationSize, fixationSize},
  }

  -- Add the trigger widget to require fixating the center of the cross.
  env.pac:addWidget{
      name = 'center_of_fixation',
      image = nil,
      pos = helpers.getUpperLeftFromCenter(CENTER, fixationSize / 2),
      size = {fixationSize / 2, fixationSize / 2},
      mouseHoverCallback = env.fixationCallback
  }
end

--[[ Creates an image for the fixation cross used in PsychLab experiments
'screenSize' (table) Contains screen height and width, in pixels.
'bgColor' (tensor) RGB colour of background
'fixationColor' (tensor) RGB colour of fixation cross
'fixationSize' (number in (0, 1]) size of fixation cross as fraction of screen
]]
function helpers.getFixationImage(screenSize, bgColor, fixationColor,
                                  fixationSize)
  local sizeInPixels = helpers.getSizeInPixels(screenSize, fixationSize)
  local fixation = tensor.ByteTensor(sizeInPixels.height, sizeInPixels.width, 3)
  fixation:fill(bgColor)
  local fortyPercentY = math.floor(0.5 + 0.4 * sizeInPixels.height)
  local sixtyPercentY = sizeInPixels.height - fortyPercentY + 1
  local fortyPercentX = math.floor(0.5 + 0.4 * sizeInPixels.width)
  local sixtyPercentX = sizeInPixels.width - fortyPercentX + 1
  local xBand = fixation:narrow(2, fortyPercentX,
                                sixtyPercentX + 1 - fortyPercentX)
  local yBand = fixation:narrow(1, fortyPercentY,
                                sixtyPercentY + 1 - fortyPercentY)
  for i = 1, #fixationColor do
    xBand:select(3, i):fill(fixationColor[i])
    yBand:select(3, i):fill(fixationColor[i])
  end
  return fixation
end

--[[ Returns a byte tensor with a solid cirlce.

Arguments:

*   'size' Height and width of returned image. Image will have #fgColor chanels.
*   'fgColor' Color of circle.
*   'bgColor' Color of Background.

--
Example:
```
> print(helpers.makeFilledCircle(8, {1}, {0}):reshape(8,8))
[deepmind.lab.tensor.ByteTensor]
Shape: [8, 8]
 [0, 0, 1, 1, 1, 1, 0, 0]
 [0, 1, 1, 1, 1, 1, 1, 0]
 [1, 1, 1, 1, 1, 1, 1, 1]
 [1, 1, 1, 1, 1, 1, 1, 1]
 [1, 1, 1, 1, 1, 1, 1, 1]
 [1, 1, 1, 1, 1, 1, 1, 1]
 [0, 1, 1, 1, 1, 1, 1, 0]
 [0, 0, 1, 1, 1, 1, 0, 0]
```
]]
function helpers.makeFilledCircle(size, fgColor, bgColor)
  assert(#fgColor == #bgColor)
  local circle = tensor.ByteTensor(size, size, #fgColor):fill(fgColor)
  local bg = tensor.ByteTensor(bgColor)
  local radius = size / 2
  local radiusSquared = radius ^ 2
  for row = 1, size do
    local circleRow = circle(row)
    for col = 1, size do
      local x = col - radius - 0.5
      local y = row - radius - 0.5
      if x * x + y * y >= radiusSquared then
        circleRow(col):copy(bg)
      end
    end
  end
  return circle
end

--[[ Converts size as a fraction of the screen to pixels using the full size of
the screen in pixels.
'screenSize' (table) Contains screen height and width, in pixels.
'size' (table {width, height}) Size as a fraction of the full screen.
]]
function helpers.getSizeInPixels(screenSize, size)
  return {
      width = math.floor(size * screenSize.width),
      height = math.floor(size * screenSize.height)
  }
end

-- Returns a uniform random value in range [-maxJitter, maxJitter]
function helpers.getJitter(maxJitter)
  return (random:uniformFloat(0.0, 1.0) * 2 * maxJitter) - maxJitter
end

--[[ Applies jitter to the provided XY point
'env' (table) Psychlab task environment class.
'center' (tensor) XY coordinate value
Returns: (tensor) jittered point
]]
function helpers._randomShift(env, center)
  env.jitteredCenter[1] = center[1] + helpers.getJitter(MAX_JITTER)
  env.jitteredCenter[2] = center[2] + helpers.getJitter(MAX_JITTER)
  log.info('jittered target center: ' .. tostring(env.jitteredCenter[1]) ..
           ', ' .. tostring(env.jitteredCenter[2]))
  return env.jitteredCenter
end

function helpers.getTargetCenter(env, center)
  if env.jitter then
    return helpers._randomShift(env, center)
  else
    return center
  end
end

--[[ Returns a random element from input
'input' (array) list of values
Returns:
1 random element from the input
2 index corresponding to the random element
]]
function helpers.randomFrom(input)
  local index = random:uniformInt(1, #input)
  return input[index], index
end

--[[ Returns an array with the range of values in closed interval ['lo', 'hi']
]]
function helpers.range(lo, hi, step)
  local step = step or (lo <= hi) and 1 or -1
  local range = {}
  for i = lo, hi, step do
    table.insert(range, i)
  end
  return range
end

--[[ Naive recursive pretty-printer.
Prints the table hierarchically. Assumes all the keys are simple values.
]]
function helpers.tostring(input, spacing, limit)
  limit = limit or 5
  if limit < 0 then
    return ''
  end
  spacing = spacing or ''
  if type(input) == 'table' then
    local res = '{\n'
    for k, v in pairs(input) do
      if type(k) ~= 'string' or string.sub(k, 1, 2) ~= '__' then
        res = res .. spacing .. '  [\'' .. tostring(k) .. '\'] = ' ..
          helpers.tostring(v, spacing .. '  ', limit - 1)
      end
    end
    return res .. spacing .. '}\n'
  else
    return tostring(input) .. '\n'
  end
end

return helpers
