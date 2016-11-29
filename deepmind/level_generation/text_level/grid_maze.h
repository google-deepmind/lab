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
//
// GridMaze: An internal data structure to represent a "maze", a simple kind of
// map.

#ifndef DML_DEEPMIND_LEVEL_GENERATION_TEXT_LEVEL_GRID_MAZE_H_
#define DML_DEEPMIND_LEVEL_GENERATION_TEXT_LEVEL_GRID_MAZE_H_

#include <cstddef>
#include <vector>

namespace deepmind {
namespace lab {

// The GridMaze class is our internal representation of a map as we see it
// during the translation of a text level into the .map format. The map is
// stored as a rectangular grid of cells; each cell has openings that
// determine where the walls are placed in the output, a primary entity
// value denoting its contents, and a variation determining its looks.
class GridMaze {
 public:
  enum class Direction : unsigned char {
    kNone  = 0,
    kNorth = 1,
    kSouth = 2,
    kEast  = 4,
    kWest  = 8,
  };

  // A grid cell has a primary *value*, which is either "inaccessible" or not.
  // If a cell is not inaccessible, it can have a *variation* as well as a set
  // of *openings* to adjacent cells. The absence of an opening means that there
  // is a wall in that direction.
  struct Cell {
    static const char kInaccessible = '*';

    char value = kInaccessible;
    char variation = '\0';
    Direction opening = Direction::kNone;
  };

  explicit GridMaze(std::size_t height, std::size_t width)
      : cells_(height * width), width_(width) {}

  std::size_t height() const { return cells_.size() / width_; }
  std::size_t width() const { return width_; }

  // Adds an opening in the given direction to the cell at (i, j). Since the
  // presence of an opening translates into the absence of a wall, adjacent
  // cells should consistently add openings in symmetric pairs.
  void CreateOpening(std::size_t i, std::size_t j, Direction dir) {
    Direction& op = cells_[i * width_ + j].opening;
    op = Direction(
        static_cast<unsigned char>(op) | static_cast<unsigned char>(dir));
  }

  // Sets a cell to a given entity and optional variation. The variation is only
  // set if the cell is not inaccessible.
  void SetEntityAt(std::size_t i, std::size_t j, char entity, char variation) {
    if (entity == '\0')
      entity = Cell::kInaccessible;

    Cell& cell = cells_[i * width_ + j];
    cell.value = entity;

    if (entity != Cell::kInaccessible)
      cell.variation = variation;
  }

  // Calls f(i, j, cell) for each (i, j).
  template <typename F>
  F Visit(F f) const {
    for (std::size_t k = 0, i = 0, j = 0; k != cells_.size(); ++k, ++j) {
      if (j == width_) { j = 0; ++i; }
      f(i, j, cells_[k]);
    }
    return f;
  }

 private:
  std::vector<Cell> cells_;
  std::size_t width_;
};

}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_LEVEL_GENERATION_TEXT_LEVEL_GRID_MAZE_H_
