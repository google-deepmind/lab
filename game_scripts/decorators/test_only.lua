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

local test_only = {}

--[[ Decorate the api so that an error is raised if the environment is not
invoked in an evaluation context (e.g. from the Testbed or a human agent).

This may be used to prevent researchers from accidentally training agents
against evaluation levels that they are not meant to train against.
]]

function test_only.decorate(api)
  local init = api.init
  function api:init(params)
    local holdOutAllowed = params.allowHoldOutLevels == 'true' or
                           params.invocationMode == 'testbed'  -- Legacy

    assert(holdOutAllowed,
           'This level must only be used during evaluation. ' ..
           'For training, use the *_train.lua version of the level. ' ..
           'If you really want to use this level, add the setting ' ..
           '"allowHoldOutLevels" = "true" explicitly')
    return init and init(api, params)
  end
  return api
end

return test_only
