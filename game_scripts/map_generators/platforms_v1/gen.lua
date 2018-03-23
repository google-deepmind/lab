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

local maze_generation = require 'dmlab.system.maze_generation'

-- The heights of the cells are integers in [1, 26], larger number denotes
-- higher in height.
local MAX_HEIGHT = 26
local MIN_HEIGHT = 8
-- Any none-starting platform should be at least START_HEIGHT_OFFSET lower than
-- the starting platform. This allows the player to overview the whole maze at
-- the start.
local START_HEIGHT_OFFSET = 5
local HEIGHT_DECREASE_PROBABILITY = 0.3
local MAX_HEIGHT_DECREASE = 2

local MIN_LEVEL_SIZE = 4
local MAX_LEVEL_SIZE = 10

local MIN_PATHS = 1
local MAX_PATHS = 8

-- Maximum iterations any part of the algorithm can take.
local MAX_ITERATIONS = 1000
local rng

local function getWeightedValue(min, max, weight)
  return min + math.floor(weight * (max - min) + 0.5)
end

-- Generates a player position which is randomly chosen from the first 1/3 rows
-- of the map.
local function randomPlayerPosition(size)
  local row = math.floor(rng:uniformReal(0, 1) / 3 * size) + 1
  local col = rng:uniformInt(2, size - 1)
  return {row, col}
end

-- Generates a goal position which is randomly chosen from the last 1/3 rows
-- of the map.
local function randomGoalPosition(size)
  local row = math.floor((2.0 + rng:uniformReal(0, 1)) / 3 * size) + 1
  local col = rng:uniformInt(2, size - 1)
  return {row, col}
end

-- Returns the L^1 distance of two points.
local function l1Distance(pos1, pos2)
  return math.abs(pos1[1] - pos2[1]) + math.abs(pos1[2] - pos2[2])
end

