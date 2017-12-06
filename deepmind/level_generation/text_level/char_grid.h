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
// CharGrid: A 2D view on a string of characters.

#ifndef DML_DEEPMIND_LEVEL_GENERATION_TEXT_LEVEL_CHAR_GRID_H_
#define DML_DEEPMIND_LEVEL_GENERATION_TEXT_LEVEL_CHAR_GRID_H_

#include <cstddef>
#include <string>
#include <vector>

#include "absl/strings/string_view.h"

namespace deepmind {
namespace lab {

// CharGrid is a thin wrapper around a string that exposes an immutable
// (row, column)-access to grid defined by a multiline string. The size
// of the grid is implied: The grid heigth is the number of lines, and
// the grid width is the length of the longest line.
//
// Accessing the grid out of bounds is allowed and results in a null
// character.
class CharGrid {
 public:
  // Constructs a CharGrid from the provided multi-line string.
  explicit CharGrid(std::string text);

  // Extents of the implied grid.
  std::size_t height() const { return rows_.size(); }
  std::size_t width() const { return width_; }

  // Value of the cell at row i, column j if (i, j) is within bounds,
  // or the null character value otherwise.
  char CellAt(std::size_t i, std::size_t j) const {
    return i < height() && j < rows_[i].size() ? rows_[i][j] : '\0';
  }

 private:
  std::string raw_data_;
  std::vector<absl::string_view> rows_;
  std::size_t width_;
};


}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_LEVEL_GENERATION_TEXT_LEVEL_CHAR_GRID_H_
