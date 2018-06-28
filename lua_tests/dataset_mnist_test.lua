local asserts = require 'testing.asserts'
local test_runner = require 'testing.test_runner'

local mnist = require 'datasets.mnist'

local tests = {}

function tests.mnistTrain()
  local ds = mnist{}
  asserts.EQ(60000, ds:getSize())

  local image1 = ds:getImage(1)
  asserts.tablesEQ({28, 28, 3}, image1:shape())
  local imageLast = ds:getImage(ds:getSize())
  asserts.tablesEQ({28, 28, 3}, imageLast:shape())
  asserts.NE(image1, imageLast)

  local label1 = ds:getLabel(1)
  local labelLast = ds:getLabel(ds:getSize())
  asserts.NE(label1, labelLast)
end

function tests.mnistTest()
  local ds = mnist{test = true}
  asserts.EQ(10000, ds:getSize())
  local image1 = ds:getImage(1)
  asserts.tablesEQ({28, 28, 3}, image1:shape())
  local imageLast = ds:getImage(ds:getSize())
  asserts.tablesEQ({28, 28, 3}, imageLast:shape())
  asserts.NE(image1, imageLast)

  local label1 = ds:getLabel(1)
  local labelLast = ds:getLabel(ds:getSize())
  asserts.NE(label1, labelLast)
end

return test_runner.run(tests)
