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
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import os
from absl.testing import absltest
import numpy as np
import six

import deepmind_lab

PLAYER_DEBUG_OBSERVATIONS = [
    'DEBUG.PLAYER_ID',
    'DEBUG.PLAYERS.ARMOR',
    'DEBUG.PLAYERS.GADGET',
    'DEBUG.PLAYERS.GADGET_AMOUNT',
    'DEBUG.PLAYERS.HEALTH',
    'DEBUG.PLAYERS.HOLDING_FLAG',
    'DEBUG.PLAYERS.ID',
    'DEBUG.PLAYERS.EYE.POS',
    'DEBUG.PLAYERS.EYE.ROT',
    'DEBUG.PLAYERS.VELOCITY',
    'DEBUG.PLAYERS.SCORE',
    'DEBUG.PLAYERS.IS_BOT',
    'DEBUG.PLAYERS.NAME',
    'DEBUG.PLAYERS.TEAM',
]

DEBUG_CAMERA_OBSERVATION = 'DEBUG.CAMERA.TOP_DOWN'
DEBUG_ICAMERA_OBSERVATION = 'DEBUG.CAMERA_INTERLEAVED.TOP_DOWN'
MAZE_LAYOUT_OBSERVATION = 'DEBUG.MAZE.LAYOUT'
TEST_MAP = """
********
*  *P  *
*P     *
********
""".lstrip('\n')

RANDOM_DEBUG_CAMERA_PIXEL_POS_VALUES = [
    ([55, 122], [187, 184, 255]),
    ([210, 150], [76, 108, 68]),
    ([250, 125], [214, 148, 28]),
]


class DebugObservationTest(absltest.TestCase):

  def test_amount(self):
    fps = 60
    player_name = 'PlayerInfoTest'
    env = deepmind_lab.Lab(
        'tests/debug_observation_test',
        PLAYER_DEBUG_OBSERVATIONS,
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
    for _ in six.moves.range(1000):
      env.step(action)
      obs = env.observations()
      if obs['DEBUG.PLAYERS.GADGET_AMOUNT'][0] == 0:
        break
    else:
      self.fail('Failed to empty gadget.')

    self.assertEqual(100, obs['DEBUG.PLAYERS.GADGET_AMOUNT'][1])

    action[action_index['FIRE']] = 0
    action[action_index['MOVE_BACK_FORWARD']] = 1

    for _ in six.moves.range(1000):
      env.step(action)
      obs = env.observations()
      if obs['DEBUG.PLAYERS.HEALTH'][0] <= 0:
        break
    else:
      self.fail('Failed to be tagged by agent., health still' +
                str(obs['DEBUG.PLAYERS.HEALTH'][0]))

    self.assertEqual(obs['DEBUG.PLAYERS.SCORE'][0], 0.)
    self.assertEqual(obs['DEBUG.PLAYERS.SCORE'][1], 1.)
    self.assertEqual(obs['DEBUG.PLAYERS.IS_BOT'][0], 0)
    self.assertEqual(obs['DEBUG.PLAYERS.IS_BOT'][1], 1)

    self.assertGreater(100, obs['DEBUG.PLAYERS.GADGET_AMOUNT'][1])

  def test_maze_layout(self):
    env = deepmind_lab.Lab(
        'tests/debug_observation_test', [MAZE_LAYOUT_OBSERVATION], config={})

    env.reset()
    self.assertEqual(env.observations()[MAZE_LAYOUT_OBSERVATION], TEST_MAP)

  def test_debug_camera(self):
    env = deepmind_lab.Lab(
        'tests/debug_observation_test',
        [DEBUG_CAMERA_OBSERVATION, DEBUG_ICAMERA_OBSERVATION,
         'RGB_INTERLEAVED'],
        config={'width': '320', 'height': '180'})

    env.reset()

    camera_image = env.observations()[DEBUG_CAMERA_OBSERVATION]
    icamera_image = env.observations()[DEBUG_ICAMERA_OBSERVATION]

    for (x, y), rgb in RANDOM_DEBUG_CAMERA_PIXEL_POS_VALUES:
      self.assertTrue(np.allclose(camera_image[:, y, x], rgb, atol=6))
      self.assertTrue(np.allclose(icamera_image[y, x, :], rgb, atol=6))


if __name__ == '__main__':
  if 'TEST_SRCDIR' in os.environ:
    deepmind_lab.set_runfiles_path(
        os.path.join(os.environ['TEST_SRCDIR'],
                     'org_deepmind_lab'))
  absltest.main()
