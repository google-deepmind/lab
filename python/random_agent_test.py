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
import unittest

import deepmind_lab
import random_agent


class RandomAgentsTest(unittest.TestCase):

  def test_spring_agent_run(self, length=100):
    env = deepmind_lab.Lab(
        'tests/demo_map', ['RGB_INTERLACED'],
        config={
            'fps': '60',
            'controls': 'external',
            'width': '80',
            'height': '80'
        })

    env.reset()
    agent = random_agent.SpringAgent(env.action_spec())

    reward = 0

    for _ in xrange(length):
      if not env.is_running():
        print('Environment stopped early')
        env.reset()
      obs = env.observations()
      action = agent.step(reward, obs['RGB_INTERLACED'])
      reward = env.step(action, 1)
      self.assertIsInstance(reward, float)

  def test_discretized_random_agent_run(self, length=100):
    env = deepmind_lab.Lab(
        'tests/demo_map', ['RGB_INTERLACED'],
        config={
            'fps': '60',
            'width': '80',
            'height': '80'
        })

    env.reset()
    agent = random_agent.DiscretizedRandomAgent()

    reward = 0

    for _ in xrange(length):
      if not env.is_running():
        print('Environment stopped early')
        env.reset()
      obs = env.observations()
      action = agent.step(reward, obs['RGB_INTERLACED'])
      reward = env.step(action, 1)
      self.assertIsInstance(reward, float)


if __name__ == '__main__':
  if os.environ.get('TEST_SRCDIR'):
    deepmind_lab.set_runfiles_path(
        os.path.join(os.environ['TEST_SRCDIR'],
                     'org_deepmind_lab'))
  unittest.main()
