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
"""Utility methods for DMLab testing."""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import math

ROTATION_TOLERANCE = 1  # Degree

# Time dialiation works to make 60fps become exactly 16ms.
TIME_DIALATION = 0.96

# This is a natural speed to convert between mouse movement and screen
# rotation.
PIXELS_PER_FRAME_TO_DEG_PER_SECOND = 0.11 * 60


def calculate_angle(fps, rotation_speed, frames):
  """Calculates rotation angle achieved by applying the rotation_speed."""
  time_seconds = TIME_DIALATION * frames / fps

  angle = rotation_speed * time_seconds * PIXELS_PER_FRAME_TO_DEG_PER_SECOND

  # Angles in the engine are converted to shorts to maintain determinism.
  short = int(angle * (65536.0 / 360.0) + 0.5) % 65536
  angle0_360 = short * 360.0 / 65536.0

  # Return angle normalised to [-180, 180)
  return angle0_360 if angle0_360 < 180 else angle0_360 - 360


def calculate_speed(fps, current_angle, target_angle, frames):
  """Calculates speed required to achieve the target angle."""
  time_seconds = TIME_DIALATION * frames / fps
  degrees_per_second = time_seconds * PIXELS_PER_FRAME_TO_DEG_PER_SECOND

  return [(target_angle[0] - current_angle[0]) / degrees_per_second,
          (current_angle[1] - target_angle[1]) / degrees_per_second]


def angles_within_tolerance(current_angle,
                            target_angle,
                            tolerance=ROTATION_TOLERANCE):
  """Tests whether two angles are within tolerance."""
  return (
      abs(delta_angle_degrees(current_angle[0], target_angle[0])) <= tolerance
      and
      abs(delta_angle_degrees(current_angle[1], target_angle[1])) <= tolerance)


def delta_angle_degrees(from_angle_degrees, to_angle_degrees):
  """Calculates the shortest signed delta angle."""
  delta = to_angle_degrees - from_angle_degrees
  return delta - 360.0 * math.floor((delta + 180.0) / 360.0)
