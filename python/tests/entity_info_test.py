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
"""Test for the Entity Info."""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import os
from absl.testing import absltest
import numpy as np
import six

import deepmind_lab


class EntityInfo(absltest.TestCase):

  def test_reward_info(self):
    env = deepmind_lab.Lab(
        'tests/entity_info_test', ['DEBUG.PICKUPS'],
        config={
            'fps': '60',
            'width': '32',
            'height': '32'
        })

    action_spec = env.action_spec()
    action_index = {action['name']: i for i, action in enumerate(action_spec)}

    env.reset()
    obs = env.observations()
    pickups_state = np.array([[150., 50., 16.125, 1., 1.],
                              [250., 50., 16.125, 1., 1.],
                              [350., 50., 16.125, 1., 1.],
                              [450., 50., 16.125, 1., -1.],
                              [550., 50., 16.125, 1., -1.],
                              [650., 50., 16.125, 1., -1.]])
    self.assertTrue(np.allclose(obs['DEBUG.PICKUPS'], pickups_state))

    action = np.zeros([len(action_spec)], dtype=np.intc)
    action[action_index['MOVE_BACK_FORWARD']] = 1
    reward = 0
    for _ in six.moves.range(0, 60):
      reward += env.step(action, 1)
      if reward == 3:
        break
    else:
      self.fail('Failed to all positive rewards.')

    pickups_state = np.array([[150., 50., 16.125, 0., 1.],
                              [250., 50., 16.125, 0., 1.],
                              [350., 50., 16.125, 0., 1.],
                              [450., 50., 16.125, 1., -1.],
                              [550., 50., 16.125, 1., -1.],
                              [650., 50., 16.125, 1., -1.]])
    obs = env.observations()
    self.assertTrue(np.allclose(obs['DEBUG.PICKUPS'], pickups_state))
    for _ in six.moves.range(0, 600):
      reward += env.step(action, 1)
      if reward == 0:
        break
    else:
      self.fail('Failed to pickup last negative reward')

    pickups_state = np.array([[150., 50., 16.125, 0., 1.],
                              [250., 50., 16.125, 0., 1.],
                              [350., 50., 16.125, 0., 1.],
                              [450., 50., 16.125, 0., -1.],
                              [550., 50., 16.125, 0., -1.],
                              [650., 50., 16.125, 0., -1.]])

    obs = env.observations()
    self.assertTrue(np.allclose(obs['DEBUG.PICKUPS'], pickups_state))


if __name__ == '__main__':
  if os.environ.get('TEST_SRCDIR'):
    deepmind_lab.set_runfiles_path(
        os.path.join(os.environ['TEST_SRCDIR'],
                     'org_deepmind_lab'))
  absltest.main()
