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
"""A high level DM Lab controller class."""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import numpy as np
import six

from python.tests.utils import math_utils

# Tolerance used when changing player orientation.
ROTATION_TOLERANCE = 2.

# Tolerance used when changing player position.
POSITION_TOLERANCE = 5.

_FPS = 60
_TURN_ACTION_NAME = 'LOOK_LEFT_RIGHT_PIXELS_PER_FRAME'
_MOVE_ACTION_NAME = 'MOVE_BACK_FORWARD'
_JUMP_ACTION_NAME = 'JUMP'
_NOOP_ACTION = np.array([0, 0, 0, 0, 0, 0, 0], dtype=np.intc)
_MIN_BRAKING_SPEED = 50.
_VELOCITY_TOLERANCE = 1e-08
_INTERNAL_ROTATION_TOLERANCE = ROTATION_TOLERANCE * .5
_INTERNAL_POSITION_TOLERANCE = POSITION_TOLERANCE * .7
_SLOW_MOVE_DISTANCE = 10.


class ControllerException(Exception):
  """Base class for all game controller exceptions."""


class EpisodeFinishedError(ControllerException):
  """Raised when an episode is finished while controller is running."""


class PathBlockedError(ControllerException):
  """Raised when player path is blocked."""


class GameController(object):
  """A high level game controller class.

  The class allows interacting with DMLab environment using high level actions,
  such as look at specific direction, move to specific coordinates etc.
  """

  def __init__(self, env):
    """Initialize the controller."""
    self._env = env

    action_spec = env.action_spec()
    self._action_index = {
        action['name']: i
        for i, action in enumerate(action_spec)
    }

  def look_at_2d(self, target_orientation, steps=10):
    """Rotate the player in horizontal plane towards the angle."""
    delta_angle = math_utils.delta_angle_degrees(self.orientation[1],
                                                 target_orientation)
    self.rotate(delta_angle, steps=steps)

  def rotate(self, delta_angle, steps=10):
    """Rotate the player delta_angle degrees in horizontal plane."""
    start = self.orientation
    target = np.array([start[0], start[1] + delta_angle, 0.0])

    speed = self._rotation_speed(delta_angle, steps=steps)

    if np.abs(speed) < 1.:
      return

    actions = self._get_actions(_TURN_ACTION_NAME, speed)

    error_message = ('Failed to reach requested orientation. Start: {0}, '
                     'target: {1}, current: {2}.')
    for _ in six.moves.range(steps):
      if self._env.is_running():
        self._step(actions, 1)
      else:
        raise AssertionError(
            error_message.format(start[1], target[1], self.orientation[1]))

    if abs(math_utils.delta_angle_degrees(
        self.orientation[1], target[1])) >= _INTERNAL_ROTATION_TOLERANCE:
      if steps == 1:
        raise AssertionError(
            error_message.format(start[1], target[1], self.orientation[1]))
      else:
        self.rotate(
            math_utils.delta_angle_degrees(self.orientation[1], target[1]),
            steps=1)

  def move_to(self,
              target_x,
              target_y,
              max_steps=2000,
              max_speed=None):
    """Move the player to the target location."""
    pos = self.position
    target = np.array([target_x, target_y, pos[2]])

    if max_speed is None:
      max_speed = float('inf')

    direction = (target - pos)[:2]
    last_pos = np.array([float('inf'), float('inf'), float('inf')])
    target_orientation = np.degrees(np.arctan2(direction[1], direction[0]))
    self.look_at_2d(target_orientation)

    blocked_frames_count = 0
    for _ in six.moves.range(max_steps):
      move_action_value = 1
      pos = self.position
      direction = (target - pos)[:2]
      distance = np.linalg.norm(direction)
      speed = np.linalg.norm(self.velocity[:2])

      if distance < _SLOW_MOVE_DISTANCE:
        if speed > 0.0:
          self.stop()
      elif speed > max_speed:
        move_action_value = 0

      if distance < _INTERNAL_POSITION_TOLERANCE:
        break

      if np.linalg.norm(last_pos - pos) < .1:
        blocked_frames_count += 1
        if blocked_frames_count > 10:
          raise PathBlockedError(
              'Failed to reach target. The path might be blocked.')
      else:
        blocked_frames_count = 0
      last_pos = pos

      target_orientation = np.degrees(np.arctan2(direction[1], direction[0]))
      rotation_speed = self._rotation_speed(
          math_utils.delta_angle_degrees(self.orientation[1],
                                         target_orientation),
          steps=1)
      actions = self._get_empty_actions()
      self._set_action_value(actions, _TURN_ACTION_NAME, rotation_speed)
      self._set_action_value(actions, _MOVE_ACTION_NAME, move_action_value)
      self._step(actions, 1)
    else:
      raise AssertionError('Failed to reach target in max steps.')

  def stop(self, max_steps=2000):
    """Stops the player as soon as possible."""
    start_orientation = self.orientation[1]
    for _ in six.moves.range(max_steps):
      speed = np.linalg.norm(self.velocity[:2])
      if speed < _VELOCITY_TOLERANCE and np.allclose(
          self.rotation_velocity, [0.0, 0.0, 0.0],
          atol=_VELOCITY_TOLERANCE):
        break
      if speed < _MIN_BRAKING_SPEED:
        self._step(_NOOP_ACTION, 1)
      else:
        self.look_at_2d(
            self.orientation[1] +
            np.degrees(np.arctan2(self.velocity[1], self.velocity[0])),
            steps=1)

        self._step(self._get_actions(_MOVE_ACTION_NAME, -1), 1)
    else:
      raise AssertionError('Failed to stop in max steps.')
    self.look_at_2d(start_orientation, steps=1)

  def jump(self):
    self._step(self._get_actions(_JUMP_ACTION_NAME, 1), 1)
    self.stop()

  def _step(self, actions, steps):
    try:
      return self._env.step(actions, steps)
    except RuntimeError:
      if self._env.is_running():
        raise
      else:
        raise EpisodeFinishedError()

  @property
  def position(self):
    return self._observations['DEBUG.POS.TRANS']

  @property
  def orientation(self):
    return self._observations['DEBUG.POS.ROT']

  @property
  def velocity(self):
    return self._observations['VEL.TRANS']

  @property
  def rotation_velocity(self):
    return self._observations['VEL.ROT']

  @property
  def _observations(self):
    try:
      return self._env.observations()
    except RuntimeError as e:
      if self._env.is_running():
        raise
      else:
        raise EpisodeFinishedError from e

  def _get_empty_actions(self):
    return np.zeros([len(self._action_index)], dtype=np.intc)

  def _set_action_value(self, actions, action_name, action_value):
    actions[self._action_index[action_name]] = action_value

  def _get_actions(self, action_name, action_value):
    actions = self._get_empty_actions()
    self._set_action_value(actions, action_name, action_value)
    return actions

  def _rotation_speed(self, delta_angle, steps=10):
    start = self.orientation
    target = np.array([start[0], start[1] + delta_angle, 0.0])

    if math_utils.angles_within_tolerance(self.orientation, target):
      return 0.0

    return math_utils.calculate_speed(_FPS, start, target, steps)[1]
