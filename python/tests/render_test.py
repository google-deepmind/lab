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
"""Basic test for DMLab rendering."""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import os
from absl.testing import absltest
import numpy as np

import deepmind_lab


class RenderTest(absltest.TestCase):

  def test_vert_flip_buffer(self):
    noop = np.array([0, 0, 0, 0, 0, 0, 0], dtype=np.intc)

    env0 = deepmind_lab.Lab(
        'tests/render_test', ['RGB', 'RGB_INTERLEAVED'],
        config={
            'fps': '60',
            'width': '512',
            'height': '512',
            'randomSeed': '1',
            'vertFlipBuffer': '0',
        })
    env0.reset()
    self.assertEqual(env0.step(noop, 1), 0)
    image0 = np.transpose(env0.observations()['RGB'], (1, 2, 0))

    env1 = deepmind_lab.Lab(
        'tests/render_test', ['RGB', 'RGB_INTERLEAVED'],
        config={
            'fps': '60',
            'width': '512',
            'height': '512',
            'randomSeed': '1',
            'vertFlipBuffer': '1',
        })
    env1.reset()
    self.assertEqual(env1.step(noop, 1), 0)
    image1 = np.transpose(env1.observations()['RGB'], (1, 2, 0))

    # Test skydome pixels.
    np.testing.assert_array_equal(image1[0, 0], [150, 224, 255])
    np.testing.assert_array_equal(image1[0, 511], [150, 224, 255])

    np.testing.assert_array_equal(
        image0, env0.observations()['RGB_INTERLEAVED'])
    np.testing.assert_array_equal(
        image1, env1.observations()['RGB_INTERLEAVED'])

    # Delta between original and reversed flipped observations.
    # Images won't be identical due to texture sampling and aliasing biases
    # depending on the direction of rasterisation.
    np.testing.assert_almost_equal(
        np.average(abs(image0 - np.flipud(image1))) / 255, 0, decimal=2)

if __name__ == '__main__':
  if os.environ.get('TEST_SRCDIR'):
    deepmind_lab.set_runfiles_path(
        os.path.join(os.environ['TEST_SRCDIR'],
                     'org_deepmind_lab'))
  absltest.main()
