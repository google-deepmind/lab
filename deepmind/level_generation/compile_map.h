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
// C++ bindings to the map compilation shell script.

#ifndef DML_DEEPMIND_LEVEL_GENERATION_COMPILE_MAP_H_
#define DML_DEEPMIND_LEVEL_GENERATION_COMPILE_MAP_H_

#include <string>
#include "public/level_cache_types.h"

namespace deepmind {
namespace lab {
namespace internal {

std::string CalculateMd5(const std::string& file_name);

}  // namespace internal

struct MapCompileSettings {
  // Required if level needs to be traversed by bots.
  bool generate_aas = true;

  // Optional path to the map file. (If not empty the file at this location is
  // copied to <base>.map.)
  std::string map_source_location;

  // Which level caches to use if any. A level is searched for in the
  // local cache iff enabled and then the global cache iff enabled.
  // If found in the local cache, it is made available.
  // If found in the global cache, it is copied to the local cache iff enabled
  // and made available.
  // If not found, the level is generated and copied to enabled level caches.
  bool use_local_level_cache = false;
  bool use_global_level_cache = true;

  DeepMindLabLevelCacheParams level_cache_params = {nullptr, nullptr};
};

// Runs the map compiler for the map <base>.map, producing <base>.pk3.
// The rundir parameter contains the name of base directory of the executable,
// with respect to which the compilation script is located. The compile_settings
// parameter provides additional options for the compilation (see above).
//
// Returns whether the compilation succeeded.
bool RunMapCompileFor(const std::string& rundir, const std::string& base,
                      const MapCompileSettings& compile_settings);

}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_LEVEL_GENERATION_COMPILE_MAP_H_
