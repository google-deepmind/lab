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

local custom_entities = require 'common.custom_entities'
local custom_floors = require 'decorators.custom_floors'
local custom_observations = require 'decorators.custom_observations'
local game = require 'dmlab.system.game'
local log = require 'common.log'
local hrp = require 'common.human_recognisable_pickups'
local make_map = require 'common.make_map'
local map_maker = require 'dmlab.system.map_maker'
local maze_generation = require 'dmlab.system.maze_generation'
local object_generator = require 'language.object_generator'
local pickup_decorator = require 'decorators.human_recognisable_pickups'
local pickups = require 'common.pickups'
local PositionTrigger = require 'common.position_trigger'
local random = require 'common.random'
local setting_overrides = require 'decorators.setting_overrides'
local texter = require 'language.texter'
local Timer = require 'common.timer'
local debug_observations = require 'decorators.debug_observations'

local randomMap = random(map_maker:randomGen())

local PICKUP_PREFIX = 'p:'

local factory = {}

-- Removes both leading and trailing whitespace, if any, from str.
local function trimWhitespace(str)
  return str:gsub('^%s*(.-)%s*$', '%1')
end


function factory.defaultInstructor(keys, context, goalGroup)
  local otherGroup = 2
  if goalGroup == 2 then otherGroup = 1 end

  local data = {
      objects = context:getGroups(),
      goal = context:getGroup(goalGroup)[1],
  }

  if data.goal.color == 'black' or data.goal.color == 'white' then
    data.goal.shade = ''
  end

  -- Check that otherGroup actually exists and has data before aliasing
  if otherGroup <= context:groupCount() then
    data.other = context:getGroup(otherGroup)[1]
    if data.other and (data.other.color == 'black' or
                       data.other.color == 'white') then
      data.other.shade = ''
    end
  end

  -- Call keys function if it's a function, or pick just one of the possible
  -- keys if multiple exist.
  local key = keys
  if type(keys) == 'function' then
    key = keys()
  elseif type(keys) == 'table' then
    key = random:choice(keys)
  end

  -- Format instruction text.
  return trimWhitespace(texter.format(key, data))
end


