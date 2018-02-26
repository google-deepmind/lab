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
"""A high level DM Lab controller class for maze levels."""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import numpy as np
from python.tests.utils import game_controller

_MAZE_CELL_SIZE = 100.
_WALL_SYMBOL = '*'
_PICKUP_SYMBOL = 'O'

REQUIRED_OBSERVATIONS = [
    'VEL.ROT', 'VEL.TRANS', 'DEBUG.POS.ROT', 'DEBUG.POS.TRANS',
    'DEBUG.MAZE.LAYOUT'
]


def _create_distance_map(maze, from_x, from_y, to_x, to_y):
  """Return a distance map to position: x,y."""
  if not (0 <= from_x < maze.shape[0] and 0 <= from_y < maze.shape[1]):
    return None

  if maze[from_x, from_y] != 0:
    return None

  # distance map contains -1 for unexplored cells and -2 for walls.
  distance_map = -np.copy(maze) - 1
  distance_map[from_x, from_y] = 0

  to_explore = [(from_x, from_y)]

  offsets = [[-1, 0], [1, 0], [0, -1], [0, 1]]

  distance = 1
  while to_explore:
    next_explore = []
    for pos in to_explore:
      for (offset_x, offset_y) in offsets:
        x = pos[0] + offset_x
        y = pos[1] + offset_y
        if (0 <= x < maze.shape[0] and 0 <= y < maze.shape[1] and
            distance_map[x, y] == -1):
          distance_map[x, y] = distance
          if to_x == x and to_y == y:
            return distance_map
          else:
            next_explore.append((x, y))

    to_explore = next_explore
    distance += 1

  return None


def _find_path(maze, from_x, from_y, to_x, to_y):
  """Return a path from (from_x, from_y) to (to_x, to_y) or None."""
  distance_map = _create_distance_map(maze, to_x, to_y, from_x, from_y)
  if distance_map is None or distance_map[from_x, from_y] < 0:
    return None

  path = []
  current_pos = [from_x, from_y]
  distance = distance_map[current_pos[0], current_pos[1]]

  offsets = [[-1, 0], [1, 0], [0, -1], [0, 1]]

  while distance != 0:
    for (offset_x, offset_y) in offsets:
      x = current_pos[0] + offset_x
      y = current_pos[1] + offset_y
      if (0 <= x < maze.shape[0] and 0 <= y < maze.shape[1] and
          distance_map[x, y] == distance - 1):
        path.append((x, y))
        current_pos = [x, y]
        distance -= 1
        break

  return path


class MazeGameController(object):
  """A high level game controller class for maze levels.

  The class allows navigating DMLab mazes using high level actions.
  """

  def __init__(self, env):
    """Initialize the controller."""
    self._env = env
    self._controller = game_controller.GameController(env)
    self._maze = None
    self._pickup_locations = None

  def maze_position(self):
    """Return location of the player in the maze."""
    pos = self._env.observations()['DEBUG.POS.TRANS']
    x, y = self._to_maze_coord(pos[0], pos[1])
    return np.array([x, y])

  def find_path(self, dest_x, dest_y, blocked=None):
    """Return path to the destination avoinding blocked cells."""
    start = self.maze_position()
    maze = self._get_maze()

    if blocked is not None:
      maze = np.copy(maze)
      for (x, y) in blocked:
        maze[x, y] = 1

    path = _find_path(maze, start[0], start[1], dest_x, dest_y)
    return path

  def follow_path(self, path):
    """Move the player along the path."""
    path = self._expand_path(path)

    if not path:
      return False

    for (prev_x, prev_y), (x, y), (next_x, next_y) in zip(
        path, path[1:], path[2:]):
      if next_x - x != x - prev_x or next_y - y != y - prev_y:
        world_x, world_y = self._to_world_coord(x, y)
        self._controller.move_to(world_x, world_y)

    (x, y) = path[-1]
    world_x, world_y = self._to_world_coord(x, y)
    self._controller.move_to(world_x, world_y)
    return True

  def _expand_path(self, path):
    """Return a new path that includes all intermediate steps."""
    (last_x, last_y) = self.maze_position()
    expanded_path = [(last_x, last_y)]
    maze = self._get_maze()
    for (x, y) in path:
      if last_x != x or last_y != y:
        local_path = _find_path(maze, last_x, last_y, x, y)
        if local_path:
          expanded_path += local_path
        else:
          return None
      last_x, last_y = x, y
    return expanded_path

  def move_to(self, dest_x, dest_y, blocked=None):
    """Move the player to the destination avoiding blocked cells."""
    path = self.find_path(dest_x, dest_y, blocked=blocked)
    if path is None:
      return False
    elif self.follow_path(path):
      return np.array_equal([dest_x, dest_y], self.maze_position())
    else:
      return False

  def pickup_location(self, index):
    """Return location of a pickpup."""
    locations = self._find_pickup_locations()
    assert index >= 0 and index < len(locations)
    return locations[index]

  def _get_maze(self):
    """Return the current maze as a numpy array (0 - corridors, 1 - walls)."""
    if self._maze is None:
      maze_str = self._env.observations()['DEBUG.MAZE.LAYOUT'].strip()
      lines = maze_str.split('\n')

      height = len(lines)
      width = 0
      for line in lines:
        width = max(width, len(line))

      maze = np.zeros((width, height), dtype=np.int32)

      for j, line in enumerate(lines):
        for i, cell in enumerate(line):
          if cell == _WALL_SYMBOL:
            maze[i, j] = 1
      self._maze = maze
    return self._maze

  def _to_world_coord(self, x, y):
    """Return x, y in the world space."""
    maze = self._get_maze()
    y = maze.shape[1] - y - 1
    return (float(x) + .5) * _MAZE_CELL_SIZE, (float(y) + .5) * _MAZE_CELL_SIZE

  def _to_maze_coord(self, x, y):
    """Return x, y in the maze space."""
    maze = self._get_maze()
    x = int(x / _MAZE_CELL_SIZE)
    y = int(y / _MAZE_CELL_SIZE)
    y = maze.shape[1] - y - 1
    return x, y

  def _find_pickup_locations(self):
    """Return locations of all pickups in the current maze."""
    if self._pickup_locations is None:
      maze_str = self._env.observations()['DEBUG.MAZE.LAYOUT'].strip()
      lines = maze_str.split('\n')

      self._pickup_locations = []
      for j, line in enumerate(lines):
        for i, cell in enumerate(line):
          if cell == _PICKUP_SYMBOL:
            self._pickup_locations.append((i, j))
    return self._pickup_locations
