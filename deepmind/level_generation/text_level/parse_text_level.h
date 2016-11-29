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
// ParseTextLevel: Internal transformation function that processes text level
// data into the internal Maze structure.

#ifndef DML_DEEPMIND_LEVEL_GENERATION_TEXT_LEVEL_PARSE_TEXT_LEVEL_H_
#define DML_DEEPMIND_LEVEL_GENERATION_TEXT_LEVEL_PARSE_TEXT_LEVEL_H_

#include <string>

#include "deepmind/level_generation/text_level/grid_maze.h"

namespace deepmind {
namespace lab {

// Parses a text level provided as a multi-line level text string and a
// variations string into the internal "GridMaze" structure that is used
// to translate the text level.
//
// The variations layer may be empty, in which case no variations are applied.
//
// See documentation for details.
GridMaze ParseTextLevel(std::string level_text, std::string variations_text);

}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_LEVEL_GENERATION_TEXT_LEVEL_PARSE_TEXT_LEVEL_H_
