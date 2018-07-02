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

local log = require 'common.log'
local asserts = require 'testing.asserts'
local test_runner = require 'testing.test_runner'

local function mockWriter(expectedText)
  local writer = {
    text = '',
    expectedText = false,
    messageWritten = false,
  }

  function writer:write(text)
    if text == expectedText then
      self.expectedText = true
    end
    self.text = self.text .. text
  end

  function writer:flush()
    self.messageWritten = true
  end
  return writer
end

local tests = {}

function tests.info()
  local writer = mockWriter('Hello')
  log.setOutput(writer)
  log.setLogLevel(log.NEVER)
  log.info('Hello')
  assert(not writer.messageWritten and not writer.expectedText)

  log.setLogLevel(log.ERROR)
  log.info('Hello')
  assert(not writer.messageWritten and not writer.expectedText)

  log.setLogLevel(log.WARN)
  log.info('Hello')
  assert(not writer.messageWritten and not writer.expectedText)

  log.setLogLevel(log.INFO)
  log.info('Hello')
  assert(writer.messageWritten and writer.expectedText)

  local writer2 = mockWriter('Hello2')
  log.setOutput(writer2)
  log.setLogLevel(1)
  log.info('Hello2')
  assert(writer2.messageWritten and writer2.expectedText)
end

function tests.warn()
  local writer = mockWriter('Hello')
  log.setOutput(writer)
  log.setLogLevel(log.NEVER)
  log.warn('Hello')
  assert(not writer.messageWritten and not writer.expectedText)

  log.setLogLevel(log.ERROR)
  log.warn('Hello')
  assert(not writer.messageWritten and not writer.expectedText)

  log.setLogLevel(log.WARN)
  log.warn('Hello')
  assert(writer.messageWritten and writer.expectedText)

  local writer2 = mockWriter('Hello2')
  log.setOutput(writer2)
  log.setLogLevel(log.INFO)
  log.warn('Hello2')
  assert(writer2.messageWritten and writer2.expectedText)
end

function tests.error()
  local writer = mockWriter('Hello')
  log.setOutput(writer)
  log.setLogLevel(log.NEVER)
  log.error('Hello')
  assert(not writer.messageWritten and not writer.expectedText)

  log.setLogLevel(log.ERROR)
  log.error('Hello')
  assert(writer.messageWritten and writer.expectedText)

  local writer2 = mockWriter('Hello2')
  log.setOutput(writer2)
  log.setLogLevel(log.WARN)
  log.error('Hello2')
  assert(writer2.messageWritten and writer2.expectedText)
end

function tests.v()
  local writer = mockWriter('Hello')
  log.setOutput(writer)
  log.setLogLevel(log.NEVER)
  log.v(2, 'Hello')
  assert(not writer.messageWritten and not writer.expectedText)

  log.setLogLevel(log.ERROR)
  log.v(2, 'Hello')
  assert(not writer.messageWritten and not writer.expectedText)

  log.setLogLevel(log.WARN)
  log.v(2, 'Hello')
  assert(not writer.messageWritten and not writer.expectedText)

  log.setLogLevel(log.INFO)
  log.v(2, 'Hello')
  assert(not writer.messageWritten and not writer.expectedText)

  log.setLogLevel(1)
  log.v(2, 'Hello')
  assert(not writer.messageWritten and not writer.expectedText)

  log.setLogLevel(2)
  log.v(2, 'Hello')
  assert(writer.messageWritten and writer.expectedText)
end

function tests.infoToString()
  local writer = mockWriter('25')
  log.setOutput(writer)
  log.setLogLevel(log.INFO)
  log.info(25)
  assert(writer.messageWritten and writer.expectedText)
end

return test_runner.run(tests)
