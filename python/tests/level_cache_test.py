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
"""Tests Python level cache."""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import os
import shutil
import six
import tempfile
from absl.testing import absltest

import numpy as np

import deepmind_lab


class LevelCacheTest(absltest.TestCase):

  def setUp(self):
    self._test_dir = tempfile.mkdtemp()
    self._dummy_action = np.array([0, 0, 0, 0, 0, 0, 0], dtype=np.intc)

  def tearDown(self):
    shutil.rmtree(self._test_dir)

  def _create_environment(self, level_cache):
    return deepmind_lab.Lab(
        level='contributed/dmlab30/explore_goal_locations_small',
        observations=['RGB_INTERLEAVED'],
        config={
            'width': '96',
            'height': '72',
        },
        level_cache=level_cache)

  def test_missing_write(self):
    class LevelCache(object):

      def fetch(self, key, pk3_path):
        del key
        del pk3_path
        return False

    env = self._create_environment(LevelCache())
    with six.assertRaisesRegex(self, AttributeError, 'write'):
      env.reset(episode=1, seed=123)
      env.step(self._dummy_action)

  def test_missing_fetch(self):
    class LevelCache(object):

      def write(self, key, pk3_path):
        pass

    env = self._create_environment(LevelCache())
    with six.assertRaisesRegex(self, AttributeError, 'fetch'):
      env.reset(episode=1, seed=123)
      env.step(self._dummy_action)

  def test_incorrect_fetch(self):
    class LevelCache(object):

      def fetch(self):  # Missing arguments.
        return False

      def write(self, key, pk3_path):
        pass

    env = self._create_environment(LevelCache())
    with six.assertRaisesRegex(self, TypeError,
                               '(exactly 1|takes 1 positional) argument'):
      env.reset(episode=1, seed=123)
      env.step(self._dummy_action)

  def test_exception_in_fetch(self):
    class LevelCache(object):

      def fetch(self, key, pk3_path):
        del key
        del pk3_path
        raise ValueError('foo')

      def write(self, key, pk3_path):
        pass

    env = self._create_environment(LevelCache())
    with six.assertRaisesRegex(self, ValueError, 'foo'):
      env.reset(episode=1, seed=123)
      env.step(self._dummy_action)

  def test_level_cache_none(self):
    # This should be equivalent to not set the level cache at all and just work.
    env = self._create_environment(level_cache=None)
    env.reset(episode=1, seed=123)
    env.step(self._dummy_action)

  def test_local_level_cache(self):
    num_successful_fetches = [0]
    num_failed_fetches = [0]
    num_writes = [0]

    class LevelCache(object):

      def __init__(self, cache_dir):
        self._cache_dir = cache_dir

      def fetch(self, key, pk3_path):
        path = os.path.join(self._cache_dir, key)
        if os.path.isfile(path):
          shutil.copyfile(path, pk3_path)
          num_successful_fetches[0] += 1
          return True
        num_failed_fetches[0] += 1
        return False

      def write(self, key, pk3_path):
        path = os.path.join(self._cache_dir, key)
        num_writes[0] += 1
        shutil.copyfile(pk3_path, path)

    env = self._create_environment(LevelCache(cache_dir=self._test_dir))
    env.reset(episode=1, seed=123)
    env.step(self._dummy_action)

    self.assertEqual(0, num_successful_fetches[0])
    self.assertEqual(1, num_failed_fetches[0])
    self.assertEqual(1, num_writes[0])

    env.reset(episode=1, seed=123)
    env.step(self._dummy_action)

    self.assertEqual(1, num_successful_fetches[0])
    self.assertEqual(1, num_failed_fetches[0])
    self.assertEqual(1, num_writes[0])

if __name__ == '__main__':
  if os.environ.get('TEST_SRCDIR'):
    deepmind_lab.set_runfiles_path(
        os.path.join(os.environ['TEST_SRCDIR'],
                     'org_deepmind_lab'))
  absltest.main()
