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

local texter = {}

--[[ Recursively decode dot separated key names against a table of values.

See also `format` method for a higher level interface.

Example:

    data = {
        count = 2,
        pets = {
            { name = {first = 'Ajax', last = 'The Cat' } },
            { name = {first = 'Fido', last = 'The Dog' } }
        }
    }

    assert(decode('count', data) == 2)
    assert(decode('pets.2.name.first', data) == 'Fido')

Arguments:

*   `key` (string) is a table key, delimited with dots for nested keys.
*   `data` (table) is the table to apply the key lookups to.

Returns value from  data table.
]]
function texter.decode(key, data)
  for key in string.gmatch(key, '[^.]+') do
    data = data[key] or data[tonumber(key)]
  end
  return data
end

--[[ Format a string, decoding values in square brackets against a table.

Example:

    data = {
        count = 2,
        pets = {
            { name = {first = 'Ajax', last = 'The Cat' } },
            { name = {first = 'Fido', last = 'The Dog' } }
        }
    }

    assert(format('[count] pets', data) == '2 pets')
    assert(format('Say hi to [pets.1.name.first] [pets.1.name.last]') ==
           'Say hi to Ajax The Cat')

Arguments:

*   `str` (string) format string to scan for [values.to.decode]
*   `data` (table) is the table to apply the decode lookups to.

Returns string with values replaced.
]]
function texter.format(str, data)
  local replacer = function(s)
    return texter.decode(s:sub(2, s:len() - 1), data)
  end
  local text = str:gsub('%b[]', replacer)
  return text
end

return texter
