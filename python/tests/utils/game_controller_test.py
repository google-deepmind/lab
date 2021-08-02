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
"""Tests for GameController."""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import os
from absl.testing import absltest
import numpy as np
import six

import deepmind_lab
from python.tests.utils import game_controller
from python.tests.utils import math_utils
from python.tests.utils import test_environment_decorator

_TEST_TRIALS = 5


class GameControllerTest(absltest.TestCase):

  def setUp(self):
    self._env = test_environment_decorator.TestEnvironmentDecorator(
        deepmind_lab.Lab(
            'seekavoid_arena_01',
            ['VEL.ROT', 'VEL.TRANS', 'DEBUG.POS.ROT', 'DEBUG.POS.TRANS']))
    self._controller = game_controller.GameController(self._env)
    self._env.reset()

  def testLookAt(self):
    target_angles = np.random.uniform(360, size=(_TEST_TRIALS))
    for target_angle in target_angles:
      self._controller.look_at_2d(target_angle)
      delta = math_utils.delta_angle_degrees(self._controller.orientation[1],
                                             target_angle)
      self.assertLess(abs(delta), game_controller.ROTATION_TOLERANCE)

  def testMoveTo(self):
    targets = np.random.uniform(low=-100, high=100, size=(_TEST_TRIALS, 2))
    for target in targets:
      self._controller.move_to(target[0], target[1])
      distance = np.linalg.norm(target - self._controller.position[0:2])
      self.assertLess(distance, game_controller.POSITION_TOLERANCE)

  def testStopping(self):
    for angle in [0.0, 90., 180., -90]:
      self._controller.look_at_2d(0.0)
      move_forward_action = self._controller._get_actions(
          game_controller._MOVE_ACTION_NAME, 1)
      for _ in six.moves.range(10):
        self._env.step(move_forward_action, 1)
      self._controller.look_at_2d(angle)
      num_steps_before_stopping = self._env.num_steps()
      self._controller.stop()
      num_steps_after_stopping = self._env.num_steps()
      self.assertLess(num_steps_after_stopping - num_steps_before_stopping, 15)
      delta = math_utils.delta_angle_degrees(self._controller.orientation[1],
                                             angle)
      self.assertLess(abs(delta), game_controller.ROTATION_TOLERANCE)

  def testMovingSlowly(self):
    self._controller.move_to(.0, .0)
    start_steps = self._env.num_steps()
    self._controller.move_to(100.0, .0)
    steps_moving_fast = self._env.num_steps() - start_steps
    start_steps = self._env.num_steps()
    self._controller.move_to(.0, .0, max_speed=50.0)
    self.assertLess(steps_moving_fast, self._env.num_steps() - start_steps)

  def testControllerThrowsExceptionWhenBlocked(self):
    self._controller.move_to(100.0, .0)
    with self.assertRaises(game_controller.PathBlockedError):
      self._controller.move_to(10000.0, .0)

  def testControllerThrowsExceptionWhenEpisodeFinishes(self):
    with self.assertRaises(game_controller.EpisodeFinishedError):
      for _ in six.moves.range(1000):
        self._controller.move_to(100.0, .0)
        self._controller.move_to(0.0, .0)


if __name__ == '__main__':
  if 'TEST_SRCDIR' in os.environ:
    deepmind_lab.set_runfiles_path(
        os.path.join(os.environ['TEST_SRCDIR'],
                     'org_deepmind_lab'))
  absltest.main()
