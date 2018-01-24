-- Tested in deepmind/engine/callbacks_test.cc.
local tensor = require 'dmlab.system.tensor'
local api = {}

api._count = 0

api._observations = {
    LOCATION = tensor.Tensor{10, 20, 30},
    ORDER = tensor.ByteTensor(),
    EPISODE = tensor.Tensor{0},
}

local models = {
    cube = {
        surfaces = {
            cube_surface = {
                vertices = tensor.FloatTensor{
                    { -0.5, -0.5, -0.5,  0.0,  0.0, -1.0, 0.0, 0.0 },
                    {  0.5, -0.5, -0.5,  0.0,  0.0, -1.0, 1.0, 0.0 },
                    {  0.5,  0.5, -0.5,  0.0,  0.0, -1.0, 1.0, 1.0 },
                    { -0.5,  0.5, -0.5,  0.0,  0.0, -1.0, 0.0, 1.0 },
                    {  0.5, -0.5, -0.5,  1.0,  0.0,  0.0, 0.0, 0.0 },
                    {  0.5, -0.5,  0.5,  1.0,  0.0,  0.0, 1.0, 0.0 },
                    {  0.5,  0.5,  0.5,  1.0,  0.0,  0.0, 1.0, 1.0 },
                    {  0.5,  0.5, -0.5,  1.0,  0.0,  0.0, 0.0, 1.0 },
                    {  0.5, -0.5,  0.5,  0.0,  0.0,  1.0, 0.0, 0.0 },
                    { -0.5, -0.5,  0.5,  0.0,  0.0,  1.0, 1.0, 0.0 },
                    { -0.5,  0.5,  0.5,  0.0,  0.0,  1.0, 1.0, 1.0 },
                    {  0.5,  0.5,  0.5,  0.0,  0.0,  1.0, 0.0, 1.0 },
                    { -0.5, -0.5,  0.5, -1.0,  0.0,  0.0, 0.0, 0.0 },
                    { -0.5, -0.5, -0.5, -1.0,  0.0,  0.0, 1.0, 0.0 },
                    { -0.5,  0.5, -0.5, -1.0,  0.0,  0.0, 1.0, 1.0 },
                    { -0.5,  0.5,  0.5, -1.0,  0.0,  0.0, 0.0, 1.0 },
                    { -0.5,  0.5, -0.5,  0.0,  1.0,  0.0, 0.0, 0.0 },
                    {  0.5,  0.5, -0.5,  0.0,  1.0,  0.0, 1.0, 0.0 },
                    {  0.5,  0.5,  0.5,  0.0,  1.0,  0.0, 1.0, 1.0 },
                    { -0.5,  0.5,  0.5,  0.0,  1.0,  0.0, 0.0, 1.0 },
                    {  0.5, -0.5, -0.5,  0.0, -1.0,  0.0, 0.0, 0.0 },
                    { -0.5, -0.5, -0.5,  0.0, -1.0,  0.0, 1.0, 0.0 },
                    { -0.5, -0.5,  0.5,  0.0, -1.0,  0.0, 1.0, 1.0 },
                    {  0.5, -0.5,  0.5,  0.0, -1.0,  0.0, 0.0, 1.0 }
                },
                indices = tensor.Int32Tensor{
                    {  1,  2,  3 },
                    {  1,  3,  4 },
                    {  5,  6,  7 },
                    {  5,  7,  8 },
                    {  9, 10, 11 },
                    {  9, 11, 12 },
                    { 13, 14, 15 },
                    { 13, 15, 16 },
                    { 17, 18, 19 },
                    { 17, 19, 20 },
                    { 21, 22, 23 },
                    { 21, 23, 24 }
                },
                shaderName = 'textures/model/beam'
            }
        }
    }
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
  api._observations.ORDER = tensor.ByteTensor{order:byte(1, -1)}
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

function api:createModel(modelName)
  return models[modelName]
end

return api
