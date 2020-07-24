## Copyright (C) 2016-17 Google Inc.
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License along
## with this program; if not, write to the Free Software Foundation, Inc.,
## 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
################################################################################
"""A working example of deepmind_lab using python."""
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import argparse
import pprint
import sys
import numpy as np
import six

import deepmind_lab


def run(level_script, config, num_episodes):
  """Construct and start the environment."""
  env = deepmind_lab.Lab(level_script, ['RGB_INTERLEAVED'], config)
  env.reset()

  observation_spec = env.observation_spec()
  print('Observation spec:')
  pprint.pprint(observation_spec)

  action_spec = env.action_spec()
  print('Action spec:')
  pprint.pprint(action_spec)

  obs = env.observations()  # dict of Numpy arrays
  rgb_i = obs['RGB_INTERLEAVED']
  print('Observation shape:', rgb_i.shape)
  sys.stdout.flush()

  # Create an action to move forwards.
  action = np.zeros([7], dtype=np.intc)
  action[3] = 1

  score = 0
  for _ in six.moves.range(num_episodes):
    while env.is_running():
      # Advance the environment 4 frames while executing the action.
      reward = env.step(action, num_steps=4)

      if reward != 0:
        score += reward
        print('Score =', score)
        sys.stdout.flush()


if __name__ == '__main__':
  parser = argparse.ArgumentParser(description=__doc__)
  parser.add_argument('-l', '--level_script', type=str,
                      default='seekavoid_arena_01',
                      help='The level that is to be played. Levels'
                      'are Lua scripts, and a script called \"name\" means that'
                      'a file \"assets/game_scripts/name.lua is loaded.')
  parser.add_argument('-s', '--level_settings', type=str, default=[],
                      action='append',
                      help='Applies an opaque key-value setting. The setting is'
                      'available to the level script. This flag may be provided'
                      'multiple times. Universal settings are `width` and '
                      '`height` which give the screen size in pixels, '
                      '`fps` which gives the frames per second, and '
                      '`random_seed` which can be specified to ensure the '
                      'same content is generated on every run.')
  parser.add_argument('--runfiles_path', type=str, default=None,
                      help='Set the runfiles path to find DeepMind Lab data')
  parser.add_argument('--num_episodes', type=int, default=1,
                      help='The number of episodes to play.')
  args = parser.parse_args()

  # Convert list of level setting strings (of the form "key=value") into a
  # `config` key/value dictionary.
  config = {
      k: v
      for k, v in [six.ensure_str(s).split('=') for s in args.level_settings]
  }

  if args.runfiles_path:
    deepmind_lab.set_runfiles_path(args.runfiles_path)
  run(args.level_script, config, args.num_episodes)
