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
"""Custom action tests"""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import os
from absl.testing import absltest
import numpy as np

import deepmind_lab


class CustomViewTest(absltest.TestCase):

  def test_CustomActions(self):
    env = deepmind_lab.Lab(
        'tests/custom_actions_test', [],
        config={'fps': '60'})
    action_spec = env.action_spec()
    action_index = {action['name']: i for i, action in enumerate(action_spec)}
    self.assertIn('SWITCH_GADGET', action_index)
    self.assertIn('SELECT_GADGET', action_index)
    self.assertEqual(action_spec[action_index['SWITCH_GADGET']]['min'], -1)
    self.assertEqual(action_spec[action_index['SWITCH_GADGET']]['max'], 1)
    self.assertEqual(action_spec[action_index['SELECT_GADGET']]['min'], 0)
    self.assertEqual(action_spec[action_index['SELECT_GADGET']]['max'], 10)
    action = np.zeros([len(action_spec)], dtype=np.intc)
    env.reset()
    events = env.events()
    self.assertEmpty(events)
    env.step(action)
    events = env.events()
    self.assertEmpty(events)
    action[action_index['SWITCH_GADGET']] = 1
    env.step(action)
    events = env.events()
    self.assertLen(events, 1)
    name, values = events[0]
    self.assertEqual(name, 'SWITCH_GADGET')
    self.assertEqual(values[0], '1')
    action[action_index['SWITCH_GADGET']] = 0
    action[action_index['SELECT_GADGET']] = 1
    env.step(action)
    events = env.events()
    self.assertLen(events, 1)
    name, values = events[0]
    self.assertEqual(name, 'SELECT_GADGET')
    self.assertEqual(values[0], '1')

if __name__ == '__main__':
  if os.environ.get('TEST_SRCDIR'):
    deepmind_lab.set_runfiles_path(
        os.path.join(os.environ['TEST_SRCDIR'],
                     'org_deepmind_lab'))
  absltest.main()
