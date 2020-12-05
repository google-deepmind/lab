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

local tensor = require 'dmlab.system.tensor'
local log = require 'common.log'

--[[ This is the interface for Point and Click environments. All co-ordinates
are normalised to [0, 1]. Hover and callback times are in seconds as floating
points.
]]
local pac = {}
pac.__index = pac

setmetatable(pac, {
    __call = function (cls, ...)
      local self = setmetatable({}, cls)
      self:_init(...)
      return self
    end
})

--[[ Initialise with the opts provided.

Keyword arguments:

*   `environment` (function) Needs to returns an environment constructed with
    self. The constructed environment can have two optional callbacks.
    environment:reset() and environment:step().
*   `screenSize` (table) {width=width, height=height} The screen size.
]]
function pac:_init(opts)
  local screen = opts.screenSize
  self._surface = tensor.ByteTensor(screen.height, screen.width, 4):fill(255)
  self._surfaceDirty = true
  self._reward = 0
  self._pcontinue = 0
  self._stepCount = 0
  self._captures = {}
  self._widgets = {}
  self._layers = {}
  self._timers = {}
  self._backgroundColor = {0, 0, 0}
  self.maxStepsOffScreen = opts.maxStepsOffScreen or -1
  self._env = opts.environment(self, opts)
end

-- Rewards should be integer values, otherwise they may get floored. This will
-- always be the case when running inside Labyrinth.
function pac:addReward(reward)
  local _, fraction = math.modf(reward)
  if fraction ~= 0 then
    log.warn('Fractional rewards may get floored: ' .. reward)
  end
  self._reward = self._reward + reward
end

-- End the episode.
function pac:endEpisode()
  self._pcontinue = 0
end