--[[ Generates a random path from player position to goal position.

This is under the condition that the path must pass some new cells and the
length of path must not be too long.

Returns nil if conditions cannot be satisfied after MAX_ITERATIONS iterations.
]]
local function generateRandomPath(maze, playerPos, goalPos, maxExtraSteps)
  local distance = l1Distance(playerPos, goalPos)
  local path, numNewCells
  for iters = 1, MAX_ITERATIONS do
    path = {}
    maze:visitRandomPath{
        seed = rng:uniformInt(0, math.pow(2, 31)),
        from = playerPos,
        to = goalPos,
        func = function(row, col)
          path[#path + 1] = {row, col}
        end
    }
    local extraSteps = #path - 1 - distance
    numNewCells = 0
    if extraSteps <= maxExtraSteps then
      for i = 1, #path do
        if maze:getVariationsCell(path[i][1], path[i][2]) == '.' then
          numNewCells = numNewCells + 1
        end
      end
    end
    if numNewCells > 0 then
      return path
    end
  end
  return nil
end

-- Generates the height of the platforms on the main path.
local function generateHeightForMainPath(map, path)
  local lastHeight = MAX_HEIGHT - START_HEIGHT_OFFSET
  for i = 2, #path do
    local x, y = path[i][1], path[i][2]
    local height = lastHeight
    if rng:uniformReal(0, 1) < HEIGHT_DECREASE_PROBABILITY then
      height = lastHeight - rng:uniformInt(1, MAX_HEIGHT_DECREASE)
    end
    if height < MIN_HEIGHT then
      height = MIN_HEIGHT
    end
    map[x][y] = height
    lastHeight = height
  end
end

-- Permutes the sequence p, p + 1, ..., q.
local function permute(p, q)
  local permutedIndex = {}
  for i = p, q do
    permutedIndex[i] = i
  end
  for i = p, q do
    local j = rng:uniformInt(i, q)
    permutedIndex[i], permutedIndex[j] = permutedIndex[j], permutedIndex[i]
  end
  return permutedIndex
end

-- Generates the height of the platforms on a secondary path.
local function generateHeightForSecondaryPath(map, path)
  local p = 1
  local iters = 0
  while p < #path do
    iters = iters + 1
    assert(iters < MAX_ITERATIONS, "Max iterations!")
    -- Finds the next sub-path path[p : q - 1] so that path[p - 1] and path[q]
    -- are cells in the main path, but path[p : q - 1] are not.
    while p <= #path and map[path[p][1]][path[p][2]] ~= 0 do
      p = p + 1
    end
    if p > #path then break end
    local q = p + 1
    while q <= #path and map[path[q][1]][path[q][2]] == 0 do
      q = q + 1
    end
    assert(q <= #path)

    -- Computes the heights for the sub-path path[p : q - 1].
    local startHeight = map[path[p - 1][1]][path[p - 1][2]]
    if startHeight > MAX_HEIGHT - START_HEIGHT_OFFSET then
      startHeight = MAX_HEIGHT - START_HEIGHT_OFFSET
    end

    -- The height of path[q - 1] must be lower than the path[q].
    local endHeight = math.min(map[path[p - 1][1]][path[p - 1][2]] - 1,
                                map[path[q][1]][path[q][2]] - 1)
    assert(startHeight >= endHeight)
    -- Distributes the height drop evenly among the sub-path steps.
    local baseDrop = math.floor((startHeight - endHeight) / (q - p))
    local numExtraDrops = (startHeight - endHeight) % (q - p)
    local heightDrop = {}
    for i = p, q - 1 do
      heightDrop[i] = baseDrop
    end
    local permutedIndex = permute(p, q - 1)
    for i = p, p + numExtraDrops - 1 do
      heightDrop[permutedIndex[i]] = heightDrop[permutedIndex[i]] + 1
    end
    local currentHeight = startHeight
    for i = p, q - 1 do
      currentHeight = currentHeight - heightDrop[i]
      map[path[i][1]][path[i][2]] = currentHeight
    end

    p = q
  end
end

--[[ Makes the level map.
Arguments:

*   levelSize - Size of the map used to generates paths, the size of the
    ascii map will be levelSize * 2 + 1.
*   numPaths - Number of different paths to be generated to connect player
    position and goal position.
*   maxExtraSteps - The maximum number of extra steps that a generated path can
    have. The number of extra steps of a path is calculated as the
    path's length minus the shortest path's length between player
    and goal.
]]
local function make(levelSize, numPaths, maxExtraSteps)
  -- Initializes the maze which is used to generate random paths.
  local pathMaze =
      maze_generation.mazeGeneration{width = levelSize, height = levelSize}
  for i = 1, levelSize do
    for j = 1, levelSize do
      pathMaze:setEntityCell(i, j, '.')
      pathMaze:setVariationsCell(i, j, '.')
    end
  end

  -- Generates the player and goal position, they should be neither too close
  -- nor too far.
  local playerPos
  local goalPos, distance
  local iters = 0
  repeat
    iters = iters + 1
    assert(iters < MAX_ITERATIONS, "Max iterations!")
    playerPos = randomPlayerPosition(levelSize)
    goalPos = randomGoalPosition(levelSize)
    distance = l1Distance(playerPos, goalPos)
  until distance >= levelSize - 3 and distance <= levelSize - 2

  -- Generates the random paths between player and goal.
  local paths = {}
  for i = 1, numPaths do
    paths[i] =
        generateRandomPath(pathMaze, playerPos, goalPos, maxExtraSteps)
    if not paths[i] then
        break
    end
    for j = 1, #paths[i] do
      pathMaze:setVariationsCell(paths[i][j][1], paths[i][j][2], '#')
    end
  end

  --[[ The actual level maze will double the size of the pathMaze, plus the a
  margin on each side of the area to give space to build invisible walls.
  E.g. if a pathMaze looks like this:
  (P: start, G: goal, 1: first path, 2: second path)
  P2.
  12.
  11G
  The actual maze would look like this:
  .......
  .P22...
  .1.2...
  .1.2...
  .1.2...
  .1111G.
  ....... ]]
  local map = {}
  local mapSize = levelSize * 2 + 1
  for i = 1, mapSize do
    map[i] = {}
    for j = 1, mapSize do
      map[i][j] = 0
    end
  end
  -- Scale the paths to fit the actual size of the level maze.
  local scaledPaths = {}
  for i = 1, #paths do
    local path = {}
    path[1] = {
        paths[i][1][1] * 2,
        paths[i][1][2] * 2,
    }
    for j = 2, #paths[i] do
      path[#path + 1] = {
          paths[i][j - 1][1] + paths[i][j][1],
          paths[i][j - 1][2] + paths[i][j][2],
      }
      path[#path + 1] = {
          paths[i][j][1] * 2,
          paths[i][j][2] * 2,
      }
    end
    scaledPaths[#scaledPaths + 1] = path
  end
  -- Set the height of the start position much higher than others so that the
  -- player can view the whole maze without view being obstructed.
  map[playerPos[1] * 2][playerPos[2] * 2] = MAX_HEIGHT

  -- Convert the height map into an ASCII map.
  local function generateAscii(map)
    local asciiMap = ''
    for i = 1, mapSize do
      for j = 1, mapSize do
        if map[i][j] == 0 then
          asciiMap = asciiMap .. '.'
        else
          asciiMap = asciiMap .. string.char(97 + MAX_HEIGHT - map[i][j])
        end
      end
      asciiMap = asciiMap .. '\n'
    end
    return asciiMap
  end

  generateHeightForMainPath(map, scaledPaths[1])
  for i = 2, #paths do
    generateHeightForSecondaryPath(map, scaledPaths[i])
  end
  return {
      map = generateAscii(map),
      playerPosition = {
          x = playerPos[1] * 2 - 1,
          y = playerPos[2] * 2 - 1
      },
      goalPosition = {
          x = goalPos[1] * 2 - 1,
          y = goalPos[2] * 2 - 1
      },
  }
end

--[[ Makes platforms level.

Arguments:

*   difficulty     - The difficulty of the level, if unset, will be randomly
    drawn from [0, 1).
*   random           - random for randomization.

Returns a table with three keys:

*   map            - The ASCII map of the level.
*   playerPosition - The coordinates of the player spawn position.
*   goalPosition   - The coordinates of the goal position.

The player and goal positions are 0-indexed. ]]
local function makeLevel(difficulty, random)
  rng = random
  difficulty = difficulty or rng:uniformReal(0, 1)
  assert(difficulty >= 0 and difficulty <= 1)
  local levelSize =
      getWeightedValue(MIN_LEVEL_SIZE, MAX_LEVEL_SIZE, difficulty)
  local numPaths =
      getWeightedValue(MIN_PATHS, MAX_PATHS, difficulty)
  local maxExtraSteps = math.floor(levelSize * (0.3 + difficulty))
  -- Ensure that a short path can be stepped off and onto again.
  if maxExtraSteps == 1 then maxExtraSteps = 2 end
  return make(levelSize, numPaths, maxExtraSteps)
end

return {makeLevel = makeLevel}
