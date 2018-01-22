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
// Entry point for text level map generation.

#ifndef DML_DEEPMIND_LEVEL_GENERATION_TEXT_LEVEL_TRANSLATE_TEXT_LEVEL_H_
#define DML_DEEPMIND_LEVEL_GENERATION_TEXT_LEVEL_TRANSLATE_TEXT_LEVEL_H_

#include <cstddef>
#include <functional>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include "deepmind/level_generation/text_level/text_level_settings.h"

namespace deepmind {
namespace lab {

// A utility class that users can call into to produce snippets of custom .map
// data. See below for context.
class MapSnippetEmitter {
 public:
  // Emits an entity with the given class name and set of attributes located in
  // the centre of the the (i, j) cell and at a given height.
  std::string AddEntity(
      double i, double j, double height, std::string class_name,
      const std::vector<std::pair<std::string, std::string>>& attributes) const;

  // Emits a spawn point in the (i, j) cell and at a given height.
  std::string AddSpawn(double i, double j, double height,
                       double angle_rad) const;

  // Emits a door cell. There are two possible directions for a door, labeled
  // 'I' and 'H'. A door if type 'I' at cell (i, j) can be traversed in the
  // east-west direction (i.e. it connects cells (i, j - 1) and (i, j + 1)),
  // and a door of type 'H' can be traversed in the north-south direction (i.e.
  // it connects cells (i - 1, j) and (i + 1, j)):
  //
  //    ********************
  //    *          I       *
  //    ****************H***
  //                 *     *
  //                 *******
  //
  std::string AddDoor(double i, double j, char direction) const;

  // Emits a fence door cell. Fence door cells are very similar to door cells,
  // except that the fence doors are composed of a series of blocks so that we
  // can see through them.
  std::string AddFenceDoor(double i, double j, char direction) const;

  // Emits a platform cell at specified position and height. The platform
  // surface size is a whole maze cell.
  // A platform is composed by several layers from top to bottom:
  //   - A top thin layer with a floor texture.
  //   - An intermediate layer with black texture, to enphasize the platform
  //     borders.
  //   - A plaform base, with a brick-like textures.
  //   - An invisible column from the bottom of the platform down to the ground
  //     which prevents the player from steping below other platforms.
  std::string AddPlatform(double i, double j, double height) const;

  // Emits an invisible column all the way from the ground to the specified
  // height. The column section will be a whole maze cell.
  std::string AddGlassColumn(double i, double j, double height) const;

 protected:
  MapSnippetEmitter() = default;
  ~MapSnippetEmitter() = default;

  MapSnippetEmitter(const MapSnippetEmitter&) = delete;
  void operator=(const MapSnippetEmitter&) = delete;
};

// Translates a text level provided as a multi-line level text string and a
// variations string into .map format.
//
// The user-provided callback is used to add domain-specific content to the map.
// The callback is called for every non-wall, non-empty cell (i, j) as
//
//    callback(i, j, value, &v)
//
// where "value" is the entity value of that cell (any character value that is
// neither '*' nor ' '), and v is a vector of strings. The return value states
// whether the user wants to handle the given entity. If true, the contents of
// v are appended to the final map output verbatim. If false, a default action
// is taken for certain entities:
//
//   * 'P' creates a spawn point.
//   * 'I' and 'H' create doors.
//
// A random number generator must be provided which is used to select variations
// and decorations.
//
// Additional parameters for level generation (e.g. use skybox) can be specified
// via level_settings.
//
// See documentation for details.

using TranslateTextLevelCallback = std::function<bool(
    std::size_t,                    // i coordinate
    std::size_t,                    // j coordinate
    char,                           // entity value
    const MapSnippetEmitter&,       // helper object to create .map data
    std::vector<std::string>*)>;    // sink for user-produced .map data

std::string TranslateTextLevel(
    std::string level_text,
    std::string variations_text,
    std::mt19937_64* rng,
    const TranslateTextLevelCallback& callback,
    TextLevelSettings* level_settings);

}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_LEVEL_GENERATION_TEXT_LEVEL_TRANSLATE_TEXT_LEVEL_H_