function factory.createLevelApi(kwargs)
  assert(kwargs.episodeLengthSeconds)
  kwargs.useSkybox = kwargs.useSkybox == nil or kwargs.useSkybox
  local api = {}

  -- Called to select the next task.
  api._taskSelector = kwargs.taskSelector
  -- Called to select the next map layout.
  api._levelMapSelector = kwargs.levelMapSelector
  -- Called to transform data into the instruction to present to the agent.
  api._levelInstructor = kwargs.instructor or factory.defaultInstructor
  -- Object responsible for pickup reward logic.
  api._defaultRewardController = kwargs.rewardController
  -- Local map cache.
  api._maps = {}

  local function nameToPickupId(name)
    return tonumber(name:match('^' .. PICKUP_PREFIX .. '(%d+)$'))
  end

  function api:_createPickupFromDescription(desc, group, groupSize)
    local color1 = object_generator.generateRgbColor(desc.color,
                                                     desc.noise,
                                                     desc.shade)
    local color2 = object_generator.generateRgbColor(desc.color2,
                                                     desc.noise,
                                                     desc.shade)

    local id = #self._pickups + 1
    local classname = hrp.create{
        shape = desc.shape,
        color1 = color1,
        color2 = color2,
        pattern = desc.pattern,
        moveType = desc.static and pickups.moveType.STATIC,
        -- The size string is converted to a scale in hrp.
        scale = desc.size,
    }
    -- Annotate the description
    desc._pickupId = id
    desc._group = group
    desc._groupSize = groupSize
    self._pickups[id] = {classname = classname, _desc = desc}
  end

  function api:_createObjects(taskSpec, validRegions)
    hrp.reset()
    self._pickups = {}

    local objectContext = taskSpec.objectContext or self._defaultObjectContext
    objectContext:setChoicesFor('region', validRegions)

    local itemsPerGroup = type(taskSpec.itemsPerGroup) == 'function' and
        taskSpec.itemsPerGroup() or
        taskSpec.itemsPerGroup
    objectContext:processSpecs(taskSpec, itemsPerGroup)

    for groupNum, group in ipairs(objectContext:getGroups()) do
      for _, description in ipairs(group) do
        self:_createPickupFromDescription(description, groupNum, #group)
      end
    end

    return objectContext, itemsPerGroup
  end

  function api:init(params)
    -- If task construction depends on any command-line parameters, it can't
    -- be evaluated until init() time
    assert(self._taskSelector and not kwargs.taskBuilder or kwargs.taskBuilder)
    if kwargs.taskBuilder then
      self._taskSelector = kwargs.taskBuilder(kwargs)
    end
    assert(self._taskSelector)
  end

  function api:start(episode, seed)
    random:seed(seed)
    randomMap:seed(random:themeGenerationSeed())
    self._defaultObjectContext = kwargs.objectContext or
      object_generator.createContext()
    self._timers = Timer.new()
  end

  function api:canPickup(id)
    return self._pickups[id]._desc.canPickup == true
  end

  function api:pickup(id)
    local respawnTime, newInstr = self._rewardController:handlePickup(
                                                       self._pickups[id]._desc)
    if newInstr then
      self.setInstruction(newInstr)
    end
    return respawnTime
  end

  function api:updateSpawnVars(spawnVars)
    if spawnVars.classname == 'info_player_start' then
      -- Check if this spawn location (given by a 'P' on the maze) is rejected.
      if kwargs.playerSpawnValid and not kwargs.playerSpawnValid(spawnVars) then
        return nil
      end
      -- Every spawn location could have a different spawn angle.
      spawnVars.angle = self._playerSpawnAngle()
      spawnVars.randomAngleRange = self._playerSpawnAngleRange
    else
      local id = nameToPickupId(spawnVars.classname)
      if id then
        local actualId = self._mapIdToPickupId[id]
        if actualId and self._pickups[actualId] then
          spawnVars.classname = self._pickups[actualId].classname
          spawnVars.id = tostring(actualId)
        else
          return nil
        end
      end
    end
    return spawnVars
  end

  function api:_makePickup(c)
    if c == 'O' then
      self._objectPlaceholderCount = self._objectPlaceholderCount + 1
      return PICKUP_PREFIX .. self._objectPlaceholderCount
    end
  end

  local function mapRegions(mapSource)
    local regions = {}
    for region, _ in pairs(mapSource.rooms or {}) do
      regions[#regions + 1] = region
    end
    return regions
  end

  local function assignRegionColors(regionColorSelector, regions,
                                    objectDescriptions)
    local regionColors = regionColorSelector{
        regions = regions, objectDescriptions = objectDescriptions}

    for _, region in ipairs(regions) do
      local floorColorName = regionColors[region].floor
      if not floorColorName then
        error("No floor color defined for region " .. region)
      end

      custom_floors.setVariationColor(
          region, object_generator.generateRgbColor(floorColorName))
    end

    -- Set location color information for each object, so it's available for
    -- the instructor's text substitution.
    for _, desc in ipairs(objectDescriptions) do
      if desc.region and desc.region ~= 'any' then
        desc.roomColor = regionColors[desc.region]
      end
    end
  end

  function api:_makeMap(mapInfo)
    assert(mapInfo.entityLayer)
    self._objectPlaceholderCount = 0

    local maze = maze_generation.mazeGeneration{entity = mapInfo.entityLayer}
    local result = {
        maze = maze,
        source = mapInfo,
        generatedMap = make_map.makeMap{
            mapName = mapInfo.name,
            mapEntityLayer = mapInfo.entityLayer,
            mapVariationsLayer = mapInfo.variationsLayer,
            textureSet = mapInfo.textureSet,
            useSkybox = kwargs.useSkybox,
            callback = function (i, j, c, maker)
              local pickup = api:_makePickup(c)
              if pickup then
                return maker:makeEntity{i = i, j = j, classname = pickup}
              end
              if c == 'T' then
                return custom_entities.makeTeleporter(
                    {maze:toWorldPos(i + 1, j + 1)},
                    'teleporter_dest_second_room')
              end
              if c == 't' then
                return custom_entities.makeTeleporterTarget(
                    {maze:toWorldPos(i + 1, j + 1)},
                    'teleporter_dest_second_room')
              end
        end
        }
    }
    result.objectPlaceholderCount = self._objectPlaceholderCount

    return result
  end

  function api:nextMap()
    self._task = self._taskSelector()
    -- Log the task name if present to help with debugging multiple tasks.
    if self._task.name then log.info("Selected task: " .. self._task.name) end

    local mapInfo = self._task.taskMapSelector and self._task.taskMapSelector()
      or api._levelMapSelector()

    local fastReload = kwargs.fastReload and
                       self._maps[mapInfo.name] and
                       mapInfo.name == self._lastMapName
    self._lastMapName = mapInfo.name

    if not self._maps[mapInfo.name] then
      self._maps[mapInfo.name] = self:_makeMap(mapInfo)
    end
    local map = self._maps[mapInfo.name]

    debug_observations.setMaze(map.maze)

    self._objectPlaceholderCount = map.objectPlaceholderCount

    -- Add a level of indirection so map position i doesn't have to always
    -- correspond to pickup i.
    self._mapIdToPickupId = {}
    for i = 1, self._objectPlaceholderCount do self._mapIdToPickupId[i] = i end

    -- Objects may care about where they're placed in the map - tell the
    -- generator what regions/rooms/variations are valid for the chosen map.
    local regions = mapRegions(map.source)
    local objectContext, itemsPerGroup = self:_createObjects(self._task,
                                                             regions)

    local descriptions = {}
    for _, pickup in ipairs(self._pickups) do
      descriptions[#descriptions + 1] = pickup._desc
    end
    local placer = self._task.objectPlacer or kwargs.objectPlacer
    placer{mapIdToPickupId = self._mapIdToPickupId,
           map = map,
           objectDescriptions = descriptions}

    -- Associate colors with regions if requested.
    local regionColorSelector = self._task.regionColorSelector or
        kwargs.levelRegionColorSelector
    if regionColorSelector and next(regions) then
      assignRegionColors(regionColorSelector, regions, descriptions)
    end

    local taskData = {
        descriptions = descriptions,
        objectsPerGroup = itemsPerGroup,
    }

    self._rewardController = self._task.rewardController or
      self._defaultRewardController
    self._rewardController:init(self._task, taskData)

    -- Finally generate the language instruction:
    local instructionGenerator = self._task.instructor or self._levelInstructor
    local instruction = instructionGenerator(self._task.key, objectContext,
                                             self._task.goalGroup or 1)
    log.info('Instruction: ' .. instruction)
    self.setInstruction(instruction)

    self._playerSpawnAngle = function()
      local playerSpawnAngle = self._task.playerSpawnAngle or
                               kwargs.playerSpawnAngle
      return playerSpawnAngle == nil and '0' or
                 tostring(tonumber(playerSpawnAngle) or playerSpawnAngle())
    end
    self._playerSpawnAngleRange = tostring(self._task.playerSpawnAngleRange or
                                               kwargs.playerSpawnAngleRange or
                                               '0')

    -- Maintain the current time in the timer triggers.
    self._timers:removeAll()
    -- Reset the current position in the position triggers.
    self._positionTriggers = PositionTrigger.new()

    -- Set up time and position triggers.
    if self._task.triggers then
      for i, trigger in ipairs(self._task.triggers) do
        assert(trigger.time or trigger.triggerWhenEnter or
               trigger.triggerWhenExit)
        assert(trigger.key or trigger.reward or trigger.callback)

        -- Callbacks are the same for all triggers.
        local function callbackForTrigger()
            local persist = trigger.persist

            -- Update the instruction with the trigger's `key`.
            if trigger.key then
              local instruction = instructionGenerator(
                  trigger.key, objectContext, self._task.goalGroup or 1)
              self.setInstruction(instruction)
            end

            -- Update the game reward.
            if trigger.reward then
              game:addScore(trigger.reward)
            end

            -- Call the trigger's callback, if it exists.
            if trigger.callback then
              persist = trigger.callback(trigger)
            end

            -- By default (i.e. if `persist` is nil) trigger will be removed.
            -- If `true`, trigger will be reset to fire again.
            return persist
        end

        if trigger.time then
          self._timers:start{
              name = 'language timer ' .. tostring(i),
              time = trigger.time,
              callback = callbackForTrigger,
          }
        else
          local maze = trigger.maze or mapInfo.variationsLayer or
                       mapInfo.entityLayer
          assert(maze)
          self._positionTriggers:start{
              name = 'language position trigger ' .. tostring(i),
              maze = maze,
              triggerWhenEnter = trigger.triggerWhenEnter,
              triggerWhenExit = trigger.triggerWhenExit,
              callback = callbackForTrigger,
          }
        end
      end
    end

    return fastReload and '' or map.generatedMap
  end

  function api:hasEpisodeFinished(timeSeconds)
    self._timers:update(timeSeconds)
    self._positionTriggers:update(game:playerInfo().pos)
    if kwargs.hasEpisodeFinished then
      return kwargs.hasEpisodeFinished(timeSeconds)
    end
  end

  custom_floors.decorate(api)
  custom_observations.decorate(api)
  pickup_decorator.decorate(api)
  setting_overrides.decorate{
      api = api,
      apiParams = kwargs,
      decorateWithTimeout = true
  }
  return api
end

return factory
