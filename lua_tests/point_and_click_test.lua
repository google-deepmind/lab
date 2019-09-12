--[[ Copyright (C) 2018-2019 Google Inc.

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

local asserts = require 'testing.asserts'
local test_runner = require 'testing.test_runner'
local tensor = require 'dmlab.system.tensor'
local point_and_click = require 'factories.psychlab.point_and_click'

local tests = {}

local function environment(opts)
  return {
    _inits = 0,
    _steps = 0,
    _resets = 0,
    init = function(self, opts) self._inits = self._inits + 1 end,
    step = function(self, opts) self._steps = self._steps + 1 end,
    reset = function(self, opts) self._resets = self._resets + 1 end,
  }
end

function tests.addWidgetAbsolute()
  local SIZE = 8
  local pac = point_and_click{
      environment = environment,
      screenSize = {height = SIZE, width = SIZE},
  }
  pac:reset(0, 0)
  local lookCenter = {{0, 0}, {0.5, 0.5}}
  pac:step(lookCenter)

  local relativeMousePos = {}
  for i = 1, SIZE do
    local size = (SIZE - i + 1)
    pac:addWidget{
        name = 'GREY_' .. i,
        posAbs = {i - 1, i - 1},
        sizeAbs = {size, size},
        image = tensor.ByteTensor(size, size, 3):fill(i),
        imageLayer = i,
        mouseHoverCallback = function(self, name, mousePos, hoverTime, userData)
          userData[name] = {mousePos[1], mousePos[2]}
        end,
        userData = relativeMousePos,
    }
  end
  local surface, reward, pcontinue, wasDirty = pac:step(lookCenter)

  local result = tensor.ByteTensor(SIZE, SIZE, 4)
  result:applyIndexed(function(val, index)
      local y, x, c = unpack(index)
      return c < 4 and (x < y and x or y) or 255
  end)
  asserts.EQ(result, surface, 'Invalid surface')
  asserts.EQ(reward, 0, 'Invalid reward')
  asserts.EQ(pcontinue, 1, 'Invalid pcontinue')
  asserts.EQ(wasDirty, true, 'Invalid wasDirty')
  asserts.tablesEQ(relativeMousePos.GREY_1, {0.5, 0.5}, 'Bad widget position!')
  local surface, reward, pcontinue, wasDirty = pac:step{{0, 0}, {0.1, 0.1}}
  asserts.tablesEQ(relativeMousePos.GREY_1, {0.1, 0.1}, 'Bad widget position!')
  asserts.EQ(wasDirty, false, 'Invalid wasDirty')
end

function tests.addWidgetsRelative()
  local SIZE = 7
  local pac = point_and_click{
      environment = environment,
      screenSize = {height = SIZE, width = SIZE},
  }
  pac:reset(0, 0)

  local relativeMousePos = {}
  for i = 1, SIZE do
    local p = (i - 1) / SIZE
    local size = (SIZE - i + 1)
    pac:addWidget{
        name = 'GREY_' .. i,
        pos = {p, p},
        size = {size / SIZE, size / SIZE},
        image = tensor.ByteTensor(size, size, 3):fill(i),
        imageLayer = i,
        mouseHoverCallback = function(self, name, mousePos, hoverTime, userData)
          userData[name] = {mousePos[1], mousePos[2]}
        end,
        userData = relativeMousePos,
    }
  end
  local surface, reward, pcontinue, wasDirty = pac:step{{0, 0}, {0.5, 0.5}}

  local result = tensor.ByteTensor(SIZE, SIZE, 4)
  result:applyIndexed(function(val, index)
      local y, x, c = unpack(index)
      return c < 4 and (x < y and x or y) or 255
  end)
  asserts.EQ(result, surface, 'Invalid surface')
  asserts.EQ(reward, 0, 'Invalid reward')
  asserts.EQ(pcontinue, 1, 'Invalid pcontinue')
  asserts.EQ(wasDirty, true, 'Invalid wasDirty')
  asserts.tablesEQ(relativeMousePos.GREY_1, {0.5, 0.5}, 'Bad widget position!')

  local surface, reward, pcontinue, wasDirty = pac:step{{0, 0}, {0.1, 0.1}}
  asserts.tablesEQ(relativeMousePos.GREY_1, {0.1, 0.1}, 'Bad widget position!')
  asserts.EQ(wasDirty, false, 'Invalid wasDirty')
end


function tests.addWidgetsClipped()
  local SIZE = 10
  local pac = point_and_click{
      environment = environment,
      screenSize = {height = SIZE, width = SIZE},
  }
  pac:reset(0, 0)

  local relativeMousePos = {}

  local positions = {{0, 0}, {1, 0}, {0, 1}, {1, 1}}
  for i, p in ipairs(positions) do
    local layer = i
    pac:addWidget{
        name = 'GREY_' .. layer,
        pos = p,
        posAbs = {-5, -5},
        sizeAbs = {10, 10},
        image = tensor.ByteTensor(10, 10, 3):fill(layer),
        imageLayer = layer,
        mouseHoverCallback = function(self, name, mousePos, hoverTime, userData)
          userData[name] = {mousePos[1], mousePos[2]}
        end,
        userData = relativeMousePos,
    }
  end

  local surface, reward, pcontinue, wasDirty = pac:step{{0, 0}, {0.5, 0.5}}

  local result = tensor.ByteTensor(SIZE, SIZE, 4)
  result:applyIndexed(function(val, index)
      local y, x, c = unpack(index)
      if c == 4 then return 255 end
      if x <= 5 and y <= 5 then return 1 end
      if x > 5 and y <= 5 then return 2 end
      if x <= 5 and y > 5 then return 3 end
      if x > 5 and y > 5 then return 4 end
  end)
  asserts.EQ(result, surface, 'Invalid surface')
  asserts.EQ(reward, 0, 'Invalid reward')
  asserts.EQ(pcontinue, 1, 'Invalid pcontinue')
  asserts.EQ(wasDirty, true, 'Invalid wasDirty')

  for i, p in ipairs(positions) do
    for k in pairs(relativeMousePos) do
      relativeMousePos[k] = nil
    end
    pac:step{{0, 0}, p}
    asserts.tablesEQ(relativeMousePos['GREY_' .. i], {0.5, 0.5},
                     'Bad widget position! - GREY_' .. i)
  end
end

return test_runner.run(tests)
