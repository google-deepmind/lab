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
"""Tests for inventory modification API."""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import os
from absl.testing import absltest
import numpy as np
import six

import deepmind_lab


class ScoreEventTest(absltest.TestCase):

  def test_self_orb(self):
    env = deepmind_lab.Lab(
        'tests/spawn_inventory_test', ['DEBUG.POS.ROT'],
        config={
            'fps': '60',
            'width': '80',
            'height': '80'
        })

    action_spec = env.action_spec()
    action_index = {action['name']: i for i, action in enumerate(action_spec)}

    action = np.zeros([len(action_spec)], dtype=np.intc)

    action[action_index['LOOK_DOWN_UP_PIXELS_PER_FRAME']] = 100

    reward = env.reset()
    for _ in six.moves.range(60):
      env.step(action, 1)
      if env.observations()['DEBUG.POS.ROT'][0] > 85:
        break
    else:
      self.fail('Failed to look at floor!')

    action[action_index['LOOK_DOWN_UP_PIXELS_PER_FRAME']] = 0
    action[action_index['FIRE']] = 1

    for _ in six.moves.range(600):
      reward = env.step(action, 1)
      if reward < 0:
        self.assertEqual(reward, -1)
        break
    else:
      self.fail('Failed to orb floor!')


if __name__ == '__main__':
  if 'TEST_SRCDIR' in os.environ:
    deepmind_lab.set_runfiles_path(
        os.path.join(os.environ['TEST_SRCDIR'],
                     'org_deepmind_lab'))
  absltest.main()
