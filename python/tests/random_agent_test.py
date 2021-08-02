# Copyright 2016 Google Inc.
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
import six

import deepmind_lab
from python import random_agent


class RandomAgentsTest(absltest.TestCase):

  def test_spring_agent_run(self, length=100):
    env = deepmind_lab.Lab(
        'tests/empty_room_test', ['RGB_INTERLEAVED'],
        config={
            'fps': '60',
            'controls': 'external',
            'width': '80',
            'height': '80'
        })

    env.reset()
    agent = random_agent.SpringAgent(env.action_spec())

    reward = 0

    for _ in six.moves.range(length):
      if not env.is_running():
        print('Environment stopped early')
        env.reset()
      obs = env.observations()
      action = agent.step(reward, obs['RGB_INTERLEAVED'])
      reward = env.step(action, 1)
      self.assertIsInstance(reward, float)

  def test_discretized_random_agent_run(self, length=100):
    env = deepmind_lab.Lab(
        'tests/empty_room_test', ['RGB_INTERLEAVED'],
        config={
            'fps': '60',
            'width': '80',
            'height': '80'
        })

    env.reset()
    agent = random_agent.DiscretizedRandomAgent()

    reward = 0

    for _ in six.moves.range(length):
      if not env.is_running():
        print('Environment stopped early')
        env.reset()
      obs = env.observations()
      action = agent.step(reward, obs['RGB_INTERLEAVED'])
      reward = env.step(action, 1)
      self.assertIsInstance(reward, float)

  def test_map_frame_count(self, length=100):
    env = deepmind_lab.Lab(
        'tests/empty_room_test', ['MAP_FRAME_NUMBER'],
        config={'fps': '60',
                'width': '80',
                'height': '80'})

    env.reset()
    agent = random_agent.DiscretizedRandomAgent()

    reward = 0
    for frame in six.moves.range(length):
      if not env.is_running():
        print('Environment stopped early')
        env.reset()
      obs = env.observations()
      action = agent.step(reward, None)
      env.step(action, 1)
      frame_number = int(obs['MAP_FRAME_NUMBER'])
      self.assertEquals(frame, frame_number)


if __name__ == '__main__':
  if os.environ.get('TEST_SRCDIR'):
    deepmind_lab.set_runfiles_path(
        os.path.join(os.environ['TEST_SRCDIR'],
                     'org_deepmind_lab'))
  absltest.main()
