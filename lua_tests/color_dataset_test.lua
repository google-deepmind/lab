local asserts = require 'testing.asserts'
local test_runner = require 'testing.test_runner'

local color_dataset = require 'datasets.color_dataset'
local selector = require 'datasets.selector'

local tests = {}

function tests.colours()
  local ds = color_dataset(4, 4, 5000)
  asserts.EQ(5000, ds:getSize())
  local image1 = ds:getImage(1)
  asserts.tablesEQ({4, 4, 3}, image1:shape())
  local imageLast = ds:getImage(ds:getSize())
  asserts.tablesEQ({4, 4, 3}, imageLast:shape())
  asserts.NE(image1, imageLast)
end

function tests.selector()
  local ds = selector.loadDataset('color')
  asserts.EQ(1500, ds:getSize())
  local image1 = ds:getImage(1)
  asserts.tablesEQ({8, 8, 3}, image1:shape())
  local imageLast = ds:getImage(ds:getSize())
  asserts.tablesEQ({8, 8, 3}, imageLast:shape())
  asserts.NE(image1, imageLast)
end

return test_runner.run(tests)
