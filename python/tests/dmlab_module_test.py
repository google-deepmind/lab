# Copyright 2016-2018 Google Inc.
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
from absl.testing import absltest
import numpy as np
import six

import deepmind_lab


class DeepMindLabTest(absltest.TestCase):

  def testInitArgs(self):
    with six.assertRaisesRegex(self, TypeError, 'must be dict, not list'):
      deepmind_lab.Lab('tests/empty_room_test', [], ['wrongconfig'])
    with six.assertRaisesRegex(self, TypeError, 'str|bad argument type'):
      deepmind_lab.Lab('tests/empty_room_test', [], {'wrongtype': 3})
    with six.assertRaisesRegex(self, TypeError, 'must be list, not None'):
      deepmind_lab.Lab('tests/empty_room_test', None, {})
    with six.assertRaisesRegex(self, ValueError, 'Unknown observation'):
      deepmind_lab.Lab('tests/empty_room_test', ['nonexisting_obs'], {})

  def testReset(self):
    lab = deepmind_lab.Lab('tests/empty_room_test', [], {})
    with six.assertRaisesRegex(self, ValueError,
                               '\'seed\' must be int or None, was \'str\''):
      lab.reset(seed='invalid')

  def testTempFolder(self):
    temp_folder = os.path.join(os.environ['TEST_TMPDIR'], 'test_temp_folder')
    lab = deepmind_lab.Lab(
        'contributed/dmlab30/explore_goal_locations_small', [], {},
        temp_folder=temp_folder)
    lab.reset()
    self.assertGreaterEqual(len(os.listdir(temp_folder)), 1)

  def testSpecs(self):
    lab = deepmind_lab.Lab('tests/empty_room_test', [])
    observation_spec = lab.observation_spec()
    observation_spec_names = {o['name'] for o in observation_spec}
    observation_spec_lookup = {o['name']: o for o in observation_spec}
    action_names = {a['name'] for a in lab.action_spec()}
    observation_set = {'RGB_INTERLEAVED', 'RGB', 'RGBD_INTERLEAVED', 'RGBD',
                       'BGR_INTERLEAVED', 'BGRD_INTERLEAVED'}
    self.assertGreaterEqual(observation_spec_names, observation_set)
    for k in observation_set:
      o = observation_spec_lookup[k]
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
        deepmind_lab.Lab('tests/empty_room_test', []) for _ in range(5)]
    for lab in labs:
      self.assertTrue(lab.close())

  def testRun(self, steps=10, observation='RGB_INTERLEAVED'):
    env = deepmind_lab.Lab('lt_chasm', [observation])
    env.reset()

    for _ in six.moves.range(steps):
      obs = env.observations()
      action = np.zeros((7,), dtype=np.intc)
      reward = env.step(action, num_steps=4)

      self.assertEqual(obs[observation].shape, (240, 320, 3))
      self.assertEqual(reward, 0.0)

  def testRunClosed(self):
    env = deepmind_lab.Lab('lt_chasm', ['RGB_INTERLEAVED'])
    env.reset(episode=42, seed=7)
    env.close()

    action = np.zeros((7,), dtype=np.intc)

    with six.assertRaisesRegex(self, RuntimeError, 'wrong status to advance'):
      env.step(action)
    with six.assertRaisesRegex(self, RuntimeError, 'wrong status'):
      env.observations()

  def testRunfilesPath(self):
    self.assertTrue(os.stat(deepmind_lab.runfiles_path()))

  def testWidthHeight(self, width=80, height=80, steps=10, num_steps=1):
    observation = 'RGBD'
    env = deepmind_lab.Lab(
        'lt_chasm', [observation],
        config={'height': str(height), 'width': str(width)})
    env.reset()

    for _ in six.moves.range(steps):
      obs = env.observations()
      action = np.zeros((7,), dtype=np.intc)
      reward = env.step(action, num_steps=num_steps)

      self.assertEqual(obs[observation].shape, (4, width, height))
      self.assertEqual(reward, 0.0)

  def testStringObervations(self):
    observation = 'CUSTOM_TEXT'
    env = deepmind_lab.Lab(
        'tests/text_observation_test', [observation],
        config={'height': '32', 'width': '32'})
    observation_spec = env.observation_spec()
    observation_spec_lookup = {o['name']: o for o in observation_spec}
    spec = observation_spec_lookup[observation]
    self.assertEqual(spec['name'], observation)
    self.assertEqual(spec['shape'], ())
    self.assertEqual(spec['dtype'], str)
    env.reset()
    self.assertEqual(env.observations()[observation], 'Example Output')

  def testEvents(self):
    env = deepmind_lab.Lab(
        'tests/event_test', [],
        config={'height': '32', 'width': '32'})
    env.reset(episode=1, seed=7)
    events = env.events()
    self.assertLen(events, 4)

    name, obs = events[0]
    self.assertEqual(name, 'TEXT')
    self.assertEqual(obs[0], 'EPISODE 1')

    name, obs = events[1]
    self.assertEqual(name, 'DOUBLE')
    np.testing.assert_array_equal(obs[0], np.array([[1., 0.], [0., 1.]]))

    name, obs = events[2]
    self.assertEqual(name, 'BYTE')
    np.testing.assert_array_equal(obs[0], np.array([2, 2], dtype=np.uint8))

    name, obs = events[3]
    self.assertEqual(name, 'ALL')
    self.assertEqual(obs[0], 'Text')
    np.testing.assert_array_equal(obs[1], np.array([3], dtype=np.uint8))
    np.testing.assert_array_equal(obs[2], np.array([7.]))

    action = np.zeros((7,), dtype=np.intc)
    env.step(action, num_steps=1)
    self.assertEmpty(env.events())
    env.step(action, num_steps=58)
    self.assertEmpty(env.events())
    env.step(action, num_steps=1)
    self.assertFalse(env.is_running())

    events = env.events()
    self.assertLen(events, 1)
    name, obs = events[0]
    self.assertEqual(name, 'LOG')
    self.assertEqual(obs[0], 'Episode ended')

    env.reset(episode=2, seed=8)

    events = env.events()
    self.assertLen(events, 4)

    name, obs = events[0]
    self.assertEqual(name, 'TEXT')
    self.assertEqual(obs[0], 'EPISODE 2')

  def testVeloctyObservations(self, width=80, height=80):
    noop_action = np.zeros((7,), dtype=np.intc)
    forward_action = np.array([0, 0, 0, 1, 0, 0, 0], dtype=np.intc)
    backward_action = - forward_action
    look_sideways_action = np.array([512, 0, 0, 0, 0, 0, 0], dtype=np.intc)

    env = deepmind_lab.Lab(
        'seekavoid_arena_01', ['VEL.TRANS', 'VEL.ROT', 'RGBD_INTERLEAVED'],
        config={'height': str(height), 'width': str(width), 'fps': '60'})

    env.reset(seed=1)

    # Initial landing on the ground.
    env.step(noop_action, num_steps=180)

    for _ in six.moves.range(3):
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

    for _ in six.moves.range(3):
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
  absltest.main()
