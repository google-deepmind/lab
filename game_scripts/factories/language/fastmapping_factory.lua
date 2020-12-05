--[[ Copyright (C) 2018-2020 Google Inc.

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

local factory = require 'factories.language.factory'
local game = require 'dmlab.system.game'
local lcm = require 'language.make'
local maze_generation = require 'dmlab.system.maze_generation'
local object_generator = require 'language.object_generator'
local placers = require 'language.placers'
local random = require 'common.random'
local selectors = require 'language.selectors'
local texter = require 'language.texter'


local fastmapping = {}
local confoundNames
local confound = {}


local function _map(objectCount, distractorObjectCount)
  local totalObjectCount = objectCount + distractorObjectCount
  local mazeSize = {
    width = objectCount * 2 + 3,
    height = totalObjectCount * 4 + 6}
  local maze = maze_generation:mazeGeneration(mazeSize)
  local a = string.byte('A')

  for i = 2, mazeSize.height - 1 do
    for j = 2, mazeSize.width - 1 do
      maze:setEntityCell(i, j, '.')
    end
  end

  -- Player spawns at centre top.
  local centerJ = (mazeSize.width + 1) / 2
  maze:setEntityCell(2, centerJ, 'P')

  -- Objects for learning mapping are positioned in narrow opening in walls.
  for object = 1, totalObjectCount do
    local objectRow = (object - 1) * 4 + 5
    for j = 1, mazeSize.width do
      maze:setEntityCell(objectRow, j, '*')
    end
    maze:setEntityCell(objectRow, centerJ, 'O')
  end

  -- Objects for demonstrating mapping are spread across final row.
  for object = 1, objectCount do
    local lastObjectRow = mazeSize.height - 1
    maze:setEntityCell(lastObjectRow, (object - 1) * 2 + 3, 'O')
  end

  return maze
end


function fastmapping.defaultInstructor(key, context, goalGroup)
  local data = {
      objects = context:getGroups(),
  }
  return confound[texter.format(key, data)]
end


--[[ Produce maps of the pattern:

With distractorObjectCount = 0:

objectCount = 1, distractorObjectCount = 0:
*****
*.P.*
*...*
*...*
**O**
*...*
*...*
*...*
*.O.*
*****

objectCount = 2, distractorObjectCount = 0:
*******
*..P..*
*.....*
*.....*
***O***
*.....*
*.....*
*.....*
***O***
*.....*
*.....*
*.....*
*.O.O.*
*******

objectCount = 3, distractorObjectCount = 0:
*********
*...P...*
*.......*
*.......*
****O****
*.......*
*.......*
*.......*
****O****
*.......*
*.......*
*.......*
****O****
*.......*
*.......*
*.......*
*.O.O.O.*
*********

With distractorObjectCount = 1:

objectCount = 1, distractorObjectCount = 1:
*****
*.P.*
*...*
*...*
**O**
*...*
*...*
*...*
**O**
*...*
*...*
*...*
*.O.*
*****

objectCount = 2, distractorObjectCount = 1:
*******
*..P..*
*.....*
*.....*
***O***
*.....*
*.....*
*.....*
***O***
*.....*
*.....*
*.....*
***O***
*.....*
*.....*
*.....*
*.O.O.*
*******

objectCount = 3, distractorObjectCount = 1:
*********
*...P...*
*.......*
*.......*
****O****
*.......*
*.......*
*.......*
****O****
*.......*
*.......*
*.......*
****O****
*.......*
*.......*
*.......*
****O****
*.......*
*.......*
*.......*
*.O.O.O.*
*********

Further increases to distractorObjectCount add more rooms in the same way.
--]]
function fastmapping.buildMap(objectCount, distractorObjectCount)
  local maze = _map(objectCount, distractorObjectCount)
  local fastMap = {
      name = 'fastmapping',
      entityLayer = maze:entityLayer(),
      variationsLayer = maze:variationsLayer(),
      rooms = {}
  }
  local totalObjectCount = objectCount + distractorObjectCount
  fastMap.rooms[tostring(totalObjectCount + 1)] = {}
  for i = 1, totalObjectCount do
    fastMap.rooms[tostring(i)] = {}
    fastMap.rooms[tostring(i)][i] = true
    if i <= objectCount then
      fastMap.rooms[tostring(totalObjectCount + 1)][totalObjectCount + i] = true
    end
  end

  return fastMap
end


--[[ Create a reward controller for fast mapping.

Does not use task.reward; hardcodes learning samples to +learningReward and
goals to +/-goalReward.
Sets up _descriptions so we can work around the instructor.
Sets up confound: if confoundNames is true, remaps names of objects.
--]]
local function _createController(kwargs, objectCount, distractorObjectCount)
  local controller = {}
  local totalObjectCount = objectCount + distractorObjectCount

  function controller:init(task, taskData)
    self._goalObject = random:uniformInt(1, objectCount)
    self._goalGroup = totalObjectCount + self._goalObject

    -- If confounding is turned off, this name remapping is identity.
    local reorderedConfound = confoundNames and
        random:shuffle(kwargs.confounderShapes) or kwargs.goalShapes
    confound = {}
    for i = 1, #kwargs.goalShapes do
      confound[kwargs.goalShapes[i]] = reorderedConfound[i]
    end

    -- task[*].description.shape aren't strings becase we're using lcm.
    self._descriptions = {}
    for i = 1, totalObjectCount do
      self._descriptions[i] = confound[taskData.descriptions[i].shape]
    end
    self._descriptions[totalObjectCount + 1] =
        confound[taskData.descriptions[self._goalGroup].shape]
  end

  function controller:handlePickup(description)
    local reward
    local terminate = false
    local newInstruction

    if description._group <= totalObjectCount then
      reward = kwargs.learningReward
      -- Assumes we walk through the objects in sequence, and the N+1'th
      -- key is our goal object.
      -- Has to ape instructor directly, since the factory only
      -- invokes its instructionGenerator on the initial instruction and we
      -- can't call the instructor without context.
      newInstruction = self._descriptions[description._group + 1]
    elseif description._group == self._goalGroup then
      reward = kwargs.goalReward
      terminate = true
    else
      reward = -kwargs.goalReward
      terminate = true
    end

    if terminate then
      game:finishMap()
    end
    game:addScore(reward)
    return -1, newInstruction
  end

  return controller
end

--[[ Create a language level where the agent is given names for several
types objects, then must collect one of those objects.

Arguments:

    * confoundNames -- true if novel names should be given to each type of
      object in every episode, false if consistent names should be used.
    * confounderShapes -- list of strings to substitute for names of shapes.
    * episodeLengthSeconds -- how long an episode lasts, in seconds.
    * goalReward -- points awarded for correct goal and deducted for
      incorrect goal; default 7.
    * goalShapes -- list of names of shapes (object models).
    * learningReward -- points awarded for each object visited during learning,
      default 0. In early prototypes this was 5, but that would create a large
      positive reward from incorrect behavior.
    * objectCount -- how many types of objects should be presented that will
      be chosen from at the end.
    * distractorObjectCount -- how many types of objects should be presented
      after the intitial objectCount have been presented. These objects will
      not be one of the objects to choose from in the final room.
--]]
function fastmapping.createLevelApi(kwargs)
  assert(kwargs.confounderShapes or not kwargs.confoundNames)
  assert(kwargs.episodeLengthSeconds)
  assert(kwargs.goalShapes)

  confoundNames = kwargs.confoundNames or false
  kwargs.goalReward = kwargs.goalReward or 7
  kwargs.learningReward = kwargs.learningReward or 0
  local objectCount = kwargs.objectCount or 2
  local distractorObjectCount = kwargs.distractorObjectCount or 0
  local totalObjectCount = objectCount + distractorObjectCount
  assert(distractorObjectCount >= 0)
  assert(not confoundNames or objectCount <= #kwargs.confounderShapes)
  assert(totalObjectCount <= #kwargs.goalShapes)

  local fastMap = fastmapping.buildMap(objectCount, distractorObjectCount)
  local task = {
      itemsPerGroup = {}
  }
  local groups = {}

  -- The initial rooms each have one random object, different from all
  -- previously-seen objects.
  task[1] = {
      region = "1"
  }
  task.itemsPerGroup[1] = 1

  for i = 2, totalObjectCount do
    -- The ith object must be distinct from objects 1, 2, ... i-1.
    table.insert(groups, i - 1)
    task[i] = {
        shape = lcm.differentTo{unpack(groups)},
        region = tostring(i)
    }
    task.itemsPerGroup[i] = 1
  end

  -- The final room has one of each non-distractor object.
  for i = 1, objectCount do
    task[totalObjectCount + i] = {
        shape = lcm.sameAs(i),
        region = tostring(totalObjectCount + 1),
        goalObject = true
    }
    task.itemsPerGroup[totalObjectCount + i] = 1
  end

  -- The initial instruction always names the first object.
  task.key = "[objects.1.1.shape]"
  task.objectPlacer = placers.createRoom()
  task.playerSpawnAngle = -90
  task.rewardController = _createController(
    kwargs, objectCount, distractorObjectCount)

  return factory.createLevelApi{
      episodeLengthSeconds = kwargs.episodeLengthSeconds,
      instructor = fastmapping.defaultInstructor,
      levelMapSelector = selectors.createIdentity(fastMap),
      objectContext = object_generator.createContext{
          attributes = {
              shape = kwargs.goalShapes,
              pattern = {'solid'}
          }
      },
      taskSelector = selectors.createIdentity(task),
  }
end

return fastmapping
