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

#include "absl/container/flat_hash_map.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "deepmind/engine/lua_random.h"
#include "deepmind/level_generation/compile_map.h"
#include "deepmind/level_generation/text_level/lua_bindings.h"
#include "deepmind/lua/call.h"
#include "deepmind/lua/push.h"
#include "deepmind/lua/read.h"
#include "deepmind/lua/table_ref.h"
#include "deepmind/util/files.h"

namespace deepmind {
namespace lab {
namespace {

// Theme based on a table of functions returned from Lua.
// A texture is defined by a table of the form:
// {
//    tex = 'map/poltergeist' -- Relative to assets directory.
//    width  = '1024'         -- Source width of texture.
//    height = '1024'         -- Source height of texture.
//    scale = '1.0'           -- Amount to scale texture.
//    angle = '0.0'           -- Amount to rotate texture in degrees.
// }
// Returns a table of textures for the variation.
// It shall return a complete table for variation 'default'.
// If no specific texture is specifed for a variation the 'default' is used.
// function theme:mazeVariation(variation)
//   return {
//       floor = textureFloor,     -- Floor texture.
//       ceiling = textureCeiling, -- Ceiling texture when there is no sky box.
//       wallN = textureWallN,     -- North wall texture. (Top of text map.)
//       wallE = textureWallE,     -- East wall texture. (Right of text map.)
//       wallS = textureWallS,     -- South wall texture. (Bottom of text map.)
//       wallW = textureWallW,     -- West wall texture. (Right of text map.)
//       riser = textureRiser,     -- Wall texture surounding platforms.
//       tread = textureTread,     -- Texture between riser and floor texture.
//   }
// end

// 'args' is an array of tables of the form:
// {
//     index = 1,            -- Index of table in args.
//     i = 1,                -- Cell row.
//     j = 1,                -- Cell col,
//     variation = 'default' -- Character of underlying variation or 'default'.
//     direction -- One of N, E, S, W wall side.
// }
//
// function theme:placeWallDecals(args)
//
//   return {
//       {
//           location = 1 -- Index location in args.
//           texture = tex -- Texture as defined above.
//       }, ...
//   }
// end
// A model is defined by a table of the form:
// {
//     mod = ''    -- Name of model (md3) relative to assets directory.
//     scale = 1.0 -- Amount to scale the model.
//     angle = 0.0 -- Amount to rotate the model in degrees.
// }
//
// 'args' is an array of tables of the form:
// {
//     index = 1,            -- Index of table in args.
//     i = 1,                -- Cell row.
//     j = 1,                -- Cell col,
//     variation = 'default' -- Character of underlying variation or 'default'.
// }
//
// function theme:placeFloorModels(args)
//   return {
//       {
//           index = 1 -- Index of location in args.
//           model = model -- Model as defined above.
//       }, ...
//   }
// end
class LuaTheme : public Theme {
 public:
  LuaTheme(lua_State* state, lua::TableRef theme)
      : lua_state_(state), theme_(std::move(theme)) {}

  Texture floor(int variation) override {
    return ReadThemeTexture("floor", variation);
  }

  Texture wall(int variation, Direction direction) override {
    switch (direction) {
      case Direction::North:
        return ReadThemeTexture("wallN", variation);
      case Direction::East:
        return ReadThemeTexture("wallE", variation);
      case Direction::South:
        return ReadThemeTexture("wallS", variation);
      case Direction::West:
        return ReadThemeTexture("wallW", variation);
    }
  }

  Texture ceiling(int variation) override {
    return ReadThemeTexture("ceiling", variation);
  }

  Texture platform_riser() override { return ReadThemeTexture("riser", 0); }

  Texture platform_tread() override { return ReadThemeTexture("tread", 0); }

