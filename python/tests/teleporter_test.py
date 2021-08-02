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


class TeleporterTest(absltest.TestCase):

  def test_movement(self):
    fps = 60
    env = deepmind_lab.Lab(
        'tests/teleporter_test', [
            'VEL.TRANS',
            'DEBUG.POS.TRANS',
            'DEBUG.POS.ROT',
        ],
        config={
            'fps': str(fps),
            'width': '80',
            'height': '80',
        })

    action_spec = env.action_spec()
    action_index = {action['name']: i for i, action in enumerate(action_spec)}

    action = np.zeros([len(action_spec)], dtype=np.intc)

    env.reset()
    vel = env.observations()['VEL.TRANS']
    self.assertTrue(np.array_equal(vel, np.array([0, 0, 0])))

    # Agent begins facing south
    initial_facing = env.observations()['DEBUG.POS.ROT']
    self.assertTrue(np.allclose(initial_facing, np.array([0, -90, 0]),
                                atol=0.1))

    # Player moves straight ahead through the teleporter
    action[action_index['MOVE_BACK_FORWARD']] = 1
    self.assertEqual(env.events(), [])
    for _ in six.moves.range(120):
      p_before = env.observations()['DEBUG.POS.TRANS']
      env.step(action, 1)
      p_after = env.observations()['DEBUG.POS.TRANS']
      if p_after[1] - p_before[1] > 100:
        break
    else:
      self.fail('Failed to teleport')
    self.assertEqual(env.events(), [('PLAYER_TELEPORTED', [])])
    env.step(action, 1)
    self.assertEqual(env.events(), [])

if __name__ == '__main__':
  if 'TEST_SRCDIR' in os.environ:
    deepmind_lab.set_runfiles_path(
        os.path.join(os.environ['TEST_SRCDIR'],
                     'org_deepmind_lab'))
  absltest.main()
