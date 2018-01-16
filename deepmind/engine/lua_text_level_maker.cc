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

#include "deepmind/engine/lua_text_level_maker.h"

#include <fstream>
#include <functional>

#include "deepmind/engine/lua_random.h"
#include "deepmind/level_generation/compile_map.h"
#include "deepmind/level_generation/text_level/lua_bindings.h"
#include "deepmind/lua/push.h"
#include "deepmind/lua/read.h"
#include "deepmind/lua/table_ref.h"
#include "deepmind/util/files.h"

namespace deepmind {
namespace lab {
namespace {

// An entity callback that handles nothing.
bool NoOp(std::size_t, std::size_t, char,
          const MapSnippetEmitter&, std::vector<std::string>*) {
  return false;
}

}  // namespace

LuaTextLevelMaker::LuaTextLevelMaker(
    const std::string& self, const std::string& output_folder,
    bool use_local_level_cache, bool use_global_level_cache,
    DeepMindLabLevelCacheParams level_cache_params)
    : prng_(0), settings_{}, rundir_(self), output_folder_(output_folder) {
  settings_.use_local_level_cache = use_local_level_cache;
  settings_.use_global_level_cache = use_global_level_cache;
  settings_.level_cache_params = level_cache_params;
}

const char* LuaTextLevelMaker::ClassName() {
  return "deepmind.lab.TextLevelMaker";
}

void LuaTextLevelMaker::Register(lua_State* L) {
  const Class::Reg methods[] = {
    {"mapFromTextLevel", Member<&LuaTextLevelMaker::MapFromTextLevel>},
    {"randomGen", Member<&LuaTextLevelMaker::ViewRandomness>},
  };
  Class::Register(L, methods);
}

lua::NResultsOr LuaTextLevelMaker::MapFromTextLevel(lua_State* L) {
  MapCompileSettings settings = settings_;
  lua::TableRef args;

  if (lua_gettop(L) != 2 || !lua::Read(L, 2, &args)) {
    return "Bad invocation, need exactly one (table) argument.";
  }

  std::string ents, vars, mapstr, map_name, theme;

  if (!args.LookUp("entityLayer", &ents))
    return "Bad or missing arg 'entityLayer'";
  args.LookUp("variationsLayer", &vars);  // optional, no error if missing
  if (!args.LookUp("mapName", &map_name))
    map_name = "luamap";
  // optional, no error if missing

  std::string game_dir = output_folder_ + "/baselab";
  if (!util::MakeDirectory(game_dir)) {
    return "Failed to create output directory: " + game_dir;
  }

  const std::string base = game_dir + "/" + map_name;

  args.LookUpToStack("callback");

  if (lua_isnil(L, -1)) {
    mapstr = TranslateTextLevel(ents, vars, &prng_, NoOp);
  } else {
    using namespace std::placeholders;
    auto user_cb = std::bind(LuaCustomEntityCallback, L, -1, _1, _2, _3, _4, _5);
    mapstr = TranslateTextLevel(ents, vars, &prng_, user_cb);
  }

  if (!(std::ofstream(base + ".map") << mapstr)) {
    return "Failed to create output file.";
  }

  settings.generate_aas = false;
  args.LookUp("allowBots", &settings.generate_aas);

  if (!RunMapCompileFor(rundir_, base, settings)) {
    std::string error_message = "Failed to compile map.\n";
    error_message += "Map Name:\n";
    error_message += map_name;
    error_message += "Entity Layer:\n";
    error_message += ents;
    error_message += "Variations Layer:\n";
    error_message += vars;
    return error_message;
  }

  lua::Push(L, base + ".pk3");
  return 1;
}


lua::NResultsOr LuaTextLevelMaker::ViewRandomness(lua_State* L) {
  LuaRandom::CreateObject(L, &prng_);
  return 1;
}

}  // namespace lab
}  // namespace deepmind