  std::vector<WallDecoration> WallDecorations(
      const std::vector<WallArtLocation>& wall_locations) override {
    theme_.PushMemberFunction("placeWallDecals");
    if (lua_isnil(lua_state_, -2)) {
      lua_pop(lua_state_, 2);
      return {};
    }

    lua::TableRef wall_locs = lua::TableRef::Create(lua_state_);
    for (std::size_t i = 0, e = wall_locations.size(); i < e; ++i) {
      const auto& wall_loc = wall_locations[i];
      lua::TableRef wall_loc_table = lua::TableRef::Create(lua_state_);
      wall_loc_table.Insert("index", i + 1);
      wall_loc_table.Insert("i", wall_loc.cell.x());
      wall_loc_table.Insert("j", wall_loc.cell.y());
      wall_loc_table.Insert("variation", wall_loc.variation);
      switch (wall_loc.direction) {
        case Direction::North:
          wall_loc_table.Insert("direction", "N");
          break;
        case Direction::East:
          wall_loc_table.Insert("direction", "E");
          break;
        case Direction::South:
          wall_loc_table.Insert("direction", "S");
          break;
        case Direction::West:
          wall_loc_table.Insert("direction", "W");
          break;
      }
      wall_locs.Insert(i + 1, wall_loc_table);
    }
    lua::Push(lua_state_, wall_locs);
    auto result = lua::Call(lua_state_, 2);
    CHECK(result.ok()) << "[Theme.placeWallDecals] - " << result.error();
    CHECK(result.n_results() == 0 || result.n_results() == 1)
        << "[Theme.placeWallDecals] - Must return a table of locations";
    std::vector<WallDecoration> decs;
    if (result.n_results() == 1) {
      lua::TableRef wall_decs;
      lua::Read(lua_state_, -1, &wall_decs);
      decs.reserve(wall_decs.ArraySize());
      for (std::size_t i = 0, e = wall_decs.ArraySize(); i < e; ++i) {
        std::size_t index = 1;
        lua::TableRef wall_dec;
        CHECK(wall_decs.LookUp(i + 1, &wall_dec))
            << "[Theme.placeWallDecals] - Must return array of tables.";
        CHECK(wall_dec.LookUp("index", &index))
            << "[Theme.placeWallDecals] - Missing number 'index'!";
        CHECK(1 <= index && index <= wall_locations.size())
            << "[Theme.placeWallDecals] - 'index' is out of bounds!";
        lua::TableRef decal;
        CHECK(wall_dec.LookUp("decal", &decal))
            << "[Theme.placeWallDecals] - Missing texture 'decal'!";
        decs.push_back({wall_locations[index - 1], ReadTexture(decal)});
      }
    }
    lua_pop(lua_state_, result.n_results());

    return decs;
  }

  std::vector<FloorDecoration> FloorDecorations(
      const std::vector<FloorArtLocation>& floor_locations) override {
    std::vector<FloorDecoration> decs;
    theme_.PushMemberFunction("placeFloorModels");
    if (lua_isnil(lua_state_, -2)) {
      lua_pop(lua_state_, 2);
      return decs;
    }
    lua::TableRef floor_locs = lua::TableRef::Create(lua_state_);
    for (std::size_t i = 0, e = floor_locations.size(); i < e; ++i) {
      lua::TableRef floor_loc_table = lua::TableRef::Create(lua_state_);
      floor_loc_table.Insert("index", i + 1);
      floor_loc_table.Insert("i", floor_locations[i].cell.x());
      floor_loc_table.Insert("j", floor_locations[i].cell.y());
      floor_loc_table.Insert("variation", floor_locations[i].variation);
      floor_locs.Insert(i + 1, floor_loc_table);
    }
    lua::Push(lua_state_, floor_locs);
    auto result = lua::Call(lua_state_, 2);
    CHECK(result.ok()) << "[Theme.placeFloorModels] - " << result.error();
    CHECK(result.n_results() == 0 || result.n_results() == 1)
        << "[Theme.placeFloorModels] - Must return an array of tables";

    if (result.n_results() == 1) {
      lua::TableRef floor_decs;
      lua::Read(lua_state_, -1, &floor_decs);
      decs.reserve(floor_decs.ArraySize());
      for (std::size_t i = 0, e = floor_decs.ArraySize(); i < e; ++i) {
        std::size_t index = 1;
        lua::TableRef floor_dec;
        CHECK(floor_decs.LookUp(i + 1, &floor_dec))
            << "[Theme.placeFloorModels] - Must return an array of tables.";
        CHECK(floor_dec.LookUp("index", &index))
            << "[Theme.placeFloorModels] - 'index' is missing!";
        CHECK(1 <= index && index <= floor_locations.size())
            << "[Theme.placeFloorModels] - 'index' is out of bounds!";
        lua::TableRef model;
        CHECK(floor_dec.LookUp("model", &model))
            << "[Theme.placeFloorModels] - 'model' table must be supplied.";
        decs.push_back({floor_locations[index - 1], ReadModel(model)});
      }
    }
    lua_pop(lua_state_, result.n_results());
    return decs;
  }

