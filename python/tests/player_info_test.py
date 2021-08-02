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
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import os
from absl.testing import absltest
import numpy as np
import six

import deepmind_lab
from python.tests.utils import math_utils


class PlayerInfo(absltest.TestCase):

  def test_movement(self):
    fps = 60
    env = deepmind_lab.Lab(
        'tests/empty_room_test', [
            'VEL.TRANS',
            'VEL.ROT',
            'DEBUG.POS.TRANS',
            'DEBUG.POS.ROT',
            'DEBUG.PLAYER_ID',
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
    player_id = env.observations()['DEBUG.PLAYER_ID']
    self.assertEqual(player_id[0], 1)
    vel = env.observations()['VEL.TRANS']
    self.assertTrue(np.array_equal(vel, np.array([0, 0, 0])))

    ops = [{
        'axis': 0,
        'fact': 320,
        'lr': 0,
        'bf': 1
    }, {
        'axis': 0,
        'fact': -320,
        'lr': 0,
        'bf': -1
    }, {
        'axis': 1,
        'fact': -320,
        'lr': 1,
        'bf': 0
    }, {
        'axis': 1,
        'fact': 320,
        'lr': -1,
        'bf': 0
    }]

    before = env.observations()['DEBUG.POS.TRANS']
    for op in ops:
      action[action_index['STRAFE_LEFT_RIGHT']] = op['lr']
      action[action_index['MOVE_BACK_FORWARD']] = op['bf']
      for _ in six.moves.range(60):
        env.step(action, 1)
        vel = env.observations()['VEL.TRANS']
        vel_axis = vel[op['axis']] / op['fact']
        self.assertLessEqual(0, vel_axis)
        if vel_axis >= 1:
          break
      else:
        print(env.observations()['VEL.TRANS'])
        self.fail('Failed to reach max velocity')

      action[action_index['STRAFE_LEFT_RIGHT']] = 0
      action[action_index['MOVE_BACK_FORWARD']] = 0

      for _ in six.moves.range(60):
        env.step(action, 1)
        vel = env.observations()['VEL.TRANS']
        vel_axis = vel[op['axis']] / op['fact']
        if vel_axis == 0:
          break
      else:
        print(env.observations()['VEL.TRANS'])
        self.fail('Failed to stop')
    after = env.observations()['DEBUG.POS.TRANS']

    self.assertTrue(np.allclose(before, after, atol=3.0))

    frames = 66
    speed = 22
    env.reset()
    env.step(action, 2)
    action[action_index['LOOK_LEFT_RIGHT_PIXELS_PER_FRAME']] = speed
    env.step(action, frames)
    action[action_index['LOOK_LEFT_RIGHT_PIXELS_PER_FRAME']] = 0
    env.step(action, 10)
    rot = env.observations()['DEBUG.POS.ROT']
    # Angles are stored 0 East and CCW is positive. So negate speed.
    yaw = math_utils.calculate_angle(fps, -speed, frames)
    self.assertTrue(np.allclose(rot, np.array([0, yaw, 0]), atol=1e-2))
    action[action_index['LOOK_LEFT_RIGHT_PIXELS_PER_FRAME']] = speed / -2
    env.step(action, frames * 2)
    action[action_index['LOOK_LEFT_RIGHT_PIXELS_PER_FRAME']] = 0
    env.step(action, 10)
    rot = env.observations()['DEBUG.POS.ROT']
    self.assertTrue(np.allclose(rot, np.array([0, 0, 0]), atol=0.01))

    speed = -10
    env.reset()
    env.step(action, 2)
    action[action_index['LOOK_DOWN_UP_PIXELS_PER_FRAME']] = speed
    env.step(action, frames)
    action[action_index['LOOK_DOWN_UP_PIXELS_PER_FRAME']] = 0
    env.step(action, 10)
    rot = env.observations()['DEBUG.POS.ROT']
    pitch = math_utils.calculate_angle(fps, speed, frames)
    self.assertTrue(np.allclose(rot, np.array([pitch, 0, 0]), atol=1e-2))
    action[action_index['LOOK_DOWN_UP_PIXELS_PER_FRAME']] = speed / -2
    env.step(action, frames * 2)
    action[action_index['LOOK_DOWN_UP_PIXELS_PER_FRAME']] = 0
    env.step(action, 10)
    rot = env.observations()['DEBUG.POS.ROT']
    self.assertTrue(np.allclose(rot, np.array([0, 0, 0]), atol=0.01))

if __name__ == '__main__':
  if 'TEST_SRCDIR' in os.environ:
    deepmind_lab.set_runfiles_path(
        os.path.join(os.environ['TEST_SRCDIR'],
                     'org_deepmind_lab'))
  absltest.main()
