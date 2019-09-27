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

-- Generates as many colors as needed and images are the size requested.
-- Maybe used in place of missing datasets.

local tensor = require 'dmlab.system.tensor'
local colors = require 'common.colors'
local psychlab_helpers = require 'factories.psychlab.helpers'

--[[ Creates a dataset of images of variably configured squares and Es at
different colors and orientations. The image at index i is generated the first
time it gets requested by proceduralDataset.getImage(i). Subsequent calls to
proceduralDataset.getImage(i) will always return the same image.

Arguments:

*   'numObjects' how many isolated objects to draw per image.
*   'screenSize' {width = (pixels), height = (pixels)}.
*   'gridParams' {size = (pixels), step = (pixels)}.
*   'targetSize' fraction of the screen to fill with generated image.

Methods:

*   'getImage' this function returns the image at the specified index. Even
    the dataset is procedural, the image at a specified index is always
    guaranteed to be unique. It generates and stores the image metadata the
    first time it is called for a given imageIndex.
*   'getSize' returns the number of images that have been generated so far.
]]
local function createProceduralDataset(numObjects, screenSize, gridParams,
                                       targetSize, bgColor)
  local COLORS = {{255, 191, 0}, -- sunflower yellow
                  {0, 255, 255}, -- light blue
                  {0, 63, 255}, -- dark blue
                  {127, 0, 255}, -- purple
                  {255, 0, 191}} -- -- magenta
  -- Types of objects to display
  local ALL_OPTOTYPES = {'E', 'Square'}
  -- Orientations at which to display the 'E' optotype
  local ORIENTATIONS = {'left', 'right', 'up', 'down'}
  --[[ Domains from which to sample optotypes. All optotypes in a given image
  are always from the same domain.]]
  local DOMAINS = {
      'E_ALL',
      'E_COLOR',
      'E_ORIENTATION',
      'SQUARE_COLOR',
      'ALL',
  }

  --[[ Returns a random point with integer-valued x, y coordinates.
  'limit' (integer) Largest possible x value
  'step' (integer) Defines the size of a grid square, should evenly divide limit
  Returns: (table {x, y}) where x and y are sampled from the grid.
  ]]
  local function getRandomCoordinates(limit, step)
    -- prepare domain
    local fullDomain = psychlab_helpers.range(1, limit, step)
    -- sample coordinates
    local x, _ = psychlab_helpers.randomFrom(fullDomain)
    local y, _ = psychlab_helpers.randomFrom(fullDomain)
    return {x, y}
  end

  local proceduralDataset = {}

  proceduralDataset.screenSize = screenSize
  proceduralDataset.gridParams = gridParams
  proceduralDataset.targetSize = targetSize
  proceduralDataset.numObjects = numObjects

  proceduralDataset._size = 0
  proceduralDataset.images = {}
  proceduralDataset.metaData = {}
  proceduralDataset._serializedMetaData = {}

  function proceduralDataset._setupGrid(opt)
    local screenSize = opt.screenSize
    local targetSize = opt.targetSize
    local gridParams = opt.gridParams

    -- The gridLimit defines the extent of the grid's subregion where
    -- procedural objects may be placed. We reserve the first and last row and
    -- column for other objects like buttons.
    proceduralDataset._gridLimit = gridParams.size - gridParams.step
    proceduralDataset._gridStep = gridParams.step

    assert(proceduralDataset._gridLimit % proceduralDataset._gridStep == 0,
           '_gridStep must evenly divide gridLimit')

    proceduralDataset.targetPixels = {
        width = targetSize * screenSize.width,
        height = targetSize * screenSize.height
    }

    proceduralDataset._hFactor = proceduralDataset.targetPixels.width /
                                 gridParams.size
    proceduralDataset._vFactor = proceduralDataset.targetPixels.height /
                                 gridParams.size
  end

  function proceduralDataset._getDomain(domainType)
    if domainType == 'E_ALL' then
      return {
          optotypes = {'E'},
          colors = COLORS,
          colorIds = psychlab_helpers.range(1, #COLORS),
          orientations = ORIENTATIONS
      }
    elseif domainType == 'E_COLOR' then
      local fixedOrientation, _ = psychlab_helpers.randomFrom(ORIENTATIONS)
      return {
          optotypes = {'E'},
          colors = COLORS,
          colorIds = psychlab_helpers.range(1, #COLORS),
          orientations = {fixedOrientation}
      }
    elseif domainType == 'E_ORIENTATION' then
      local fixedColor, fixedColorId = psychlab_helpers.randomFrom(COLORS)
      return {
          optotypes = {'E'},
          colors = {fixedColor},
          colorIds = {fixedColorId},
          orientations = ORIENTATIONS
      }
    elseif domainType == 'SQUARE_COLOR' then
      return {
          optotypes = {'Square'},
          colors = COLORS,
          colorIds = psychlab_helpers.range(1, #COLORS),
          -- orientation does nothing for a square
          orientations = ORIENTATIONS
      }
    elseif domainType == 'ALL' then
      return {
          optotypes = ALL_OPTOTYPES,
          colors = COLORS,
          colorIds = psychlab_helpers.range(1, #COLORS),
          orientations = ORIENTATIONS
      }
    end
  end

  -- Return a table with the properties of each study object to draw.
  function proceduralDataset._getArrayData()
    local domainType = psychlab_helpers.randomFrom(DOMAINS)
    local domain = proceduralDataset._getDomain(domainType)

    local arrayData = {
        locations = {},
        colors = {},
        colorIds = {},
        optotypes = {},
        orientations = {}
    }

    -- iterate over objects in the study array
    local currentLocationsSet = {}
    for i = 1, proceduralDataset.numObjects do
      -- generate random location, color, optotype, and orientation
      local location = getRandomCoordinates(proceduralDataset._gridLimit,
                                            proceduralDataset._gridStep)
      -- make sure the random location was not already used
      while currentLocationsSet[psychlab_helpers.tostring(location)] do
        location = getRandomCoordinates(proceduralDataset._gridLimit,
                                        proceduralDataset._gridStep)
      end
      currentLocationsSet[psychlab_helpers.tostring(location)] = true

      local color, index = psychlab_helpers.randomFrom(domain.colors)
      local colorId = domain.colorIds[index]
      local optotype = psychlab_helpers.randomFrom(domain.optotypes)
      local orientation
      if optotype == 'Square' then
        orientation = 'none'
      else
        orientation = psychlab_helpers.randomFrom(domain.orientations)
      end

      table.insert(arrayData.locations, location)
      table.insert(arrayData.colors, color)
      table.insert(arrayData.colorIds, colorId)
      table.insert(arrayData.optotypes, optotype)
      table.insert(arrayData.orientations, orientation)
    end
    return arrayData
  end

  function proceduralDataset._drawRectangle(location, color)
    -- fill with solid color
    for i = 1, #color do
      proceduralDataset._array:narrow(
        1, location.top, location.bottom - location.top):narrow(
        2, location.left, location.right - location.left):select(
        3, i):fill(color[i])
    end
  end

  function proceduralDataset._drawE(location, color, orientation)
    local height = location.bottom - location.top
    local width = location.right - location.left

    local twentyPercentY = math.floor(0.2 * height) + location.top
    local fortyPercentY = math.floor(0.4 * height) + location.top
    local sixtyPercentY = math.floor(0.6 * height) + location.top
    local eightyPercentY = math.floor(0.8 * height) + location.top

    local twentyPercentX = math.floor(0.2 * width) + location.left
    local fortyPercentX = math.floor(0.4 * width) + location.left
    local sixtyPercentX = math.floor(0.6 * width) + location.left
    local eightyPercentX = math.floor(0.8 * width) + location.left

    -- First draw a solid square
    proceduralDataset._drawRectangle(location, color)

    -- block out notches in the E by filling with background color
    if orientation == 'right' then
      proceduralDataset._array:narrow(
        1, twentyPercentY, fortyPercentY - twentyPercentY):narrow(
        2, fortyPercentX, location.right - fortyPercentX):fill(bgColor)
      proceduralDataset._array:narrow(
        1, sixtyPercentY, eightyPercentY - sixtyPercentY):narrow(
        2, fortyPercentX, location.right - fortyPercentX):fill(bgColor)
    elseif orientation == 'left' then
      proceduralDataset._array:narrow(
        1, twentyPercentY, fortyPercentY - twentyPercentY):narrow(
        2, location.left, sixtyPercentX - location.left):fill(bgColor)
      proceduralDataset._array:narrow(
        1, sixtyPercentY, eightyPercentY - sixtyPercentY):narrow(
        2, location.left, sixtyPercentX - location.left):fill(bgColor)
    elseif orientation == 'up' then
      proceduralDataset._array:narrow(
        1, location.top, sixtyPercentY - location.top):narrow(
        2, twentyPercentX, fortyPercentX - twentyPercentX):fill(bgColor)
      proceduralDataset._array:narrow(
        1, location.top, sixtyPercentY - location.top):narrow(
        2, sixtyPercentX, eightyPercentX - sixtyPercentX):fill(bgColor)
    elseif orientation == 'down' then
      proceduralDataset._array:narrow(
        1, fortyPercentY, location.bottom - fortyPercentY):narrow(
        2, twentyPercentX, fortyPercentX - twentyPercentX):fill(bgColor)
      proceduralDataset._array:narrow(
        1, fortyPercentY, location.bottom - fortyPercentY):narrow(
        2, sixtyPercentX, eightyPercentX - sixtyPercentX):fill(bgColor)
    end
  end

  --[[ Draw object of specified type
  Keyword arguments:
  * `optotype`, (string in {'Square', 'E'}) type of object to draw
  * `location`, (table) {top = ..., bottom = ..., left = ..., right = ...}
  * `color`, (table) RGB values for the color to draw the object
  * `orientation`, (string in {'left', 'right', up', 'down'}) orientation of E
  ]]
  function proceduralDataset._drawObject(opt)
    if opt.optotype == 'Square' then
      proceduralDataset._drawRectangle(opt.location, opt.color)
    elseif opt.optotype == 'E' then
      proceduralDataset._drawE(opt.location, opt.color, opt.orientation)
    else
      error('Unrecognized object type: ' .. opt.type)
    end
  end

  -- Create the image tensor to display
  function proceduralDataset._renderArray(arrayData)
    proceduralDataset._array = tensor.ByteTensor(
        proceduralDataset.targetPixels.height,
        proceduralDataset.targetPixels.width,
        3
    ):fill(bgColor)

    -- draw objects
    for i = 1, #arrayData.locations do
      local location = {
          left = math.floor(arrayData.locations[i][1] *
            proceduralDataset._hFactor),
          right = math.floor((arrayData.locations[i][1] +
            proceduralDataset.gridParams.step) * proceduralDataset._hFactor),
          top = math.floor(arrayData.locations[i][2] *
            proceduralDataset._vFactor),
          bottom = math.floor((arrayData.locations[i][2] +
            proceduralDataset.gridParams.step) * proceduralDataset._vFactor),
      }

      proceduralDataset._drawObject{location = location,
                                    color = arrayData.colors[i],
                                    optotype = arrayData.optotypes[i],
                                    orientation = arrayData.orientations[i]}
    end

    return proceduralDataset._array
  end

  -- This function wraps _getArrayData, ensures that no metaData is repeated.
  function proceduralDataset._getUniqueMetaData()
    local metaData = proceduralDataset._getArrayData()
    local attempts = 0
    while proceduralDataset._serializedMetaData[
        psychlab_helpers.tostring(metaData)] do
      metaData = proceduralDataset._getArrayData()
      attempts = attempts + 1
      assert(attempts <= 5000,
             'Failed to generate unique meta data after 5000 attempts')
    end
    proceduralDataset._serializedMetaData[
      psychlab_helpers.tostring(metaData)] = true
    return metaData
  end

  --[[ Get the image and metaData at index. Create them if they do not exist.
  ]]
  function proceduralDataset.getImage(index)
    if proceduralDataset.images[index] == nil then
      proceduralDataset.metaData[index] = proceduralDataset._getUniqueMetaData()
      proceduralDataset.images[index] = proceduralDataset._renderArray(
        proceduralDataset.metaData[index])
      proceduralDataset._size = proceduralDataset._size + 1
    end
    return proceduralDataset.images[index], proceduralDataset.metaData[index]
  end

  function proceduralDataset.getSize()
    return proceduralDataset._size
  end

  proceduralDataset._setupGrid{
      screenSize = proceduralDataset.screenSize,
      targetSize = proceduralDataset.targetSize,
      gridParams = proceduralDataset.gridParams
  }

  return proceduralDataset
end

--[[ Returns a procedurally generated dataset consisting of rotated and scaled
Es and squares in various colors.

Arguments:

*   'numObjects' Number of isolated objects in each image.
*   'maxNumberOfImages' Maximum number of images to generate.
*   'screenSize' {width = (pixels), height = (pixels)}.
*   'gridParams' {size = (pixels), step = (pixels)}.
*   'targetSize' fraction of the screen to fill with generated image.
]]
local function dataset(numObjects, maxNumberOfImages, screenSize,
                       gridParams, targetSize, bgColor)
  assert(numObjects > 0, 'numObjects must be > 0')
  assert(maxNumberOfImages > 0,
         'maxNumberOfImages must be > 0')

  local data = {}
  local proceduralDataset = createProceduralDataset(
    numObjects, screenSize, gridParams, targetSize, bgColor)

  function data:getImage(imageIndex)
    local result = proceduralDataset.getImage(imageIndex)
    return result
  end

  function data:getSize()
    return maxNumberOfImages
  end
  return data
end

return dataset
