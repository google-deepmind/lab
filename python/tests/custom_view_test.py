# Copyright 2017 Google Inc.
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

import deepmind_lab
from python.tests.utils import math_utils


def DetectRed(screen):
  red_count = (np.linalg.norm(screen.transpose([1, 2, 0])
                              - np.array([255, 0, 0]), axis=2) < 128).sum()
  all_count = screen.shape[1] * screen.shape[2]
  return float(red_count) / all_count > 0.01


class CustomViewTest(absltest.TestCase):

  def test_LookRender(self):
    env = deepmind_lab.Lab(
        'tests/custom_view', ['RGB', 'RGB.LOOK_FORWARD', 'RGB.LOOK_BACK'],
        config={
            'fps': '60',
            'width': '64',
            'height': '64',
            'debugCamera': 'false',
            'maxAltCameraWidth': '128',
            'maxAltCameraHeight': '128',
        })

    env.reset()
    action_spec = env.action_spec()
    action_index = {action['name']: i for i, action in enumerate(action_spec)}
    action = np.zeros([len(action_spec)], dtype=np.intc)
    env.step(action)

    player = env.observations()['RGB']
    forward = env.observations()['RGB.LOOK_FORWARD']
    back = env.observations()['RGB.LOOK_BACK']

    self.assertEqual(player.shape, (3, 64, 64))
    self.assertEqual(forward.shape, (3, 128, 128))
    self.assertEqual(back.shape, (3, 128, 128))

    self.assertTrue(DetectRed(player))
    self.assertTrue(DetectRed(forward))
    self.assertFalse(DetectRed(back))

    current_angle = [0, 0]
    target_angle = [180, 0]
    look_speed = math_utils.calculate_speed(60, current_angle, target_angle, 8)
    action[action_index['LOOK_LEFT_RIGHT_PIXELS_PER_FRAME']] = look_speed[0]
    env.step(action, 8)  # Turn 180 degrees.

    player = env.observations()['RGB']
    forward = env.observations()['RGB.LOOK_FORWARD']
    back = env.observations()['RGB.LOOK_BACK']

    self.assertFalse(DetectRed(player))
    self.assertFalse(DetectRed(forward))
    self.assertTrue(DetectRed(back))

if __name__ == '__main__':
  if os.environ.get('TEST_SRCDIR'):
    deepmind_lab.set_runfiles_path(
        os.path.join(os.environ['TEST_SRCDIR'],
                     'org_deepmind_lab'))
  absltest.main()
