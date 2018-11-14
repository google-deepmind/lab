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
"""Basic random agent for DeepMind Lab."""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import argparse
import random
import numpy as np
import six

import deepmind_lab


def _action(*entries):
  return np.array(entries, dtype=np.intc)


class DiscretizedRandomAgent(object):
  """Simple agent for DeepMind Lab."""

  ACTIONS = {
      'look_left': _action(-20, 0, 0, 0, 0, 0, 0),
      'look_right': _action(20, 0, 0, 0, 0, 0, 0),
      'look_up': _action(0, 10, 0, 0, 0, 0, 0),
      'look_down': _action(0, -10, 0, 0, 0, 0, 0),
      'strafe_left': _action(0, 0, -1, 0, 0, 0, 0),
      'strafe_right': _action(0, 0, 1, 0, 0, 0, 0),
      'forward': _action(0, 0, 0, 1, 0, 0, 0),
      'backward': _action(0, 0, 0, -1, 0, 0, 0),
      'fire': _action(0, 0, 0, 0, 1, 0, 0),
      'jump': _action(0, 0, 0, 0, 0, 1, 0),
      'crouch': _action(0, 0, 0, 0, 0, 0, 1)
  }

  ACTION_LIST = list(six.viewvalues(ACTIONS))

  def step(self, unused_reward, unused_image):
    """Gets an image state and a reward, returns an action."""
    return random.choice(DiscretizedRandomAgent.ACTION_LIST)


class SpringAgent(object):
  """A random agent using spring-like forces for its action evolution."""

  def __init__(self, action_spec):
    self.action_spec = action_spec
    print('Starting random spring agent. Action spec:', action_spec)

    self.omega = np.array([
        0.1,  # look left-right
        0.1,  # look up-down
        0.1,  # strafe left-right
        0.1,  # forward-backward
        0.0,  # fire
        0.0,  # jumping
        0.0  # crouching
    ])

    self.velocity_scaling = np.array([2.5, 2.5, 0.01, 0.01, 1, 1, 1])

    self.indices = {a['name']: i for i, a in enumerate(self.action_spec)}
    self.mins = np.array([a['min'] for a in self.action_spec])
    self.maxs = np.array([a['max'] for a in self.action_spec])
    self.reset()

    self.rewards = 0

  def critically_damped_derivative(self, t, omega, displacement, velocity):
    r"""Critical damping for movement.

    I.e., x(t) = (A + Bt) \exp(-\omega t) with A = x(0), B = x'(0) + \omega x(0)

    See
      https://en.wikipedia.org/wiki/Damping#Critical_damping_.28.CE.B6_.3D_1.29
    for details.

    Args:
      t: A float representing time.
      omega: The undamped natural frequency.
      displacement: The initial displacement at, x(0) in the above equation.
      velocity: The initial velocity, x'(0) in the above equation

    Returns:
       The velocity x'(t).
    """
    a = displacement
    b = velocity + omega * displacement
    return (b - omega * t * (a + t * b)) * np.exp(-omega * t)

  def step(self, reward, unused_frame):
    """Gets an image state and a reward, returns an action."""
    self.rewards += reward

    action = (self.maxs - self.mins) * np.random.random_sample(
        size=[len(self.action_spec)]) + self.mins

    # Compute the 'velocity' 1 time unit after a critical damped force
    # dragged us towards the random `action`, given our current velocity.
    self.velocity = self.critically_damped_derivative(1, self.omega, action,
                                                      self.velocity)

    # Since walk and strafe are binary, we need some additional memory to
    # smoothen the movement. Adding half of action from the last step works.
    self.action = self.velocity / self.velocity_scaling + 0.5 * self.action

    # Fire with p = 0.01 at each step
    self.action[self.indices['FIRE']] = int(np.random.random() > 0.99)

    # Jump/crouch with p = 0.005 at each step
    self.action[self.indices['JUMP']] = int(np.random.random() > 0.995)
    self.action[self.indices['CROUCH']] = int(np.random.random() > 0.995)

    # Clip to the valid range and convert to the right dtype
    return self.clip_action(self.action)

  def clip_action(self, action):
    return np.clip(action, self.mins, self.maxs).astype(np.intc)

  def reset(self):
    self.velocity = np.zeros([len(self.action_spec)])
    self.action = np.zeros([len(self.action_spec)])


def run(length, width, height, fps, level, record, demo, demofiles, video):
  """Spins up an environment and runs the random agent."""
  config = {
      'fps': str(fps),
      'width': str(width),
      'height': str(height)
  }
  if record:
    config['record'] = record
  if demo:
    config['demo'] = demo
  if demofiles:
    config['demofiles'] = demofiles
  if video:
    config['video'] = video
  env = deepmind_lab.Lab(level, ['RGB_INTERLEAVED'], config=config)

  env.reset()

  # Starts the random spring agent. As a simpler alternative, we could also
  # use DiscretizedRandomAgent().
  agent = SpringAgent(env.action_spec())

  reward = 0

  for _ in six.moves.range(length):
    if not env.is_running():
      print('Environment stopped early')
      env.reset()
      agent.reset()
    obs = env.observations()
    action = agent.step(reward, obs['RGB_INTERLEAVED'])
    reward = env.step(action, num_steps=1)

  print('Finished after %i steps. Total reward received is %f'
        % (length, agent.rewards))


if __name__ == '__main__':
  parser = argparse.ArgumentParser(description=__doc__)
  parser.add_argument('--length', type=int, default=1000,
                      help='Number of steps to run the agent')
  parser.add_argument('--width', type=int, default=80,
                      help='Horizontal size of the observations')
  parser.add_argument('--height', type=int, default=80,
                      help='Vertical size of the observations')
  parser.add_argument('--fps', type=int, default=60,
                      help='Number of frames per second')
  parser.add_argument('--runfiles_path', type=str, default=None,
                      help='Set the runfiles path to find DeepMind Lab data')
  parser.add_argument('--level_script', type=str,
                      default='tests/empty_room_test',
                      help='The environment level script to load')
  parser.add_argument('--record', type=str, default=None,
                      help='Record the run to a demo file')
  parser.add_argument('--demo', type=str, default=None,
                      help='Play back a recorded demo file')
  parser.add_argument('--demofiles', type=str, default=None,
                      help='Directory for demo files')
  parser.add_argument('--video', type=str, default=None,
                      help='Record the demo run as a video')

  args = parser.parse_args()
  if args.runfiles_path:
    deepmind_lab.set_runfiles_path(args.runfiles_path)
  run(args.length, args.width, args.height, args.fps, args.level_script,
      args.record, args.demo, args.demofiles, args.video)
