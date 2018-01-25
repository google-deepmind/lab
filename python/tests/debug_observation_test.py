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

ALL_DEBUG_OBSERVATIONS = [
    'DEBUG.PLAYER_ID',
    'DEBUG.PLAYERS.ARMOR',
    'DEBUG.PLAYERS.GADGET',
    'DEBUG.PLAYERS.GADGET_AMOUNT',
    'DEBUG.PLAYERS.HEALTH',
    'DEBUG.PLAYERS.HOLDING_FLAG',
    'DEBUG.PLAYERS.ID',
    'DEBUG.PLAYERS.EYE.POS',
    'DEBUG.PLAYERS.EYE.ROT',
    'DEBUG.PLAYERS.NAME',
    'DEBUG.PLAYERS.TEAM',
]


class DebugObservationTest(unittest.TestCase):

  def test_amount(self):
    fps = 60
    player_name = 'PlayerInfoTest'
    env = deepmind_lab.Lab(
        'tests/debug_observation_test', ALL_DEBUG_OBSERVATIONS,
        config={
            'fps': str(fps),
            'width': '80',
            'height': '80',
            'playerName': player_name,
        })

    action_spec = env.action_spec()
    action_index = {action['name']: i for i, action in enumerate(action_spec)}

    action = np.zeros([len(action_spec)], dtype=np.intc)
    env.reset()
    obs = env.observations()
    self.assertEqual(obs['DEBUG.PLAYER_ID'], obs['DEBUG.PLAYERS.ID'][0])

    names = obs['DEBUG.PLAYERS.NAME'].split('\n')
    self.assertEqual(player_name, names[0])

    self.assertEqual(100, obs['DEBUG.PLAYERS.GADGET_AMOUNT'][0])
    self.assertEqual(100, obs['DEBUG.PLAYERS.GADGET_AMOUNT'][1])

    ## Empty player's gadget ammo.
    action[action_index['FIRE']] = 1
    for _ in xrange(1000):
      env.step(action)
      obs = env.observations()
      if obs['DEBUG.PLAYERS.GADGET_AMOUNT'][0] == 0:
        break
    else:
      self.fail('Failed to empty gadget.')

    self.assertEqual(100, obs['DEBUG.PLAYERS.GADGET_AMOUNT'][1])

    action[action_index['FIRE']] = 0
    action[action_index['MOVE_BACK_FORWARD']] = 1

    for _ in xrange(1000):
      env.step(action)
      obs = env.observations()
      if obs['DEBUG.PLAYERS.HEALTH'][0] <= 0:
        break
    else:
      self.fail('Failed to be tagged by agent., health still' +
                str(obs['DEBUG.PLAYERS.HEALTH'][0]))

    self.assertGreater(100, obs['DEBUG.PLAYERS.GADGET_AMOUNT'][1])

if __name__ == '__main__':
  if 'TEST_SRCDIR' in os.environ:
    deepmind_lab.set_runfiles_path(
        os.path.join(os.environ['TEST_SRCDIR'],
                     'org_deepmind_lab'))
  unittest.main()