 private:
  Texture ReadThemeTexture(const std::string& name, int variation) {
    auto pair = theme_cache_.try_emplace(variation, lua::TableRef());
    if (pair.second) {
      theme_.PushMemberFunction("mazeVariation");
      const char variation_char = variation;
      if (variation == 0) {
        lua::Push(lua_state_, "default");
      } else {
        lua_pushlstring(lua_state_, &variation_char, 1);
      }
      auto result = lua::Call(lua_state_, 2);
      CHECK(result.ok()) << "[mazeVariation] - " << result.error();
      bool table_exists = lua::Read(lua_state_, -1, &pair.first->second);
      CHECK (table_exists || variation != 0)
          << "[mazeVariation] - Must return a table for variation 'default'";
      lua_pop(lua_state_, result.n_results());
      if (!table_exists) {
        return ReadThemeTexture(name, 0);
      }
    }
    lua::TableRef texture;
    if (pair.first->second.LookUp(name, &texture)) {
      return ReadTexture(texture);
    } else {
      if (variation == 0) {
        return {"map/poltergeist", 64, 64, 1.0};
      } else {
        return ReadThemeTexture(name, 0);
      }
    }
  }

  Texture ReadTexture(const lua::TableRef& texture) {
    std::string name;
    int sizeWidth = 1024;
    int sizeHeight = 1024;
    double scale = 1.0;
    double angle = 0.0;
    if (!texture.LookUp("tex", &name)) {
      name = "map/poltergeist";
    }
    texture.LookUp("width", &sizeWidth);
    texture.LookUp("height", &sizeHeight);
    texture.LookUp("scale", &scale);
    texture.LookUp("angle", &angle);
    return Texture{std::move(name), sizeWidth, sizeHeight, scale, angle};
  }

  Model ReadModel(const lua::TableRef& model) {
    std::string name;
    double scale = 1.0;
    double angle = 0.0;
    model.LookUp("mod", &name);
    model.LookUp("scale", &scale);
    model.LookUp("angle", &angle);
    return Model{std::move(name), scale, angle};
  }

  absl::flat_hash_map<int, lua::TableRef> theme_cache_;
  lua_State* lua_state_;
  lua::TableRef theme_;
};

// An entity callback that handles nothing.
bool NoOp(std::size_t, std::size_t, char,
          const MapSnippetEmitter&, std::vector<std::string>*) {
  return false;
}

}  // namespace

LuaTextLevelMaker::LuaTextLevelMaker(
    std::string self, std::string output_folder, bool use_local_level_cache,
    bool use_global_level_cache, DeepMindLabLevelCacheParams level_cache_params,
    std::uint32_t mixer_seed)
    : prng_(0),
      mixer_seed_(mixer_seed),
      rundir_(std::move(self)),
      output_folder_(std::move(output_folder)) {
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

  TextLevelSettings level_settings;
  std::string ents, vars, mapstr, map_name, theme;

  if (!args.LookUp("entityLayer", &ents))
    return "Bad or missing arg 'entityLayer'";
  args.LookUp("variationsLayer", &vars);  // optional, no error if missing
  if (!args.LookUp("mapName", &map_name))
    map_name = "luamap";
  // optional, no error if missing
  args.LookUp("skyboxTextureName", &level_settings.skybox_texture_name);
  args.LookUp("ceilingHeight", &level_settings.ceiling_height);
  args.LookUp("drawDefaultLayout", &level_settings.draw_default_layout);
  double cell_size_game_units = 100.0;
  args.LookUp("cellSize", &cell_size_game_units);
  level_settings.cell_size = cell_size_game_units /
      map_builder::kWorldToGameUnits;
  lua::TableRef custom_theme;

  if (args.LookUp("theme", &custom_theme)) {
    level_settings.theme.reset(new LuaTheme(L, std::move(custom_theme)));
  }

  std::string game_dir = output_folder_ + "/baselab";
  if (!util::MakeDirectory(game_dir)) {
    return "Failed to create output directory: " + game_dir;
  }

  const std::string base = game_dir + "/" + map_name;

  args.LookUpToStack("callback");

  if (lua_isnil(L, -1)) {
    mapstr = TranslateTextLevel(ents, vars, &prng_, NoOp, &level_settings);
  } else {
    using namespace std::placeholders;
    auto user_cb =
        std::bind(LuaCustomEntityCallback, L, -1, _1, _2, _3, _4, _5);
    mapstr = TranslateTextLevel(ents, vars, &prng_, user_cb, &level_settings);
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
  LuaRandom::CreateObject(L, &prng_, mixer_seed_);
  return 1;
}

}  // namespace lab
}  // namespace deepmind
