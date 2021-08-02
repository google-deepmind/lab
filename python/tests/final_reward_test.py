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
"""Make sure final reward is received."""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import os
from absl.testing import absltest
import numpy as np
import six

import deepmind_lab


class FinalRewardTest(absltest.TestCase):

  def test_final_reward(self):
    env = deepmind_lab.Lab(
        'tests/final_reward_test', [],
        config={
            'fps': '60',
            'width': '80',
            'height': '80'
        })

    action_spec = env.action_spec()
    action = np.zeros([len(action_spec)], dtype=np.intc)
    env.reset()
    reward = 0
    for _ in six.moves.range(11):
      if not env.is_running():
        break
      reward += env.step(action, 1)

    self.assertEqual(reward, 11 * 10 / 2)

  def test_final_reward_skip(self):
    env = deepmind_lab.Lab(
        'tests/final_reward_test', [],
        config={
            'fps': '60',
            'width': '80',
            'height': '80'
        })

    action_spec = env.action_spec()
    action = np.zeros([len(action_spec)], dtype=np.intc)
    env.reset()
    reward = 0
    for _ in six.moves.range(4):
      if not env.is_running():
        break
      reward += env.step(action, 4)

    self.assertEqual(reward, 11 * 10 / 2)

if __name__ == '__main__':
  if 'TEST_SRCDIR' in os.environ:
    deepmind_lab.set_runfiles_path(
        os.path.join(os.environ['TEST_SRCDIR'],
                     'org_deepmind_lab'))
  absltest.main()
