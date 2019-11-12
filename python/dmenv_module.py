# Copyright 2019 Google LLC
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

"""A DeepMind Lab Python module that implements DeepMind's dm_env API."""

import dm_env
import numpy as np
import six
import deepmind_lab


def set_runfiles_path(path):
  """Module-level function to set the path of the DeepMind Lab DSOs."""
  deepmind_lab.set_runfiles_path(path)


class Lab(dm_env.Environment):
  """Implements dm_env.Environent; forwards calls to deepmind_lab."""

  def __init__(self, level, observation_names, config):
    self._lab = deepmind_lab.Lab(level, observation_names, config)
    self._observation_names = observation_names
    self._needs_reset = True

    lab_action_specs = self._lab.action_spec()
    self._action_spec = {}
    self._action_map = {}
    self._action_count = len(lab_action_specs)
    for i, spec in enumerate(lab_action_specs):
      name = spec["name"]
      self._action_map[name] = i
      self._action_spec[name] = dm_env.specs.BoundedArray(
          dtype=np.dtype("int32"),
          shape=(),
          name=name,
          minimum=spec["min"],
          maximum=spec["max"])

    self._observation_spec = {}
    for spec in self._lab.observation_spec():
      name = spec["name"]
      shape = spec["shape"]
      if name in self._observation_names:
        if 0 in shape:
          raise NotImplementedError(
              "Dynamic shapes are not supported by dm_env; requested shape for "
              "observation {} was {}.".format(name, shape))
        self._observation_spec[name] = dm_env.specs.Array(
            dtype=spec["dtype"], shape=shape, name=name)

  def _observation(self):
    return {
        name: np.asarray(data, dtype=self._observation_spec[name].dtype)
        for name, data in six.iteritems(self._lab.observations())
    }

  def reset(self):
    self._lab.reset()
    self._needs_reset = False
    return dm_env.restart(self._observation())

  def step(self, action):
    if self._needs_reset:
      return self.reset()

    lab_action = np.empty(self._action_count, dtype=np.dtype("int32"))
    for name, value in six.iteritems(action):
      lab_action[self._action_map[name]] = value

    reward = self._lab.step(lab_action)

    if self._lab.is_running():
      return dm_env.transition(reward=reward, observation=self._observation())
    else:
      self._needs_reset = True
      return dm_env.termination(reward=reward, observation=self._observation())

  def action_spec(self):
    return self._action_spec

  def observation_spec(self):
    return self._observation_spec
