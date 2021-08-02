# Copyright 2018 Google Inc.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import os
from absl.testing import absltest
import numpy as np
import six

import deepmind_lab

MAZE_LAYOUT_OBSERVATION = 'DEBUG.MAZE.LAYOUT'
MAZE_LAYOUT_TRIALS = 50


class MazeGenerationTest(absltest.TestCase):

  def test_maze_layout_spread(self):
    layouts = set()
    for i in six.moves.range(MAZE_LAYOUT_TRIALS):
      print('phase 1: trial {} out of {}'.format(i+1, MAZE_LAYOUT_TRIALS))
      env = deepmind_lab.Lab(
          'tests/maze_generation_test', [MAZE_LAYOUT_OBSERVATION], config={})
      env.reset(seed=i+1)
      layouts.add(env.observations()[MAZE_LAYOUT_OBSERVATION])
    num_layouts = len(layouts)
    self.assertEqual(num_layouts, MAZE_LAYOUT_TRIALS)
    for i in six.moves.range(MAZE_LAYOUT_TRIALS):
      print('phase 2: trial {} out of {}'.format(i+1, MAZE_LAYOUT_TRIALS))
      env = deepmind_lab.Lab(
          'tests/maze_generation_test', [MAZE_LAYOUT_OBSERVATION],
          config={
              'mixerSeed': '0',
          })
      env.reset(seed=i+1)
      layouts.add(env.observations()[MAZE_LAYOUT_OBSERVATION])
    self.assertLen(layouts, num_layouts)
    for i in six.moves.range(MAZE_LAYOUT_TRIALS):
      print('phase 3: trial {} out of {}'.format(i+1, MAZE_LAYOUT_TRIALS))
      env = deepmind_lab.Lab(
          'tests/maze_generation_test', [MAZE_LAYOUT_OBSERVATION],
          config={
              'mixerSeed': '1',
          })
      env.reset(seed=i+1)
      layouts.add(env.observations()[MAZE_LAYOUT_OBSERVATION])
    self.assertLen(layouts - num_layouts, MAZE_LAYOUT_TRIALS)

if __name__ == '__main__':
  if 'TEST_SRCDIR' in os.environ:
    deepmind_lab.set_runfiles_path(
        os.path.join(os.environ['TEST_SRCDIR'],
                     'org_deepmind_lab'))
  absltest.main()
