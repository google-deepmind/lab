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
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import os
import unittest
import numpy as np

import deepmind_lab


class TeamModelSelectTest(unittest.TestCase):

  def test_with_six_bots(self):
    env = deepmind_lab.Lab(
        'tests/team_model_select', [],
        config={'botCount': '6'})
    env.reset()
    action_spec = env.action_spec()
    action = np.zeros([len(action_spec)], dtype=np.intc)
    event_names = sorted([name for name, _ in env.events()])
    self.assertEqual(len(event_names), 7)
    for i, name in enumerate(event_names):
      self.assertEqual(name, 'skinModified' + str(i))
    env.step(action, 1)
    self.assertEqual(len(env.events()), 0)
    env.reset()
    self.assertEqual(len(env.events()), 7)

  def test_without_bot(self):
    env = deepmind_lab.Lab(
        'tests/team_model_select', [],
        config={'botCount': '0'})
    env.reset()
    action_spec = env.action_spec()
    action = np.zeros([len(action_spec)], dtype=np.intc)
    event_names = sorted([name for name, _ in env.events()])
    self.assertEqual(len(event_names), 1)
    self.assertEqual(event_names[0], 'skinModified0')
    env.step(action, 1)
    self.assertEqual(len(env.events()), 0)
    env.reset()
    self.assertEqual(len(env.events()), 1)

if __name__ == '__main__':
  if 'TEST_SRCDIR' in os.environ:
    deepmind_lab.set_runfiles_path(
        os.path.join(os.environ['TEST_SRCDIR'],
                     'org_deepmind_lab'))
  unittest.main()
