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
"""Basic test for DeepMind Lab Python wrapper."""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import os
import unittest
import numpy as np

import deepmind_lab


class DeepMindLabTest(unittest.TestCase):

  def testInitArgs(self):
    with self.assertRaisesRegexp(TypeError, 'must be dict, not list'):
      deepmind_lab.Lab('tests/demo_map', [], ['wrongconfig'])
    with self.assertRaisesRegexp(TypeError, 'str'):
      deepmind_lab.Lab('tests/demo_map', [], {'wrongtype': 3})
    with self.assertRaisesRegexp(TypeError, 'must be list, not None'):
      deepmind_lab.Lab('tests/demo_map', None, {})
    with self.assertRaisesRegexp(ValueError, 'Unknown observation'):
      deepmind_lab.Lab('tests/demo_map', ['nonexisting_obs'], {})

  def testReset(self):
    lab = deepmind_lab.Lab('tests/demo_map', [], {})
    with self.assertRaisesRegexp(ValueError,
                                 '\'seed\' must be int or None, was \'str\''):
      lab.reset(seed='invalid')

  def testSpecs(self):
    lab = deepmind_lab.Lab('tests/demo_map', [])
    observation_spec = lab.observation_spec()
    observation_names = {o['name'] for o in observation_spec}
    action_names = {a['name'] for a in lab.action_spec()}

    self.assertSetEqual(observation_names,
                        {'RGB_INTERLACED', 'RGB', 'RGBD_INTERLACED', 'RGBD'})
    for o in observation_spec:
      self.assertIn('shape', o)
      self.assertDictContainsSubset({'dtype': np.uint8}, o)

    self.assertSetEqual(
        action_names,
        {'LOOK_LEFT_RIGHT_PIXELS_PER_FRAME',
         'LOOK_DOWN_UP_PIXELS_PER_FRAME',
         'STRAFE_LEFT_RIGHT',
         'MOVE_BACK_FORWARD',
         'FIRE',
         'JUMP',
         'CROUCH'})

    for a in lab.action_spec():
      self.assertIs(type(a['min']), int)
      self.assertIs(type(a['max']), int)

    self.assertTrue(lab.close())

  def testOpenClose(self):
    labs = [
        deepmind_lab.Lab('tests/demo_map', []) for _ in range(5)]
    for lab in labs:
      self.assertTrue(lab.close())

  def testRun(self, steps=10, observation='RGB_INTERLACED'):
    env = deepmind_lab.Lab('lt_chasm', [observation])
    env.reset()

    for _ in xrange(steps):
      obs = env.observations()
      action = np.zeros((7,), dtype=np.intc)
      reward = env.step(action, num_steps=4)

      self.assertEqual(obs[observation].shape, (240, 320, 3))
      self.assertEqual(reward, 0.0)

  def testRunClosed(self):
    env = deepmind_lab.Lab('lt_chasm', ['RGB_INTERLACED'])
    env.reset(episode=42, seed=7)
    env.close()

    action = np.zeros((7,), dtype=np.intc)

    with self.assertRaisesRegexp(RuntimeError, 'wrong status to advance'):
      env.step(action)
    with self.assertRaisesRegexp(RuntimeError, 'wrong status'):
      env.observations()

  def testRunfilesPath(self):
    self.assertTrue(os.stat(deepmind_lab.runfiles_path()))

  def testWidthHeight(self, width=80, height=80, steps=10, num_steps=1):
    observations = ['RGBD']
    env = deepmind_lab.Lab('lt_chasm', observations,
                           config={'height': str(height),
                                   'width': str(width)})
    env.reset()

    for _ in xrange(steps):
      obs = env.observations()
      action = np.zeros((7,), dtype=np.intc)
      reward = env.step(action, num_steps=num_steps)

      self.assertEqual(obs[observations[0]].shape, (4, width, height))
      self.assertEqual(reward, 0.0)

  def testVeloctyObservations(self, width=80, height=80):
    noop_action = np.zeros((7,), dtype=np.intc)
    forward_action = np.array([0, 0, 0, 1, 0, 0, 0], dtype=np.intc)
    backward_action = - forward_action
    look_sideways_action = np.array([512, 0, 0, 0, 0, 0, 0], dtype=np.intc)


    env = deepmind_lab.Lab('seekavoid_arena_01', ['VEL.TRANS', 'VEL.ROT'],
                           config={'height': str(height),
                                   'width': str(width),
                                   'fps': '60'})

    env.reset(seed=1)

    # Initial landing on the ground.
    env.step(noop_action, num_steps=180)

    for _ in xrange(3):
      # Doing nothing should result in velocity observations of zero.
      env.step(noop_action, num_steps=100)
      obs = env.observations()
      np.testing.assert_array_equal(obs['VEL.TRANS'], np.zeros((3,)))
      np.testing.assert_array_equal(obs['VEL.ROT'], np.zeros((3,)))

      env.step(forward_action, num_steps=100)
      obs = env.observations()

      forward_vel = obs['VEL.TRANS']
      self.assertEqual(forward_vel[2], 0.0)  # zero velocity in z direction
      self.assertTrue(np.any(forward_vel))
      np.testing.assert_array_equal(obs['VEL.ROT'], np.zeros((3,)))

      env.step(noop_action, num_steps=4)

      # Going backward should result in negative velocity of going forward
      env.step(backward_action, num_steps=100)
      obs = env.observations()
      self.assertAlmostEqual(np.linalg.norm(obs['VEL.TRANS'] + forward_vel),
                             0.0, delta=3)
      np.testing.assert_array_equal(obs['VEL.ROT'], np.zeros((3,)))

    env.reset(seed=1)

    for _ in xrange(3):
      env.step(noop_action, num_steps=100)
      obs = env.observations()
      np.testing.assert_array_equal(obs['VEL.TRANS'], np.zeros((3,)))
      np.testing.assert_array_equal(obs['VEL.ROT'], np.zeros((3,)))

      env.step(look_sideways_action, num_steps=100)
      obs = env.observations()

      sideways_vel = obs['VEL.ROT']
      self.assertEqual(sideways_vel[2], 0.0)
      self.assertTrue(np.any(forward_vel))
      np.testing.assert_array_equal(obs['VEL.TRANS'], np.zeros((3,)))

      env.step(noop_action, num_steps=4)

      env.step(-look_sideways_action, num_steps=100)
      obs = env.observations()
      self.assertAlmostEqual(np.linalg.norm(obs['VEL.ROT'] + sideways_vel),
                             0.0, delta=3)
      np.testing.assert_array_equal(obs['VEL.TRANS'], np.zeros((3,)))


if __name__ == '__main__':
  if os.environ.get('TEST_SRCDIR'):
    deepmind_lab.set_runfiles_path(os.path.join(
        os.environ['TEST_SRCDIR'], 'org_deepmind_lab'))
  unittest.main()
