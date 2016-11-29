// Copyright (C) 2016 Google Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
////////////////////////////////////////////////////////////////////////////////

#include "deepmind/level_generation/text_level/parse_text_level.h"

#include <string>
#include <utility>

#include "deepmind/level_generation/text_level/char_grid.h"

namespace deepmind {
namespace lab {

GridMaze ParseTextLevel(std::string level_text, std::string variations_text) {
  if (variations_text.empty()) variations_text.push_back(' ');
  CharGrid grid(std::move(level_text));
  CharGrid vars(std::move(variations_text));
  GridMaze maze(grid.height(), grid.width());

  auto IsWall = [](char c) -> bool { return c == '*' || c == '\0'; };

  // Break vertical walls.
  for (std::size_t i = 0; i < grid.height(); ++i) {
    bool old_wall = IsWall(grid.CellAt(i, 0));
    for (std::size_t j = 0; j + 1 < grid.width(); ++j) {
      bool new_wall = IsWall(grid.CellAt(i, j + 1));
      if (old_wall == new_wall) {
        maze.CreateOpening(i, j + 0, GridMaze::Direction::kEast);
        maze.CreateOpening(i, j + 1, GridMaze::Direction::kWest);
      }
      old_wall = new_wall;
    }
  }

  // Break horizontal walls.
  for (std::size_t j = 0; j < grid.width(); ++j) {
    bool old_wall = IsWall(grid.CellAt(0, j));
    for (std::size_t i = 0; i + 1 < grid.height(); ++i) {
      bool new_wall = IsWall(grid.CellAt(i + 1, j));
      if (old_wall == new_wall) {
        maze.CreateOpening(i + 0, j, GridMaze::Direction::kSouth);
        maze.CreateOpening(i + 1, j, GridMaze::Direction::kNorth);
      }
      old_wall = new_wall;
    }
  }

  for (std::size_t i = 0; i < grid.height(); ++i) {
    for (std::size_t j = 0; j < grid.width(); ++j) {
      char var = vars.CellAt(i, j);

      // Portabilibty note: This code assumes that letters A-Z are encoded
      // contiguously (e.g. as in ASCII).
      if (var < 'A' || 'Z' < var) var = '\0';

      maze.SetEntityAt(i, j, grid.CellAt(i, j), var);
    }
  }

  return maze;
}

}  // namespace lab
}  // namespace deepmind
