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
"""Tests for Adding extra Entities and their interaction with Bots."""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import os
from absl.testing import absltest
import numpy as np
import six

import deepmind_lab


class ExtraEntitiesWithBotsTest(absltest.TestCase):

  def test_bot_with_spawned_weapon(self):
    env = deepmind_lab.Lab(
        'tests/extra_entities_with_bots_test', ['DEBUG.POS.TRANS'],
        config={
            'fps': '60',
            'width': '32',
            'height': '32',
            'spawnWeapons': 'true'
        })

    noop = np.zeros([len(env.action_spec())], dtype=np.intc)
    env.reset()
    for _ in six.moves.range(6000):
      env.step(noop, 1)
      if [event for event in env.events() if event[0] == 'PLAYER_TAGGED']:
        break
    else:
      self.fail('Failed to be tagged!')


if __name__ == '__main__':
  if 'TEST_SRCDIR' in os.environ:
    deepmind_lab.set_runfiles_path(
        os.path.join(os.environ['TEST_SRCDIR'],
                     'org_deepmind_lab'))
  absltest.main()