--[[ set the color with which to fill the background of the surface.
*   `rgbTable` (table) Three numbers, red, green, and blue, each in [0, 255].
]]
function pac:setBackgroundColor(rgbTable)
  assert(#rgbTable == 3, 'Incorrect color format, should be RGB.')
  for i = 1, 3 do
    assert(rgbTable[i] >= 0 and rgbTable[i] <= 255, 'color must be in [0, 255]')
  end
  self._backgroundColor = rgbTable
end

--[[ Add a widget.

Note posAbs and sizeAbs can be combined with pos and size.
Example:

Align center-right is achieved with:

    local imageY, imageX = unpack(image:shape())
    pac:addWidget{
        name = "WIDGET_CENTRE_RIGHT"
        pos = {1.0, 0.5},
        posAbs = {-imageX, math.floor(-imageY/2)},
        sizeAbs = {imageX, imageY},
    }

Keyword arguments:

*   `name` Name of the widget - must be unique.
*   `posAbs` = {xPos, yPos} Widget position in pixels (BL origin 0,0).
*   `sizeAbs` = {width, height} The widget size in pixels.
*   `pos` = {xPos, yPos} Widget position relative to screen.
*   `size` = {width, height} The widget size relative to screen.
*   `image` (ByteTensor{hxwx3}, optional).
*   `imageLayer` (int, default = 1) Order to draw the images, 1 being the
    bottom.
*   `userData` (optional) Passed back in callbacks.
*   `mouseClickCallback` (function(self, name, mousePos, userData), optional)
    Called on mouse click events.
*   `mouseHoverCallback` (function(self, name, mousePos, hoverTime, userData),
    optional) Called on mouse over events.
*   `mouseHoverEndCallback` (function(self, name, userData),
    optional) Called on mouse no-longer-over events.
]]
function pac:addWidget(opts)
  local name = opts.name
  assert(self._widgets[name] == nil, 'Widget ' .. opts.name .. ' exists!')
  self._surfaceDirty = true
  local widget = {}
  local sizeY, sizeX = unpack(self._surface:shape())
  local posAbs = opts.posAbs or {0, 0}
  local sizeAbs = opts.sizeAbs or {0, 0}
  local pos = opts.pos or {0, 0}
  local size = opts.size or {0, 0}

  local xMin = posAbs[1] + pos[1] * sizeX
  local yMin = posAbs[2] + pos[2] * sizeY
  local xMax = xMin + sizeAbs[1] + size[1] * sizeX
  local yMax = yMin + sizeAbs[2] + size[2] * sizeY

  if xMax <= xMin or yMax <= yMin or
      xMax <= 0 or yMax <= 0 or
      xMin >= sizeX or yMin >= sizeY then
    error('Widget ' .. opts.name .. ' Invalid bounds!' ..
          '\nxMin: ' .. xMin .. ' xMax: ' .. xMax ..
          '\nyMin: ' .. yMin .. ' yMax: ' .. yMax ..
          '\nsizeX: ' .. sizeX .. ' sizeY: ' .. sizeY)
  end
  widget.bounds = {xMin = xMin, yMin = yMin, xMax = xMax, yMax = yMax}
  widget.image = opts.image
  widget.mouseClickCallback = opts.mouseClickCallback
  widget.mouseClickUpCallback = opts.mouseClickUpCallback
  widget.mouseHoverCallback = opts.mouseHoverCallback
  widget.mouseHoverEndCallback = opts.mouseHoverEndCallback
  widget.mouseClickUpCallback = opts.mouseClickUpCallback
  widget.userData = opts.userData
  widget.hoverTime = 0
  widget.imageLayer = opts.imageLayer or 1
  self._widgets[name] = widget
end

function pac:captureMouse(name)
  self._captures[name] = true
end


function pac:releaseMouse(name)
  self._captures[name] = nil
end


--[[ Set a widget's absolute position.

Arguments:

*   `name` Name of the widget.
*   `posAbs` = {xPos, yPos} Widget position in pixels (BL origin 0,0).
]]
function pac:setWidgetAbsPos(name, absPos)
  assert(self._widgets[name] ~= nil, 'Widget ' .. name .. ' does not exist!')
  local bounds = self._widgets[name].bounds
  local xMin, yMin = unpack(absPos)
  if xMin == bounds.xMin and yMin == bounds.yMin then
    return
  end
  local xMax = bounds.xMax + xMin - bounds.xMin
  local yMax = bounds.yMax + yMin - bounds.yMin
  local ySize, xSize = unpack(self._surface:shape())
  if xMax <= 0 or yMax <= 0 or xMin >= xSize or yMin >= ySize then
    error('Widget ' .. name .. ' Invalid bounds!' ..
          '\nxMin: ' .. xMin .. ' xMax: ' .. xMax ..
          '\nyMin: ' .. yMin .. ' yMax: ' .. yMax ..
          '\nsizeX: ' .. xSize .. ' sizeY: ' .. ySize)
  end
  bounds.xMin = xMin
  bounds.yMin = yMin
  bounds.xMax = xMax
  bounds.yMax = yMax
  self._surfaceDirty = true
end

--[[ Remove a widget.
Arguments:

*   `name` The widget to remove.
]]
function pac:removeWidget(name)
  self._surfaceDirty = true
  self._widgets[name] = nil
end

-- Removes all widgets.
function pac:clearWidgets()
  self._surfaceDirty = true
  self._layersDirty = true
  self._widgets = {}
  self._captures = {}
  self._layers = {}
end

--[[ Update a widget's image.
Arguments:

*   `name` Name of the widget.
*   `image` (ByteTensor{hxwx3}) The new image.
*   `imageLayer` (optional int) Images are drawn in order of their imageLayer
]]
function pac:updateWidget(name, image, imageLayer)
  local widget = self._widgets[name]
  assert(widget, 'Widget ' .. name .. ' does not exist!')
  imageLayer = imageLayer or widget.imageLayer
  if imageLayer ~= widget.imageLayer or image ~= widget.image then
    self._surfaceDirty = true
    widget.image = image
    widget.imageLayer = imageLayer
  end
end

--[[ Update a widget's image.
Arguments:

*   `name` Name of the widget.
*   `imageLayer` Images are drawn in order of their imageLayer
]]
function pac:setWidgetLayer(name, imageLayer)
  local widget = self._widgets[name]
  assert(widget, 'Widget ' .. name .. ' does not exist!')
  if imageLayer ~= widget.imageLayer then
    self._surfaceDirty = true
    widget.imageLayer = imageLayer
  end
end

--[[ Add a timer.

Keyword Arguments:

*   `name` - Name of the timer.
*   `timeout` (integer)- Time in frames.
*   `callback` (function(self, timer))- the function to call on timeout.
*   `userData` (optional) - user data to pass back with the callback.
]]
function pac:addTimer(opts)
  assert(self._timers[opts.name] == nil, 'Timer ' .. opts.name ..
    ' already exists!')
  assert(opts.callback ~= nil and opts.timeout ~= nil,
    'Must pass a callback function and timeout to addTimer!')

  self._timers[opts.name] = opts
end

-- Remove all timers.
function pac:clearTimers()
  self._timers = {}
end

-- Calculate if co-ordinates are within the widget.
function pac:_containsPoint(name, widget, mouseX, mouseY)
  return self._captures[name] or
      (widget.bounds.xMin <= mouseX and mouseX <= widget.bounds.xMax and
       widget.bounds.yMin <= mouseY and mouseY <= widget.bounds.yMax)
end

-- Calculate normalised co-ordinates with respect to the widget.
local function _normaliseCoord(widget, mouseX, mouseY)
  return {
      (mouseX - widget.bounds.xMin) / (widget.bounds.xMax - widget.bounds.xMin),
      (mouseY - widget.bounds.yMin) / (widget.bounds.yMax - widget.bounds.yMin),
      mouseX, mouseY
  }
end

-- Calculates which widgets the mouse is over and invokes any callbacks.
function pac:onMouseOver(mouseX, mouseY)
  local deferredCallbacks = {}
  for name, widget in pairs(self._widgets) do
    if widget.mouseHoverCallback ~= nil or
        widget.mouseHoverEndCallback ~= nil then
      if self:_containsPoint(name, widget, mouseX, mouseY) then
        local hoverTime = widget.hoverTime + 1
        widget.hoverTime = hoverTime
        if widget.mouseHoverCallback then
          table.insert(deferredCallbacks,
            function()
              local thisWidget = self._widgets[name]
              if thisWidget then
                thisWidget.mouseHoverCallback(self._env,
                  name, _normaliseCoord(thisWidget, mouseX, mouseY),
                  hoverTime, thisWidget.userData)
              end
            end
          )
        end
      elseif widget.hoverTime > 0 then
        if widget.mouseHoverEndCallback then
          table.insert(deferredCallbacks,
            function()
              local thisWidget = self._widgets[name]
              if thisWidget then
                thisWidget.mouseHoverEndCallback(self._env, name,
                  thisWidget.userData)
              end
            end
          )
        end
        widget.hoverTime = 0
      end
    end
  end
  for _, f in ipairs(deferredCallbacks) do f() end
end

-- Calculates which widgets the mouse click is on and invokes any callbacks.
function pac:onMouseClick(mouseX, mouseY)
  for i = #self._layerOrder, 1, -1 do
    local layer = self._layers[self._layerOrder[i]]
    for j = #layer, 1, -1 do
      local name, widget = unpack(layer[j])
      if widget.mouseClickCallback ~= nil then
        if self:_containsPoint(name, widget, mouseX, mouseY) then
          if widget.mouseClickCallback(self._env, name,
              _normaliseCoord(widget, mouseX, mouseY), widget.userData) then
            return
          end
        end
      end
    end
  end
end


-- Calculates which widgets the mouse click is on and invokes any callbacks.
function pac:onMouseClickUp(mouseX, mouseY)
  for i = #self._layerOrder, 1, -1 do
    local layer = self._layers[self._layerOrder[i]]
    for j = #layer, 1, -1 do
      local name, widget = unpack(layer[j])
      if widget.mouseClickUpCallback ~= nil then
        if self:_containsPoint(name, widget, mouseX, mouseY) then
          if widget.mouseClickUpCallback(self._env, name,
              _normaliseCoord(widget, mouseX, mouseY), widget.userData) then
            return
          end
        end
      end
    end
  end
end

function pac:_drawBackgroundColor()
  for dim = 1, 3 do
    self._surface:select(3, dim):fill(self._backgroundColor[dim])
  end
end

function pac:_updateLayers()
  self._layers = {}
  self._layerOrder = {}
  for name, widget in pairs(self._widgets) do
    if not self._layers[widget.imageLayer] then
      self._layerOrder[#self._layerOrder + 1] = widget.imageLayer
      self._layers[widget.imageLayer] = {}
    end
    table.insert(self._layers[widget.imageLayer], {name, widget})
  end
  table.sort(self._layerOrder)
end

--[[ Draws all widgets that contain images into the screen tensor, which needs
to have format {height, width, depth} and a depth of at least 3.
]]
function pac:_drawWidgets()
  self:_updateLayers()
  self:_drawBackgroundColor()
  local maxy, maxx = unpack(self._surface:shape())
  for _, i in ipairs(self._layerOrder) do
    local layer = self._layers[i]
    for _, nameWidget in pairs(layer) do
      local name, widget = unpack(nameWidget)
      if widget.image then
        local offsetx = widget.bounds.xMin
        local offsety = widget.bounds.yMin
        local sizey, sizex = unpack(widget.image:shape())
        local image = widget.image
        -- Clip Right
        if sizex + offsetx > maxx then
          sizex = maxx - offsetx
          image = image:narrow(2, 1, sizex)
        end
        -- Clip Top
        if sizey + offsety > maxy then
          sizey = maxy - offsety
          image = image:narrow(1, 1, sizey)
        end

        -- Clip Left
        if offsetx < 0 then
          sizex = sizex + offsetx
          image = image:narrow(2, -offsetx + 1, sizex)
          offsetx = 0
        end
        -- Clip Bottom
         if offsety < 0 then
          sizey = sizey + offsety
          image = image:narrow(1, -offsety + 1, sizey)
          offsety = 0
        end
        -- Narrow the screen to the region for the image and do the copy.
        self._surface:
            narrow(1, offsety + 1, sizey):
            narrow(2, offsetx + 1, sizex):
            narrow(3, 1, 3):
            copy(image)
      end
    end
  end
  self._surfaceDirty = false
end

function pac:_observations()
  local reward = self._reward
  local pcontinue = self._pcontinue
  self._reward = 0
  self._pcontinue = 1

  local wasDirty = self._surfaceDirty
  if wasDirty then
    self:_drawWidgets()
  end
  return self._surface, reward, pcontinue, wasDirty
end

-- Environment Interface --

-- Clear all widgets and timers.
function pac:reset(episode_id, seed)
  if self._env.reset then
    self._env:reset(episode_id, seed)
  end
  self._stepCount = 0
  return self:_observations()
end

function pac:actionSpec()
  return {
      scheme = "Mixed",
      discrete_actions = {{0, 1}, {0, 1}},
      contiguous_actions = {{0, 1}, {0, 1}},
  }
end

function pac:observationSpec()
  return {
      scheme = "Bytes",
      size = {self.self.screen.height, self.screen.width, 3}
  }
end

function pac:step(action)
  assert(self._pcontinue > 0, 'step called before reset')

  -- Fire any timer callbacks that have been reached.
  local timer_callbacks = {}
  for key, timer in pairs(self._timers) do
    timer.timeout = timer.timeout - 1
    if timer.timeout <= 0 then
      self._timers[key] = nil
      -- Defer call as the callback may remove timers.
      table.insert(timer_callbacks, timer)
    end
  end
  for _, timer in ipairs(timer_callbacks) do
    timer.callback(self._env, timer)
  end

  -- Process the action.
  local click = action[1][1] == 1
  local clickUp = action[1][2] == 1
  local pos = action[2]
  local sizeY, sizeX = unpack(self._surface:shape())
  local mouseXAbs, mouseYAbs = pos[1] * sizeX, pos[2] * sizeY
  local lookingAtScreen = pos[1] ~= -1 and pos[2] ~= -1
  if click and lookingAtScreen then
    self:onMouseClick(mouseXAbs, mouseYAbs)
  elseif clickUp then
    self:onMouseClickUp(mouseXAbs, mouseYAbs)
    self:onMouseOver(mouseXAbs, mouseYAbs) -- Kept for legacy behaviour
  else
    self:onMouseOver(mouseXAbs, mouseYAbs)
  end

  -- Step the environment.
  if self._env.step then
    self._env:step(lookingAtScreen)
  end
  self._stepCount = self._stepCount + 1

  -- If too long looking away from the screen then end episode. This
  -- should speed up the early stages of training.
  if not lookingAtScreen and self.maxStepsOffScreen > 0 then
    self._stepsNotLookingAtScreen = self._stepsNotLookingAtScreen + 1
    if self._stepsNotLookingAtScreen > self.maxStepsOffScreen then
      log.info('End episode due to looking away from the screen.')
      self:endEpisode()
    end
  else
    self._stepsNotLookingAtScreen = 0
  end

  return self:_observations()
end

function pac:resetSteps()
  self._stepCount = 0
end

function pac:elapsedSteps()
  return self._stepCount
end

return pac
