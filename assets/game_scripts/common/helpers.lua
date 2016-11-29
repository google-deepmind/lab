local random = require 'common.random'
-- Common utilities.

local helpers = {}
-- Shuffles an array in place. (Uses the 'common.random'.)
function helpers.shuffleInPlace(array)
  for i = 1, #array - 1 do
    local j = random.uniformInt(i, #array)
    array[j], array[i] = array[i], array[j]
  end

  return array
end

-- Returns a shuffled copy of an array. (Uses the 'common.random'.)
function helpers.shuffle(array)
  local ret = {}
  for i, obj in ipairs(array) do
    ret[i] = obj
  end
  return helpers.shuffleInPlace(ret)
end

-- Returns an array of strings split according to single character separator.
-- Skips empty fields.
function helpers.split(str, sep)
  words = {}
  for word in string.gmatch(str, '([^' .. sep .. ']+)') do
      words[#words + 1] = word
  end
  return words
end

return helpers
