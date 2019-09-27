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

local random = require 'common.random'

local staircase = {}

--[[ Function to create the one-dimensional adaptive staircase object.
At difficulty level K, a test consists of K trials. If the proportion of
correct trials in a test is greater or equal than opt.fractionToPromote
then it promotes to level K+1. If the proportion is less or equal than
opt.fractionToDemote then it demotes to level K-1. Otherwise it repeats
level K.

Arguments:

`sequence`: an array of difficulty parameter values, ordered by difficulty

`correctRewardSequence`: (number or array). If a number always return that
number when staircase:reward() is called. If an array, then calling
staircase:correctReward() will return
correctRewardSequence[currentDifficultyLevel]. If correctRewardSequence is a
table then it must have the same number of elements as `sequence`.

`fractionToPromote`: proportion of K trials in level K that must have been
correct in order to advance to the next level.

`fractionToDemote`: proportion of K trials in level K, if score is less than
this value, demote difficulty back to the previous level.

`probeProbability`: probability of displaying a probe trial, i.e. a trial
with difficulty randomly selected between 1 and the current level.
]]
function staircase.createStaircase1D(opt)
  if type(opt.correctRewardSequence) == 'table' then
    assert(#opt.correctRewardSequence == #opt.sequence)
  end
  local staircase1D = {
      _fractionToPromote = opt.fractionToPromote,
      _fractionToDemote = opt.fractionToDemote,
      _probeProbability = opt.probeProbability,
      _sequence = opt.sequence,
      _correctRewardSequence = opt.correctRewardSequence,
      _difficultyLevel = opt.initialDifficultyLevel or 1,
      _trialCount = 0,
      _correctCount = 0,
      _probeTrial = false,
      -- set to number of trials to enable fixed test length.
      _fixedTestLength = opt.fixedTestLength or false,
  }
  if staircase1D._fixedTestLength then
    staircase1D._testLength = staircase1D._fixedTestLength
  else
    staircase1D._testLength = staircase1D._difficultyLevel
  end

  -- 'staircase1D.step' is called at the end of each trial.
  function staircase1D:step(isCorrect)
    assert(type(isCorrect) == "boolean")
    if not self._probeTrial then
      self._trialCount = self._trialCount + 1
      self._correctCount = self._correctCount + (isCorrect and 1 or 0)
      if self._trialCount == self._testLength then
        local fractionCorrect = self._correctCount / self._trialCount
        if fractionCorrect >= self._fractionToPromote then
          if self._difficultyLevel < #self._sequence then
            self._difficultyLevel = self._difficultyLevel + 1
          end
        elseif fractionCorrect <= self._fractionToDemote then
          if self._difficultyLevel > 1 then
            self._difficultyLevel = self._difficultyLevel - 1
          end
        end
        self._trialCount = 0
        self._correctCount = 0
        if not self._fixedTestLength then
          self._testLength = self._difficultyLevel
        end
      end
    end
  end

  function staircase1D:parameter()
    if random:uniformReal(0, 1) < self._probeProbability then
      self._probeTrial = true
      return self._sequence[random:uniformInt(1, self._difficultyLevel)]
    else
      self._probeTrial = false
      return self._sequence[self._difficultyLevel]
    end
  end

  function staircase1D:getDifficultyLevel()
    return self._difficultyLevel
  end

  function staircase1D:correctReward()
    if type(self._correctRewardSequence) == 'number' then
      return self._correctRewardSequence
    elseif type(self._correctRewardSequence) == 'table' then
      return self._correctRewardSequence[self._difficultyLevel]
    end
  end

  return staircase1D
end

return staircase
