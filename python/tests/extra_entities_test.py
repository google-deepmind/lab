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
"""Tests for Adding extra Entities."""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import os
from absl.testing import absltest
import numpy as np
import six

import deepmind_lab


class ExtraEntitiesTest(absltest.TestCase):

  def test_pickup_apple_run(self):
    env = deepmind_lab.Lab(
        'tests/extra_entities_test', ['DEBUG.POS.TRANS'],
        config={
            'fps': '60',
            'width': '80',
            'height': '80'
        })

    action_spec = env.action_spec()
    action_index = {action['name']: i for i, action in enumerate(action_spec)}

    action = np.zeros([len(action_spec)], dtype=np.intc)

    action[action_index['MOVE_BACK_FORWARD']] = 1

    reward = env.reset()
    expected_reward = 1
    for _ in six.moves.range(60):
      reward = env.step(action, 1)
      if reward > 0:
        self.assertEqual(reward, expected_reward)
        if expected_reward == 5:
          break
        else:
          expected_reward += 1
    else:
      print(env.observations()['DEBUG.POS.TRANS'])
      self.fail('Failed to pickup all apples!')


if __name__ == '__main__':
  if 'TEST_SRCDIR' in os.environ:
    deepmind_lab.set_runfiles_path(
        os.path.join(os.environ['TEST_SRCDIR'],
                     'org_deepmind_lab'))
  absltest.main()
