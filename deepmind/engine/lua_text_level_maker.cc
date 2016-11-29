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

namespace deepmind {
namespace lab {
namespace {

// An entity callback that handles nothing.
bool NoOp(std::size_t, std::size_t, char,
          const MapSnippetEmitter&, std::vector<std::string>*) {
  return false;
}

}  // namespace

LuaTextLevelMaker::LuaTextLevelMaker(const std::string& self)
    : prng_(0),
      rundir_(self) {}

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
  lua::TableRef args;

  if (lua_gettop(L) != 2 || !lua::Read(L, 2, &args)) {
    return "Bad invocation, need exactly one (table) argument.";
  }

  std::string ents, vars, odir, mapstr, map_name;

  if (!args.LookUp("entityLayer", &ents))
    return "Bad or missing arg 'entityLayer'";
  if (!args.LookUp("outputDir", &odir))
    return "Bad or missing arg 'outputDir'";
  args.LookUp("variationsLayer", &vars);  // optional, no error if missing
  if (!args.LookUp("mapName", &map_name))
    map_name = "luamap";

  const std::string base = odir + "/" + map_name;

  args.LookUpToStack("callback");

  if (lua_isnil(L, -1)) {
    LOG(INFO) << "No custom entity handler provided, using defaults.";
    mapstr = TranslateTextLevel(ents, vars, &prng_, NoOp);
  } else {
    LOG(INFO) << "Using custom entity handler.";
    using namespace std::placeholders;
    auto user_cb = std::bind(LuaCustomEntityCallback, L, -1, _1, _2, _3, _4, _5);
    mapstr = TranslateTextLevel(ents, vars, &prng_, user_cb);
  }

  if (!(std::ofstream(base + ".map") << mapstr)) {
    return "Failed to create output file.";
  }

  if (!RunMapCompileFor(rundir_, base)) {
    return "Failed to compile map.";
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
