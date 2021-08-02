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
"""Set of tests for DMLab determinism."""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import os
import pprint
from absl.testing import absltest
import numpy as np
import six

import deepmind_lab


# Helper function to create DMLab environment.
def make_dmlab_environment(args=None, observations=None):
  config = {
      'fps': '60',
      'width': '32',
      'height': '32',
  }

  if args:
    for k, v in six.iteritems(args):
      config[k] = v
  if observations is None:
    observations = ['DEBUG.POS.TRANS']
  env = deepmind_lab.Lab(
      'seekavoid_arena_01', observations, config=config)
  env.reset(episode=1, seed=123)
  return env


class DeterminismTest(absltest.TestCase):

  # Tests that performing the same action N times produces the same player
  # position in each environment.
  def test_player_position(self, num_steps=20):
    env1 = make_dmlab_environment()
    env2 = make_dmlab_environment()

    move_fwd = np.array([0, 0, 0, 1, 0, 0, 0], dtype=np.intc)
    for _ in six.moves.range(num_steps):
      self.assertEqual(env1.step(move_fwd, 1), env2.step(move_fwd, 1))
      pos1 = env1.observations()['DEBUG.POS.TRANS']
      pos2 = env2.observations()['DEBUG.POS.TRANS']
      self.assertTrue(
          np.allclose(pos1, pos2),
          'Player positions differ!\n' + pprint.pformat({
              'pos1': pos1,
              'pos2': pos2
          }))

  # Test that skipping render frames simulates the same player positions as not
  # frame skipping
  def test_frame_skip(self, num_steps=20, repeated_actions=4):
    env1 = make_dmlab_environment()
    env2 = make_dmlab_environment()

    move_fwd = np.array([0, 0, 0, 1, 0, 0, 0], dtype=np.intc)
    for _ in six.moves.range(num_steps):
      for _ in six.moves.range(repeated_actions):
        env1.step(move_fwd, 1)
      env2.step(move_fwd, repeated_actions)
      pos1 = env1.observations()['DEBUG.POS.TRANS']
      pos2 = env2.observations()['DEBUG.POS.TRANS']
      self.assertTrue(
          np.allclose(pos1, pos2),
          'Player positions differ!\n' + pprint.pformat({
              'pos1': pos1,
              'pos2': pos2
          }))

  # Tests that calling step(a, N) is the same as calling step(a, 1) N times.
  def test_repeated_actions(self, num_steps=20, repeated_actions=4):
    env = make_dmlab_environment()
    env_repeated = make_dmlab_environment()

    self.assertEqual(num_steps % repeated_actions, 0)

    move_fwd = np.array([0, 0, 0, 1, 0, 0, 0], dtype=np.intc)
    for _ in six.moves.range(int(num_steps / repeated_actions)):
      accum_reward = 0.0
      for _ in six.moves.range(repeated_actions):
        accum_reward += env.step(move_fwd, 1)
      self.assertEqual(
          env_repeated.step(move_fwd, repeated_actions), accum_reward)
      pos1 = env.observations()['DEBUG.POS.TRANS']
      pos2 = env.observations()['DEBUG.POS.TRANS']
      self.assertTrue(
          np.allclose(pos1, pos2),
          'Player positions differ!\n' + pprint.pformat({
              'pos1': pos1,
              'pos2': pos2
          }))

  def test_pbo_pixels(self):
    env = make_dmlab_environment(
        args={'use_pbos': 'false'}, observations=['RGBD'])
    pbo_env = make_dmlab_environment(
        args={'use_pbos': 'true'}, observations=['RGBD'])

    move_fwd = np.array([0, 0, 0, 1, 0, 0, 0], dtype=np.intc)
    for _ in six.moves.range(5):
      self.assertEqual(env.step(move_fwd, 1), pbo_env.step(move_fwd, 1))
      pixels = env.observations()['RGBD']
      pbo_pixels = pbo_env.observations()['RGBD']

      self.assertTrue(
          np.allclose(pixels, pbo_pixels),
          'Pixels differ using PBOs!\n' + pprint.pformat({
              'pixels': pixels,
              'pbo pixels': pbo_pixels
          }))


if __name__ == '__main__':
  if os.environ.get('TEST_SRCDIR'):
    deepmind_lab.set_runfiles_path(
        os.path.join(os.environ['TEST_SRCDIR'],
                     'org_deepmind_lab'))
  absltest.main()
