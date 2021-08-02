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
"""Tests TestEnvironmentDecorator."""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import os
import tempfile
from absl.testing import absltest
import numpy as np
import six
from PIL import Image

from python.tests.utils import test_environment_decorator

_OBSERVATION_SPEC = [{'name': 'RGB_INTERLEAVED', 'shape': [1, 2, 3]}]


class EnvironmentStub(object):

  def __init__(self):
    self.test_observation_spec = _OBSERVATION_SPEC
    self.test_observations = [
        {
            'RGB_INTERLEAVED':
                np.array(
                    [[[255, 0, 0], [128, 0, 0], [0, 0, 255]],
                     [[0, 255, 0], [128, 0, 0], [0, 255, 0]]],
                    dtype=np.uint8)
        },
        {
            'RGB_INTERLEAVED':
                np.array([[[0, 255, 0], [0, 128, 0]]], dtype=np.uint8)
        },
        {
            'RGB_INTERLEAVED':
                np.array([[[0, 0, 255], [0, 0, 128]]], dtype=np.uint8)
        },
    ]
    self.test_rewards = [0, 1, 2, 3]
    self._frame_index = 0
    self.last_actions = None
    self.last_steps = None
    self.events_return = None
    self.is_running_return = None
    self.action_spec_return = None
    self.reset_return = None

  def step(self, actions, steps):
    self.last_actions = actions
    self.last_steps = steps
    self._frame_index += 1
    return self.test_rewards[self._frame_index - 1]

  def is_running(self):
    return self.is_running_return

  def observations(self):
    return self.test_observations[self._frame_index]

  def events(self):
    return self.events_return

  def action_spec(self):
    return self.action_spec_return

  def observation_spec(self):
    return self.test_observation_spec

  def reset(self, **_):
    self._frame_index = 0
    return self.reset_return

  def num_steps(self):
    return self._frame_index


class TestEnvironmentDecoratorTest(absltest.TestCase):

  def setUp(self):
    self._env = EnvironmentStub()
    self._decorator = test_environment_decorator.TestEnvironmentDecorator(
        self._env)

  def testStepIsCalled(self):
    actions = object()
    steps = 3

    self.assertEqual(
        self._decorator.step(actions, steps), self._env.test_rewards[0])
    self.assertEqual(self._env.last_actions, actions)
    self.assertEqual(self._env.last_steps, steps)

  def testAccumulatedReward(self):
    self._decorator.step(None, 1)
    self._decorator.step(None, 1)
    self.assertEqual(self._decorator.accumulated_reward(),
                     np.sum(self._env.test_rewards[0:2]))

  def testResetAccumulatedReward(self):
    self._decorator.step(None, 1)
    self._decorator.reset()
    self.assertEqual(self._decorator.accumulated_reward(), 0)

  def testRewardHistory(self):
    self._decorator.step(None, 1)
    self._decorator.step(None, 1)
    six.assertCountEqual(self,
                         self._decorator.reward_history(),
                         self._env.test_rewards[0:2])

  def testResetRewardHistory(self):
    self._decorator.step(None, 1)
    self._decorator.reset()
    six.assertCountEqual(self, self._decorator.reward_history(), [])

  def testAccumulatedEvents(self):
    events = ['event1', 'event2', 'event3']
    self._env.events_return = events[0]
    self._decorator.reset()
    self._env.events_return = events[1]
    self._decorator.step(None, 1)
    self._env.events_return = events[2]
    self._decorator.step(None, 1)
    six.assertCountEqual(self, self._decorator.accumulated_events(), events)

  def testResetAccumulatedEvents(self):
    events = ['event1', 'event2']

    self._env.events_return = events[0]
    self._decorator.step(None, 1)
    self._env.events_return = events[1]
    self._decorator.reset()
    six.assertCountEqual(self,
                         self._decorator.accumulated_events(), [events[1]])

  def testObservationDelegation(self):
    self.assertEqual(self._env.test_observations[0],
                     self._decorator.observations())

  def testObservationSpecDelegation(self):
    self.assertEqual(self._env.test_observation_spec,
                     self._decorator.observation_spec())

  def testNumSteps(self):
    self._decorator.reset()
    self.assertEqual(self._decorator.num_steps(), 0)
    self._decorator.step(None, None)
    self.assertEqual(self._decorator.num_steps(), 1)

  def testMethodDelegation(self):
    method_names = ['is_running', 'events', 'action_spec', 'reset']
    for name in method_names:
      result = object()
      setattr(self._env, name + '_return', result)
      self.assertEqual(getattr(self._decorator, name)(), result)

  def testSavingFrames(self):
    self._decorator.reset()
    self._decorator.step(None, 1)
    self._decorator.step(None, 1)

    temp_dir = tempfile.mkdtemp()
    self._decorator.save_frames(temp_dir)

    for index, observation in enumerate(self._env.test_observations):
      expected_image = observation['RGB_INTERLEAVED']
      image_file_name = os.path.join(temp_dir, 'frame{0}.png'.format(index))
      image = np.asarray(Image.open(image_file_name))
      self.assertTrue(np.array_equal(image, expected_image))


if __name__ == '__main__':
  absltest.main()
