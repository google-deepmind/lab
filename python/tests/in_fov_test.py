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
"""Tests for InFov observations."""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import os
from absl.testing import absltest
import numpy as np
import six

import deepmind_lab


class InFovTest(absltest.TestCase):

  def test_pickup_raycast(self):
    env = deepmind_lab.Lab(
        'tests/in_fov_test', ['FOVTESTS', 'ENTITYVIS', 'DEBUG.POS.TRANS'],
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

    fov_tests = env.observations()['FOVTESTS']
    self.assertEqual(fov_tests[0], 1)
    self.assertEqual(fov_tests[1], 1)
    self.assertEqual(fov_tests[2], 0)
    entity_vis = env.observations()['ENTITYVIS']
    self.assertEqual(entity_vis[0], 1)
    self.assertLess(entity_vis[1], 1)
    self.assertEqual(entity_vis[2], -1)

    for _ in six.moves.range(120):
      reward = env.step(action, 1)
      if reward > 0:
        self.assertEqual(reward, 1)
        break
    else:
      print(env.observations()['DEBUG.POS.TRANS'])
      self.fail('Failed to pickup 1st apple!')

    fov_tests = env.observations()['FOVTESTS']
    self.assertEqual(fov_tests[0], 1)
    self.assertEqual(fov_tests[1], 0)
    self.assertEqual(fov_tests[2], 0)
    entity_vis = env.observations()['ENTITYVIS']
    self.assertEqual(entity_vis[0], 1)
    self.assertEqual(entity_vis[1], -1)
    self.assertEqual(entity_vis[2], -1)

    action[action_index['STRAFE_LEFT_RIGHT']] = 1
    action[action_index['MOVE_BACK_FORWARD']] = 0

    for _ in six.moves.range(160):
      reward = env.step(action, 1)
      if reward > 0:
        self.assertEqual(reward, 1)
        break
    else:
      print(env.observations()['DEBUG.POS.TRANS'])
      self.fail('Failed to pickup 2st apple!')

    fov_tests = env.observations()['FOVTESTS']
    self.assertEqual(fov_tests[0], 0)
    self.assertEqual(fov_tests[1], 0)
    self.assertEqual(fov_tests[2], 0)
    entity_vis = env.observations()['ENTITYVIS']
    self.assertEqual(entity_vis[0], -1)
    self.assertEqual(entity_vis[1], -1)
    self.assertEqual(entity_vis[2], -1)

    action[action_index['STRAFE_LEFT_RIGHT']] = 0
    action[action_index['MOVE_BACK_FORWARD']] = -1

    for _ in six.moves.range(160):
      reward = env.step(action, 1)
      if reward > 0:
        self.assertEqual(reward, 1)
        break
    else:
      print(env.observations()['DEBUG.POS.TRANS'])
      self.fail('Failed to pickup 3rd apple!')

    fov_tests = env.observations()['FOVTESTS']
    self.assertEqual(fov_tests[0], 1)
    self.assertEqual(fov_tests[1], 1)
    self.assertEqual(fov_tests[2], 0)

if __name__ == '__main__':
  if 'TEST_SRCDIR' in os.environ:
    deepmind_lab.set_runfiles_path(
        os.path.join(os.environ['TEST_SRCDIR'],
                     'org_deepmind_lab'))
  absltest.main()
