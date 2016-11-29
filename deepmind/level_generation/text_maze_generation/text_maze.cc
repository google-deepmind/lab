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

#include "deepmind/level_generation/text_maze_generation/text_maze.h"

namespace deepmind {
namespace lab {
namespace maze_generation {

TextMaze::TextMaze(Size extents) : area_{{0, 0}, extents} {
  std::string level_layer(area_.size.height * (area_.size.width + 1), '*');
  std::string variations_layer(area_.size.height * (area_.size.width + 1), '.');
  for (int i = 0; i < area_.size.height; ++i) {
    int text_idx = ToTextIdx(i, area_.size.width);
    level_layer[text_idx] = '\n';
    variations_layer[text_idx] = '\n';
  }
  text_[kEntityLayer] = std::move(level_layer);
  text_[kVariationsLayer] = std::move(variations_layer);
}

}  // namespace maze_generation
}  // namespace lab
}  // namespace deepmind
