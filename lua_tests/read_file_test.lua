local game = require 'dmlab.system.game'
local asserts = require 'testing.asserts'
local test_runner = require 'testing.test_runner'
local helpers = require 'common.helpers'

local FILE_NAME = ...
local EMPTY_FILE_NAME = helpers.dirname(FILE_NAME) .. "data/empty_test_file"
local BINARY_FILE_NAME = helpers.dirname(FILE_NAME) .. "data/testL.png"
local tests = {}

local function readFile(file)
  local f = io.open(file, "rb")
  local content = f:read("*all")
  f:close()
  return content
end

function tests:readEmpty()
  local emptyFileContent = game:loadFileToString(EMPTY_FILE_NAME)
  asserts.EQ(emptyFileContent, '')
end

function tests:readBinaryFile()
  local binaryFileContent = game:loadFileToString(BINARY_FILE_NAME)
  assert(binaryFileContent == readFile(BINARY_FILE_NAME))
end

function tests:readSelf()
  local textFileContent = game:loadFileToString(FILE_NAME)
  assert(textFileContent == readFile(FILE_NAME))
end

return test_runner.run(tests)
