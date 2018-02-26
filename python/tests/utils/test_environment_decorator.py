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
"""An environment decorator with additional functions used for testing."""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import os
import numpy as np
from PIL import Image

_RGB_OBSERVATION_NAME = 'RGB_INTERLEAVED'


class TestEnvironmentDecorator(object):
  """A test environment decorator class.

  This class provides additional functionality used for testing:
    - Accumulating reward over multiple frames.
    - Accumulating events over multiple frames.
    - Collecting RGB observations and saving them to image files.
  """

  def __init__(self, environment):
    """Initializes the decorator."""
    self._environment = environment
    self._rgb_history = []
    self._frame_index = 0
    self._events = []
    self._reward_history = []

  def step(self, actions, steps):
    """Advance the environment a number of steps."""
    reward = self._environment.step(actions, steps)
    observations = self._environment.observations()
    self._save_current_frame()
    self._accumulate_events(self._environment.events())
    self._reward_history.append(reward)
    return reward

  def accumulated_reward(self):
    """Return the reward accumulated since the reset call."""
    return np.sum(self._reward_history)

  def reward_history(self):
    """Return the reward history accumulated since the reset call."""
    return self._reward_history

  def accumulated_events(self):
    """Return events accumulated since the reset call."""
    return self._events

  def is_running(self):
    """If the environment is in status RUNNING."""
    return self._environment.is_running()

  def observations(self):
    """Get the observations."""
    return self._environment.observations()

  def events(self):
    """Get the events."""
    return self._environment.events()

  def action_spec(self):
    """The shape of the actions."""
    return self._environment.action_spec()

  def observation_spec(self):
    """The shape of the observations."""
    return self._environment.observation_spec()

  def _find_observation_spec(self, name):
    for spec in self._environment.observation_spec():
      if name == spec['name']:
        return spec
    return None

  def reset(self, **kwargs):
    """Reset the environment."""
    result = self._environment.reset(**kwargs)
    self._accumulated_reward = 0
    self._events = []
    self._accumulate_events(self._environment.events())
    self._rgb_history = []
    self._save_current_frame()
    self._reward_history = []
    return result

  def _accumulate_events(self, events):
    if events is not None:
      self._events.append(events)

  def save_frames(self, folder):
    """Save RGB_INTERLEAVED observations collacted since the last reset call."""
    if not os.path.exists(folder):
      os.makedirs(folder)

    for index, image_data in enumerate(self._rgb_history):
      image = Image.fromarray(np.uint8(image_data))
      image.save(os.path.join(folder, 'frame{0}.png'.format(index)))

  def num_steps(self):
    """Number of frames since the last reset() call."""
    return self._environment.num_steps()

  def _save_current_frame(self):
    observations = self._environment.observations()
    if _RGB_OBSERVATION_NAME in observations:
      self._rgb_history.append(observations['RGB_INTERLEAVED'])
