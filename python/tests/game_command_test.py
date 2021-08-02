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
"""Game command tests."""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import os
from absl.testing import absltest
import numpy as np
import six

import deepmind_lab


class GameCommandTest(absltest.TestCase):

  def test_GameCommand(self):
    env = deepmind_lab.Lab(
        'tests/game_command_test', [],
        config={'fps': '60'})
    action_spec = env.action_spec()
    action_index = {action['name']: i for i, action in enumerate(action_spec)}
    self.assertIn('NEXT_GADGET', action_index)
    action = np.zeros([len(action_spec)], dtype=np.intc)
    env.reset()
    action[action_index['NEXT_GADGET']] = 1
    env.step(action)
    events = env.events()
    self.assertEqual(events[0][1][0], 'DISC')
    action[action_index['NEXT_GADGET']] = 0
    for _ in six.moves.range(100):
      env.step(action)
      events = env.events()
      if events[0][1][0] == 'RAPID':
        break
      self.assertEqual(events[0][1][0], 'DISC')
    else:
      self.fail('Failed to switch gadget')

    action[action_index['NEXT_GADGET']] = 1
    env.step(action)
    action[action_index['NEXT_GADGET']] = 0
    for _ in six.moves.range(100):
      env.step(action)
      events = env.events()
      if events[0][1][0] == 'BEAM':
        break
      self.assertEqual(events[0][1][0], 'RAPID')
    else:
      self.fail('Failed to switch gadget')

if __name__ == '__main__':
  if os.environ.get('TEST_SRCDIR'):
    deepmind_lab.set_runfiles_path(
        os.path.join(os.environ['TEST_SRCDIR'],
                     'org_deepmind_lab'))
  absltest.main()
