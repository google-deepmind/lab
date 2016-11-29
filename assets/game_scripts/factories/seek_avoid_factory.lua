local game = require 'dmlab.system.game'
local random = require 'common.random'
local pickups = require 'common.pickups'
local helpers = require 'common.helpers'
local custom_observations = require 'decorators.custom_observations'
local timeout = require 'decorators.timeout'

local factory = {}

--[[ Creates a Seek Avoid API.
Keyword arguments:

*   `mapName` (string) - Name of map to load.
*   `episodeLengthSeconds` (number) - Episode length in seconds.
]]
function factory.createLevelApi(kwargs)
  assert(kwargs.mapName and kwargs.episodeLengthSeconds)

  local api = {}

  function api:createPickup(class_name)
    return pickups.defaults[class_name]
  end

  function api:start(episode, seed, params)
    random.seed(seed)
    api._has_goal = false
    api._count = 0
    api._finish_count = 0
  end

  function api:pickup(spawn_id)
    api._count = api._count + 1
    if not api._has_goal and api._count == api._finish_count then
      game:finishMap()
    end
  end

  function api:updateSpawnVars(spawnVars)
    local classname = spawnVars.classname
    if spawnVars.random_items then
      local possibleClassNames = helpers.split(spawnVars.random_items, ',')
      if #possibleClassNames > 0 then
        classname = possibleClassNames[
          random.uniformInt(1,  #possibleClassNames)]
      end
    end
    local pickup = pickups.defaults[spawnVars.classname]
    if pickup then
      if pickup.type == pickups.type.kReward and pickup.quantity > 0 then
        api._finish_count = api._finish_count + 1
        spawnVars.id = tostring(api._finish_count)
      end
      if pickup.type == pickups.type.kGoal then
        api._has_goal = true
      end
    end

    return spawnVars
  end

  function api:nextMap()
    return kwargs.mapName
  end

  custom_observations.decorate(api)
  timeout.decorate(api, kwargs.episodeLengthSeconds)
  return api
end

return factory
