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
"""Test for the EpisodeTimeMs callback."""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import os
from absl.testing import absltest
import numpy as np
import six

import deepmind_lab


class EpisodeTimeTest(absltest.TestCase):

  def run_at_frame_rate(self, fps):
    env = deepmind_lab.Lab(
        'tests/episode_time_test', ['EPISODE_TIME_SECONDS'],
        config={
            'fps': str(fps),
            'width': '32',
            'height': '32'
        })

    env.reset()
    nop = np.zeros((7,), dtype=np.intc)

    for _ in six.moves.range(0, fps):
      env.step(nop, 1)

    obs = env.observations()
    self.assertEqual(obs['EPISODE_TIME_SECONDS'][0], 1.0)

  def test_at_60(self):
    self.run_at_frame_rate(60)

  def test_at_30(self):
    self.run_at_frame_rate(30)

if __name__ == '__main__':
  if os.environ.get('TEST_SRCDIR'):
    deepmind_lab.set_runfiles_path(
        os.path.join(os.environ['TEST_SRCDIR'],
                     'org_deepmind_lab'))
  absltest.main()
