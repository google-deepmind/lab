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
"""Tests for inventory reading."""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import os
from absl.testing import absltest
import numpy as np
import six

import deepmind_lab

GADGETS = {
    'IMPULSE': 2,  ## Contact gadget.
    'RAPID': 3,    ## Rapid fire gadget.
    'ORB': 6,      ## Area damage gadget. (Knocks players)
    'BEAM': 7,     ## Accurate and very rapid fire beam.
    'DISC': 8,     ## Powerful but long period between firing.
}


class UpdateInventoryTest(absltest.TestCase):

  def test_weapon_auto_switch(self):
    env = deepmind_lab.Lab(
        'tests/update_inventory_test', ['DEBUG.AMOUNT', 'DEBUG.GADGET'],
        config={
            'fps': '60',
            'width': '80',
            'height': '80'
        })

    action_spec = env.action_spec()
    action_index = {action['name']: i for i, action in enumerate(action_spec)}

    action = np.zeros([len(action_spec)], dtype=np.intc)

    action[action_index['FIRE']] = 1

    env.reset()

    self.assertEqual(env.observations()['DEBUG.GADGET'][0], GADGETS['ORB'])
    self.assertEqual(env.observations()['DEBUG.AMOUNT'][0], 2)

    ## Fire two orbs.
    for _ in six.moves.range(600):
      env.step(action, 1)
      self.assertEqual(env.observations()['DEBUG.GADGET'][0], GADGETS['ORB'])
      if env.observations()['DEBUG.AMOUNT'][0] == 0:
        break
    else:
      self.fail('Failed use orbs!')

    ## Wait for weapon switch.
    for _ in six.moves.range(600):
      env.step(action, 1)
      if env.observations()['DEBUG.GADGET'][0] == GADGETS['RAPID']:
        break
    else:
      self.fail('Failed to auto switch weapon!')

    self.assertEqual(env.observations()['DEBUG.AMOUNT'][0], 10)
    for _ in six.moves.range(600):
      env.step(action, 1)
      if env.observations()['DEBUG.AMOUNT'][0] == 0:
        break
    else:
      self.fail('Failed to use rapid!')


if __name__ == '__main__':
  if 'TEST_SRCDIR' in os.environ:
    deepmind_lab.set_runfiles_path(
        os.path.join(os.environ['TEST_SRCDIR'],
                     'org_deepmind_lab'))
  absltest.main()
