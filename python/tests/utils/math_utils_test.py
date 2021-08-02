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
"""Tests for math_utils."""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from absl.testing import absltest
from python.tests.utils import math_utils


class MathUtilsTest(absltest.TestCase):

  def testDeltaAngleDegrees(self):
    self.assertAlmostEqual(math_utils.delta_angle_degrees(10., 20.), 10.)
    self.assertAlmostEqual(math_utils.delta_angle_degrees(20., 10.), -10.)
    self.assertAlmostEqual(math_utils.delta_angle_degrees(10., 350.), -20.)
    self.assertAlmostEqual(math_utils.delta_angle_degrees(350., 10.), 20.)
    self.assertAlmostEqual(math_utils.delta_angle_degrees(350., 10.), 20.)
    self.assertAlmostEqual(math_utils.delta_angle_degrees(10., 190.), -180.)
    self.assertAlmostEqual(math_utils.delta_angle_degrees(190., 10.), -180.)
    self.assertAlmostEqual(math_utils.delta_angle_degrees(10., 195.), -175.)
    self.assertAlmostEqual(math_utils.delta_angle_degrees(195., 10.), 175.)
    self.assertAlmostEqual(math_utils.delta_angle_degrees(-350., -10.), -20.)
    self.assertAlmostEqual(math_utils.delta_angle_degrees(-10., -350.), 20.)

  def testAnglesWithinTolerance(self):
    self.assertTrue(
        math_utils.angles_within_tolerance(
            [10., -20.], [10.5, -20.5], tolerance=1.))

    self.assertTrue(
        math_utils.angles_within_tolerance(
            [10., 350], [-350.1, -9.6], tolerance=1.))

    self.assertFalse(
        math_utils.angles_within_tolerance(
            [10., -20.], [11.5, -20.5], tolerance=1.))

    self.assertFalse(
        math_utils.angles_within_tolerance(
            [10., -18.], [10.5, -20.5], tolerance=1.))


if __name__ == '__main__':
  absltest.main()
