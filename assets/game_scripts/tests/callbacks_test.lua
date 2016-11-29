local tensor = require 'dmlab.system.tensor'
local api = {}

api._count = 0

api._observations = {
    LOCATION = tensor.Tensor{10, 20, 30},
    ORDER = tensor.ByteTensor(),
    EPISODE = tensor.Tensor{0},
}

function api:customObservationSpec()
  return {
      {name = 'LOCATION', type = 'Doubles', shape = {3}},
      {name = 'ORDER', type = 'Bytes', shape = {0}},
      {name = 'EPISODE', type = 'Doubles', shape = {1}},
  }
end

function api:init(settings)
  api._settings = settings
  local order = settings.order or ''
  api._observations.ORDER = tensor.ByteTensor{order:byte(1,-1)}
end

function api:customObservation(name)
  return api._observations[name]
end

function api:start(episode, seed)
  api._observations.EPISODE:val(episode)
end

function api:commandLine(oldCommandLine)
  return oldCommandLine .. ' ' .. api._settings.command
end

function api:nextMap()
  api._count = api._count + 1
  return 'lt_chasm_' .. api._count
end

return api
