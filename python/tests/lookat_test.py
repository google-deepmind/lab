# Copyright 2017-2018 Google Inc.
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
"""Basic test for the random Python agent."""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import os
from absl.testing import absltest
import numpy as np
import six

import deepmind_lab


class LookatTest(absltest.TestCase):

  def test_lookat_run(self):
    kBrushU = 1
    kIsLooking = 3
    env = deepmind_lab.Lab(
        'tests/lookat_test', ['LOOK_AT'],
        config={
            'fps': '60',
            'controls': 'external',
            'width': '32',
            'height': '32'
        })

    env.reset()
    noop = np.array([0, 0, 0, 0, 0, 0, 0], dtype=np.intc)

    last_look_at = None
    for _ in six.moves.range(10):
      self.assertEqual(env.step(noop, 1), 0)
      next_look_at = env.observations()['LOOK_AT']
      self.assertEqual(1, next_look_at[kIsLooking])
      if last_look_at is not None and np.allclose(last_look_at, next_look_at):
        break
      last_look_at = next_look_at
    else:
      self.fail('Too many iterations to stablise!')

    self.assertAlmostEqual(last_look_at[kBrushU], 0.5, delta=0.1)
    look_left = np.array([-5, 0, 0, 0, 0, 0, 0], dtype=np.intc)

    for _ in six.moves.range(100):
      env.step(look_left, 1)
      next_look_at = env.observations()['LOOK_AT']
      if next_look_at[kIsLooking] != 1:
        self.assertAlmostEqual(last_look_at[kBrushU], 1.0, delta=0.1)
        break
      self.assertTrue(next_look_at[kBrushU] >= last_look_at[kBrushU])
      last_look_at = next_look_at
    else:
      self.fail('Failed to look away from object')

    look_right = np.array([5, 0, 0, 0, 0, 0, 0], dtype=np.intc)

    for _ in six.moves.range(5):
      env.step(look_right, 1)
      next_look_at = env.observations()['LOOK_AT']
      if next_look_at[kIsLooking] == 1:
        last_look_at = next_look_at
        break
    else:
      self.fail('Failed to look back to object')

    for _ in six.moves.range(100):
      env.step(look_right, 1)
      next_look_at = env.observations()['LOOK_AT']
      if next_look_at[kIsLooking] != 1:
        self.assertAlmostEqual(last_look_at[kBrushU], 0.0, delta=0.1)
        break
      self.assertTrue(next_look_at[kBrushU] <= last_look_at[kBrushU])
      last_look_at = next_look_at
    else:
      self.fail('Failed to all the way through object')


if __name__ == '__main__':
  if os.environ.get('TEST_SRCDIR'):
    deepmind_lab.set_runfiles_path(
        os.path.join(os.environ['TEST_SRCDIR'],
                     'org_deepmind_lab'))
  absltest.main()
