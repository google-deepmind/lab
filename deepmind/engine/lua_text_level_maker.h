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
// A small Lua class that exposes the text-level translation functionality to
// Lua code.

#ifndef DML_DEEPMIND_ENGINE_LUA_TEXT_LEVEL_MAKER_H_
#define DML_DEEPMIND_ENGINE_LUA_TEXT_LEVEL_MAKER_H_

#include <random>
#include <string>

#include "deepmind/level_generation/compile_map.h"
#include "deepmind/lua/class.h"
#include "deepmind/lua/lua.h"
#include "deepmind/lua/n_results_or.h"
#include "public/level_cache_types.h"

namespace deepmind {
namespace lab {

class LuaTextLevelMaker : public lua::Class<LuaTextLevelMaker> {
  friend class Class;
  static const char* ClassName();

 public:
  // Constructed with path to where DeepMind Lab assets are stored.
  // And the `output_folder` contains the name of an existing, writable
  // directory into which files can be stored. This directory will also contain
  // the final .pk3 file.
  explicit LuaTextLevelMaker(std::string self,
                             std::string output_folder,
                             bool use_local_level_cache,
                             bool use_global_level_cache,
                             DeepMindLabLevelCacheParams level_cache_params,
                             std::uint32_t mixer_seed);

  // Registers MapFromTextLevel as "mapFromTextLevel".
  static void Register(lua_State* L);

  // Creates a map from text level input (see text_level.md for the concepts).
  //
  // Takes one single argument that is table, and returns the full path of the
  // created .pk3 file.
  //
  // The argument table expects the following fields:
  //
  //   * entityLayer
  //   * variationsLayer [optional]
  //   * outputDir
  //   * mapName [optional]
  //   * callback [optional]
  //
  // The fields entityLayer and variationsLayer contain the text map strings as
  // described in the text level generation documentation.
  // The name of the generated map file will either be the value of mapName, if
  // that field was provided, or otherwise 'luamap'.
  //
  // The optional callback field may contain a custom entity handler function.
  // See deepmind/level_generation/text_level/lua_bindings.h for details.
  //
  // [-1, +1, e]
  lua::NResultsOr MapFromTextLevel(lua_State* L);

  // Pushes a LuaRandom object onto the stack that views the map maker's pseudo-
  // random number generation facility. That object may be used to reseed the
  // generator so that the map generation is reproducible.
  //
  // [0, +1, -]
  lua::NResultsOr ViewRandomness(lua_State* L);

 private:
  std::mt19937_64 prng_;
  std::uint32_t mixer_seed_;
  MapCompileSettings settings_;
  const std::string rundir_;
  const std::string output_folder_;
};

}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_ENGINE_LUA_TEXT_LEVEL_MAKER_H_
