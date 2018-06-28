local asserts = require 'testing.asserts'
local test_runner = require 'testing.test_runner'

local cifar10 = require 'datasets.cifar10'

local tests = {}

function tests.cifar10Train()
  local ds = cifar10{}
  asserts.EQ(50000, ds:getSize())
  local image1 = ds:getImage(1)
  asserts.tablesEQ({32, 32, 3}, image1:shape())
  local imageLast = ds:getImage(ds:getSize())
  asserts.tablesEQ({32, 32, 3}, imageLast:shape())
  asserts.NE(image1, imageLast)
end

function tests.cifar10Test()
  local ds = cifar10{test = true}
  asserts.EQ(10000, ds:getSize())
  local image1 = ds:getImage(1)
  asserts.tablesEQ({32, 32, 3}, image1:shape())
  local imageLast = ds:getImage(ds:getSize())
  asserts.tablesEQ({32, 32, 3}, imageLast:shape())
  asserts.NE(image1, imageLast)
end

return test_runner.run(tests)
