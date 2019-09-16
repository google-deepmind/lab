# Copyright 2016 Google Inc.
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
"""Measures DeepMind Lab performance by running low-overhead Random agent."""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import argparse
import random
import time
import numpy as np
import six

import deepmind_lab


def _action(*entries):
  return np.array(entries, dtype=np.intc)


class DiscretizedRandomAgent(object):
  """Simple agent for DeepMind Lab."""

  ACTION_LIST = [
      _action(0, 0, -1, 0, 0, 0, 0),  # strafe_left
      _action(0, 0, 1, 0, 0, 0, 0),   # strafe_right
      _action(0, 0, 0, 1, 0, 0, 0),   # forward
      _action(0, 0, 0, -1, 0, 0, 0),  # backward
      _action(0, 0, 0, 0, 1, 0, 0),   # fire
      _action(0, 0, 0, 0, 0, 1, 0),   # crouch
      _action(0, 0, 0, 0, 0, 0, 1),   # jump
  ]

  def step(self, unused_reward, unused_image):
    """Gets an image state and a reward, returns an action."""
    return random.choice(DiscretizedRandomAgent.ACTION_LIST)


def run(length, width, height, fps, level, observation_spec):
  """Spins up an environment and runs the random agent."""
  env = deepmind_lab.Lab(
      level, [observation_spec],
      config={
          'fps': str(fps),
          'width': str(width),
          'height': str(height)
      })

  env.reset()

  agent = DiscretizedRandomAgent()

  reward = 0

  t0 = time.time()

  for _ in six.moves.range(length):
    if not env.is_running():
      print('Environment stopped early')
      env.reset()
      agent.reset()
    obs = env.observations()
    action = agent.step(reward, obs[observation_spec])
    reward = env.step(action, num_steps=1)

  t1 = time.time()
  duration = t1 - t0

  print('resolution: %i x %i, spec: %s, steps: %i, duration: %.1f, fps: %.1f' %
        (width, height, observation_spec, length, duration, length / duration))


def main():
  parser = argparse.ArgumentParser(description=__doc__)
  parser.add_argument('--runfiles_path', type=str, default=None,
                      help='Set the runfiles path to find DeepMind Lab data')

  args = parser.parse_args()
  if args.runfiles_path:
    deepmind_lab.set_runfiles_path(args.runfiles_path)

  # Test for 1 minute of simulation time at fixed in-game frame rate.
  length = 3600
  fps = 60

  # Benchmark at each of the following levels at the specified resolutions and
  # observation specs.
  for level_script in ['nav_maze_static_01', 'lt_space_bounce_hard']:
    for width, height in [(84, 84), (160, 120), (320, 240)]:
      for observation_spec in ['RGB', 'RGBD']:
        run(length, width, height, fps, level_script, observation_spec)


if __name__ == '__main__':
  main()
