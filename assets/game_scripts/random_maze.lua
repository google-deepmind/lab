local maze_gen = require 'dmlab.system.maze_generation'
local tensor = require 'dmlab.system.tensor'
local random = require 'common.random'
local make_map = require 'common.make_map'
local pickups = require 'common.pickups'
local custom_observations = require 'decorators.custom_observations'
local timeout = require 'decorators.timeout'

-- Generates a random maze with thick walls.
-- Apples are placed near the goal and spawn points away from it.
-- Timeout is set to 3 minutes.
local api = {}

local function getRandomEvenCoodinate(rows, cols)
  -- Shape must be bigger than 3 and odd.
  assert (rows > 2 and rows % 2 == 1)
  assert (cols > 2 and cols % 2 == 1)
  return {
      random.uniformInt(1, math.floor(rows / 2)) * 2,
      random.uniformInt(1, math.floor(cols / 2)) * 2
  }
end

local function findVisitableCells(r, c, mazeT)
  local shape = mazeT:shape()
  local vistableCells = {}
  if r - 2 > 1 and mazeT(r - 2, c):val() == 0 then
    vistableCells[#vistableCells + 1] = {{r - 2, c}, {r - 1, c}}
  end
  if c - 2 > 1 and mazeT(r, c - 2):val() == 0 then
    vistableCells[#vistableCells + 1] = {{r, c - 2}, {r, c - 1}}
  end

  if r + 2 < shape[1] and mazeT(r + 2, c):val() == 0 then
    vistableCells[#vistableCells + 1] = {{r + 2, c}, {r + 1, c}}
  end
  if c + 2 < shape[2] and mazeT(r, c + 2):val() == 0 then
    vistableCells[#vistableCells + 1] = {{r, c + 2}, {r, c + 1}}
  end
  return vistableCells
end

local function generateTensorMaze(rows, cols)
  local maze = tensor.ByteTensor(rows, cols)
  local start = getRandomEvenCoodinate(rows, cols)
  local stack = {start}
  while #stack ~= 0 do
    local r, c = unpack(stack[#stack])
    maze(r, c):val(1)
    local vistableCells = findVisitableCells(r, c, maze)
    if #vistableCells > 0 then
      local choice = vistableCells[random.uniformInt(1, #vistableCells)]
      maze(unpack(choice[2])):val(1)
      stack[#stack + 1] = choice[1]
    else
      stack[#stack] = nil
    end
  end
  return maze
end

function api:commandLine(oldCommandLine)
  return make_map.commandLine(oldCommandLine)
end

function api:createPickup(className)
  return pickups.defaults[className]
end

function api:start(episode, seed, params)
  random.seed(seed)
  local rows, cols = 15, 15
  local mazeT = generateTensorMaze(rows, cols)
  local maze = maze_gen.MazeGeneration{height = rows, width = cols}
  local variations = {'.', 'A', 'B', 'C'}
  mazeT:applyIndexed(function(val, index)
    local row, col = unpack(index)
    if 1 < row and row < rows and 1 < col and row < rows and
        random.uniformReal(0, 1) < 0.15 then
      maze:setEntityCell(row, col, ' ')
    else
      maze:setEntityCell(row, col, val == 0 and '*' or ' ')
    end
    local variation = 1
    variation = variation + (row > rows / 2 and 1 or 0)
    variation = variation + (col > cols / 2 and 2 or 0)
    maze:setVariationsCell(row, col, variations[variation])
  end)

  local goal = getRandomEvenCoodinate(rows, cols)

  maze:setEntityCell(goal[1], goal[2], 'G')

  print('Maze Generated:')
  print(maze:entityLayer())

  print('Adding spawn points and apples:')
  maze:visitFill{
      cell = goal,
      func = function(row, col, distance)
        if distance > 5 then
            maze:setEntityCell(row, col, 'P')
        end
        if 0 < distance and distance < 5 then
            maze:setEntityCell(row, col, 'A')
        end
      end
  }

  print(maze:entityLayer())
  io.flush()
  api._maze_name = make_map.makeMap('map_' .. episode .. '_' .. seed,
                                    maze:entityLayer(), maze:variationsLayer())
end

function api:nextMap()
  return api._maze_name
end

custom_observations.decorate(api)
timeout.decorate(api, 3 * 60)

return api
