-- A user-controlled system for pseudo-random number generation.
-- Helper functions also use this as a source of randomness.

local sys_random = require 'dmlab.system.random'

local random = {}

-- Set the seed of the underlying pseudo-random-bit generator. The argument
-- may be a number or a string representation of a number. Beware of precision
-- loss when using very large numeric values.
--
-- It is probably useful to call this function with the per-episode seed in
-- the "init" callback so that episodes play out reproducibly.
function random.seed(value)
  sys_random:seed(value)
end

-- Returns an integer sampled uniformly at random from the closed range
-- [lower, upper].
function random.uniformInt(lower, upper)
  return sys_random:uniformInt(lower, upper)
end

-- Returns a real number sampled uniformly at random from the half-open range
-- [lower, upper).
function random.uniformReal(lower, upper)
  return sys_random:uniformReal(lower, upper)
end

return random
