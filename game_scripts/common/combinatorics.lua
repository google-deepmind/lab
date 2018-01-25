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

-- Utility functions for combinatorics operations.
-- See https://en.wikipedia.org/wiki/Combinatorics

local combinatorics = {}

--[[ Returns "n choose k" = n! / (k! * (n - k)!).

This is the number of unique ways to sample without replacement k elements from
a bucket of n elements.

See https://en.wikipedia.org/wiki/Combination.
--]]
function combinatorics.choose(k, n)
  assert(0 <= k and k <= n)

  --[[ We can repeatedly use the identity

         C(n, k) = n / k * C(n - 1, k - 1)

       to calculate C(n, k) while avoiding large numbers (and corresponding
       floating point error).

         C(n, k) = product[i = 0 .. k - 1] (n - i) / (k - i) * C(n - k, 0)
                 = product[i = 0 .. k - 1] (n - i) / (k - i)

       Moreover, since C(n, k) = C(n, n - k), we can ensure we loop at most n/2
       times by setting k' = min(k, n-k). Then:

         C(n, k) = C(n, k')
  --]]
  local kPrime = math.min(k, n - k)
  local accumulator = 1
  for i = 0, kPrime - 1 do
    accumulator = accumulator * ((n - i) / (kPrime - i))
  end
  return accumulator
end

--[[ Return the `idx`th selection from an enumeration of all possible
selections {a, b} with 1 <= a < b <= n.

`idx` is a number between 1 and C(n, 2) = n * (n - 1) / 2.

Given n items, there are C(n, 2) = n * (n - 1) / 2 ways to select two of those
items. We enumerate all those ways and assign each an index from 1 to C(n, 2).
We return the `idx`th pair from that enumeration.
--]]
function combinatorics.twoItemSelection(idx, n)
  assert(1 <= idx and idx <= combinatorics.choose(2, n))

  --[[
  First, enumerate possible combinatorics of {x,y} with 0 <= x <= y <= n-2 by
  drawing them in the triangular configuration below.

  Then, use triangular numbers to map from i = idx-1 to {x,y} coordinates
  (see https://en.wikipedia.org/wiki/Triangular_number).

       0 1 2 3 . . n-2
    0  *
    1  * *
    2  * * *
    .  * * * *
    .  * * i * * --- x     <== The `i`th triangular point.
    .  * * * * * *             We map it to coordinates {x, y}.
  n-2  * * * * * * *
           |
           y

  If R is the number of points in first r rows, then
    R = 1 + 2 + 3 + ... + r
    R = r * (r + 1) / 2
    0 = r^2 + r - 2R
    r = [-1 + sqrt(1 - 4*1*(-2R))] / 2, by quadratic formula,
                                        and since r must be positive
    r = [-1 + sqrt(1 + 8R)] / 2
    r = -0.5 + sqrt(0.25 + 2R)
  This is the "triangular root" of x, which is monotonically increasing,
  so the floor of it will give us the closest row.

    x = floor(-0.5 + sqrt(2 * i + 0.25))

  The column is given by subtracting the number of points in the first
  x rows from i.

    y = i - [x * (x + 1) / 2]
  --]]
  local i = idx - 1
  local x = math.floor(-0.5 + math.sqrt(0.25 + 2 * i))
  local y = i - x * (x + 1) / 2

  -- We want {a, b} where 1 <= a < b <= n, so we need to adjust the {x,y}
  -- indices to match the coordinate system below.
  --
  -- Note that `a` runs over 1..n-1, and `b` runs over 2..n because a < b.
  --
  --       n . . . 4 3 2
  --   n-1 *
  --    .  * *
  --    .  * * *
  --    .  * * * *
  --    3  * *idx* * --- a     <== The `idx`th triangular point.
  --    2  * * * * * *             We map it to coordinates {a, b}.
  --    1  * * * * * * *
  --           |
  --           b
  --
  local a = n - x - 1
  local b = n - y
  return {a, b}
end

return combinatorics
