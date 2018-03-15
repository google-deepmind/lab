local game = require 'dmlab.system.game'

local function decorator(api, kwargs)
  local ACTIONS = {
      SELECT_GADGET = 1,
      SWITCH_GADGET = 2,
  }

  local currentActions = {0, 0}

  local modifyControl = api.modifyControl
  function api:modifyControl(control)
    local selectGadget = currentActions[ACTIONS.SELECT_GADGET]
    if selectGadget ~= 0 then
      game:console("weapon " .. selectGadget)
    end
    local switchGadget = currentActions[ACTIONS.SWITCH_GADGET]
    if switchGadget == -1 then
      game:console("weapprev")
    elseif switchGadget == 1 then
      game:console("weapnext")
    end
    -- Prevent gadget switching across multiple frames.
    currentActions = {0, 0}
    return modifyControl and modifyControl(self, control) or nil
  end

  local customDiscreteActionSpec = api.customDiscreteActionSpec
  local startIdx = 0
  function api:customDiscreteActionSpec()
    local customActions = customDiscreteActionSpec and
                          api.customDiscreteActionSpec(self) or {}
    startIdx = #customActions
    if kwargs.gadgetSelect then
      customActions[#customActions + 1] = {
          name = 'SELECT_GADGET',
          min = 0,
          max = 10
      }
    end
    if kwargs.gadgetSwitch then
      customActions[#customActions + 1] = {
          name = 'SWITCH_GADGET',
          min = -1,
          max = 1
      }
    end
    return customActions
  end

  local customDiscreteActions = api.customDiscreteActions
  function api:customDiscreteActions(actions)
    local idx = startIdx
    if kwargs.gadgetSelect then
      idx = idx + 1
      currentActions[ACTIONS.SELECT_GADGET] = actions[idx]
    end
    if kwargs.gadgetSwitch then
      idx = idx + 1
      currentActions[ACTIONS.SWITCH_GADGET] = actions[idx]
    end
    return customDiscreteActions and customDiscreteActions(self, actions)
  end
end

return decorator
