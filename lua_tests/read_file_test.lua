--[[ Copyright (C) 2017-2019 Google Inc.

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
