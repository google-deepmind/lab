local asserts = require 'testing.asserts'
local test_runner = require 'testing.test_runner'
local geometric_pickups = require 'common.geometric_pickups'
local random = require 'common.random'

local tests = {}

local CounterMT = {
    add = function(self, k) self[k] = (self[k] or 0) + 1 end
}

local function Counter()
  local counter = {}
  setmetatable(counter, CounterMT)
  return counter
end

function tests.randomPickupsTest()
  random:seed(1)
  local allPickups = geometric_pickups.createPickups()
  for nTypes = 1, #allPickups do
    for nRepeats = 1, 10 do
      local keys = geometric_pickups.randomPickups(allPickups, nTypes, nRepeats)
      asserts.EQ(nTypes * nRepeats, #keys)
      local counter = Counter()
      for _, k in ipairs(keys) do
        counter:add(k)
      end
      for k, count in pairs(counter) do
        asserts.EQ(nRepeats, count)
      end
    end
  end
end

return test_runner.run(tests)
