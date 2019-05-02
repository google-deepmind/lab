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

#include "deepmind/engine/lua_maze_generation.h"

#include <array>
#include <string>
#include <vector>

#include "deepmind/engine/lua_random.h"
#include "deepmind/level_generation/text_level/char_grid.h"
#include "deepmind/level_generation/text_maze_generation/algorithm.h"
#include "deepmind/level_generation/text_maze_generation/flood_fill.h"
#include "deepmind/lua/bind.h"
#include "deepmind/lua/call.h"
#include "deepmind/lua/push.h"
#include "deepmind/lua/read.h"
#include "deepmind/lua/table_ref.h"

namespace deepmind {
namespace lab {
namespace {

std::mt19937_64* GetRandomNumberGenerator(lua::TableRef* table,
                                          std::mt19937_64* seeded_rng,
                                          std::uint64_t mixer_seq) {
  std::mt19937_64* prng = nullptr;
  lua_State* L = table->LuaState();
  table->LookUpToStack("random");
  if (!lua_isnil(L, -1)) {
    LuaRandom* random = LuaRandom::ReadObject(L, -1);
    if (random != nullptr) {
      prng = random->GetPrbg();
    }
  }
  lua_pop(L, 1);
  if (prng == nullptr) {
    int seed = 0;
    if (table->LookUp("seed", &seed)) {
      seeded_rng->seed(static_cast<std::uint64_t>(seed) ^ mixer_seq);
      prng = seeded_rng;
    }
  }
  return prng;
}

}  // namespace

// Default maximum number of attempts to lay out rooms in
// LuaMazeGeneration::CreateRandom.
constexpr int kDefaultRetryCount = 1000;
constexpr int kGridWidth = 100;

class LuaRoom : public lua::Class<LuaRoom> {
 public:
  LuaRoom(std::vector<maze_generation::Pos> room) : room_(std::move(room)) {}

  static const char* ClassName() {
    return "deepmind.lab.LuaRoom";
  }

  static void Register(lua_State* L) {
    Class::Reg regs[] = {
        {"visit", &Class::Member<&LuaRoom::Visit>},
        {"size", &Class::Member<&LuaRoom::Size>},
    };
    Class::Register(L, regs);
  }

  // Implements size(), returning the number of cells within the room.
  // [0, 1, -]
  lua::NResultsOr Size(lua_State* L) {
    lua::Push(L, room_.size());
    return 1;
  }

  // Implements visit(f), where f will be called with the row and column for
  // each cell within the room.
  // [1, 0, e]
  lua::NResultsOr Visit(lua_State* L) {
    if (lua_gettop(L) != 2) {
      return "[visit] - Must provide function as argument!";
    }
    for (const auto& pos : room_) {
      lua_pushvalue(L, 2);
      lua::Push(L, pos.row + 1);
      lua::Push(L, pos.col + 1);
      auto result = lua::Call(L, 2);
      if (!result.ok()) {
        return result;
      }
      lua_pop(L, result.n_results());
    }
    return 0;
  }

 private:
  std::vector<maze_generation::Pos> room_;
};

// Bit toggle sequence applied to the 32 MSB of the 64bit seeds fed to the maze
// generation PRBGs, with the intention of creating disjoint seed subspaces for
// each different mixer_seed value as described in python_api.md
std::uint64_t LuaMazeGeneration::mixer_seq_ = 0;

const char* LuaMazeGeneration::ClassName() {
  return "deepmind.lab.LuaMazeGeneration";
}

lua::NResultsOr LuaMazeGeneration::Require(lua_State* L) {
  std::uintptr_t mixer_seed =
      reinterpret_cast<std::uintptr_t>(lua_touserdata(L, lua_upvalueindex(1)));
  mixer_seq_ = static_cast<std::uint64_t>(mixer_seed) << 32;
  auto table = lua::TableRef::Create(L);
  table.Insert("mazeGeneration", &lua::Bind<LuaMazeGeneration::Create>);
  table.Insert("randomMazeGeneration",
               &lua::Bind<LuaMazeGeneration::CreateRandom>);
  lua::Push(L, table);
  return 1;
}

lua::NResultsOr LuaMazeGeneration::Create(lua_State* L) {
  lua::TableRef table;
  if (!lua::Read(L, -1, &table)) {
    return  "[mazeGeneration] - Must be called with a table.";
  }
  if (table.Contains("entity")) {
    std::string entity_layer;
    if (!table.LookUp("entity", &entity_layer) || entity_layer.empty()) {
      return "[mazeGeneration] - 'entity' must be a non-empty string.";
    }
    std::string variations_layer;
    if (table.LookUp("variations", &variations_layer) &&
        !variations_layer.empty()) {
      CreateObject(L, maze_generation::FromCharGrid(
                          CharGrid(std::move(entity_layer)),
                          CharGrid(std::move(variations_layer))));
    } else {
      CreateObject(
          L, maze_generation::FromCharGrid(CharGrid(std::move(entity_layer))));
    }
  } else {
    int height = 0;
    int width = 0;
    if (!table.LookUp("height", &height) || height <= 0) {
      return "[mazeGeneration] - Must construct with positive width and height";
    }
    if (!table.LookUp("width", &width) || width <= 0) {
      return "[mazeGeneration] - Must construct with positive width and height";
    }
    CreateObject(L, maze_generation::Size{height, width});
  }
  return 1;
}

lua::NResultsOr LuaMazeGeneration::CreateRandom(lua_State* L) {
  lua::TableRef table;
  lua::Read(L, -1, &table);

  std::mt19937_64 seeded_rng;
  std::mt19937_64* prng =
      GetRandomNumberGenerator(&table, &seeded_rng, mixer_seq_);
  if (prng == nullptr) {
    return "[randomMazeGeneration] - Must construct with 'random' a random "
           "number generator. ('seed' is deprecated.)";
  }

  int height = 0;
  int width = 0;
  if (!table.LookUp("height", &height) || height <= 0 || height % 2 == 0) {
    return "[randomMazeGeneration] - Must construct with positive odd height";
  }
  if (!table.LookUp("width", &width) || width <= 0 || width % 2 == 0) {
    return "[randomMazeGeneration] - Must construct with positive odd width";
  }
  int max_rooms = 0;
  table.LookUp("maxRooms", &max_rooms);
  if (max_rooms < 0) {
    return "[randomMazeGeneration] - Must construct with non-negative "
           "maxRooms";
  }
  int max_variations = 26;
  table.LookUp("maxVariations", &max_variations);
  if (max_variations <= 0 || max_variations > 26) {
    return "[randomMazeGeneration] - Must construct with maxVariations in  "
           "[1,26]";
  }
  int room_min_size = 3;
  table.LookUp("roomMinSize", &room_min_size);
  if (room_min_size <= 0 || room_min_size % 2 == 0) {
    return "[randomMazeGeneration] - Must construct with positive odd "
           "roomMinSize";
  }
  int room_max_size = 7;
  table.LookUp("roomMaxSize", &room_max_size);
  if (room_max_size <= 0 || room_max_size % 2 == 0) {
    return "[randomMazeGeneration] - Must construct with positive odd "
           "roomMaxSize";
  }

  if (max_rooms > 0 &&
      (room_max_size + 2 > height || room_max_size + 2 > width)) {
    return absl::StrCat(
        "[randomMazeGeneration] - roomMaxSize must be less than width or "
        "height. roomMaxSize: ",
        room_max_size, " height: ", height, " width: ", width);
  }

  int retry_count = kDefaultRetryCount;
  table.LookUp("retryCount", &retry_count);
  if (retry_count <= 0) {
    return "[randomMazeGeneration] - Must construct with positive retryCount";
  }
  double extra_connection_probability = 0.05;
  table.LookUp("extraConnectionProbability", &extra_connection_probability);
  bool simplify = true;
  table.LookUp("simplify", &simplify);
  int room_spawn_count = 0;
  std::string spawn = "P";
  table.LookUp("roomSpawnCount", &room_spawn_count);
  table.LookUp("spawn", &spawn);
  if (spawn.length() != 1) {
    return "[randomMazeGeneration] - Must construct with single character as "
           "spawn entity";
  }
  int room_object_count = 0;
  std::string object = "G";
  table.LookUp("roomObjectCount", &room_object_count);
  table.LookUp("object", &object);
  if (object.length() != 1) {
    return "[randomMazeGeneration] - Must construct with single character as "
           "object entity";
  }
  bool has_doors = false;
  table.LookUp("hasDoors", &has_doors);

  maze_generation::TextMaze maze(maze_generation::Size{height, width});

  // Create random rooms.
  maze_generation::SeparateRectangleParams params{};
  params.min_size = maze_generation::Size{room_min_size, room_min_size};
  params.max_size = maze_generation::Size{room_max_size, room_max_size};
  params.retry_count = retry_count;
  params.max_rects = max_rooms;
  params.density = 1.0;
  const auto rects = MakeSeparateRectangles(maze.Area(), params, prng);
  const auto num_rooms = rects.size();
  for (unsigned int r = 0; r < num_rooms; ++r) {
    maze.VisitMutableIntersection(maze_generation::TextMaze::kEntityLayer,
                                  rects[r],
                                  [&maze, r](int i, int j, char* cell) {
                                    *cell = ' ';
                                    maze.SetCellId({i, j}, r + 1);
                                  });
  }

  // Fill the vacant space with corridors.
  FillSpaceWithMaze(num_rooms + 1, 0, &maze, prng);

  // Connect adjacent regions at least once.
  auto conns =
      RandomConnectRegions(-1, extra_connection_probability, &maze, prng);

  // Add variations.
  maze.VisitMutable(
      maze_generation::TextMaze::kVariationsLayer,
      [max_variations, &maze, num_rooms](int i, int j, char* cell) {
        auto id = maze.GetCellId({i, j});
        if (id > 0 && id <= num_rooms) {
          *cell = 'A' + (id - 1) % max_variations;
        }
      });


  // Simplify the maze if requested.
  if (simplify) {
    RemoveDeadEnds(' ', '*', {}, &maze);
    RemoveAllHorseshoeBends('*', {}, &maze);
  }

  // Add entities and spawn points.
  AddNEntitiesToEachRoom(rects, room_spawn_count, spawn[0], ' ', &maze, prng);
  AddNEntitiesToEachRoom(rects, room_object_count, object[0], ' ', &maze, prng);

  // Set each connection cell connection type.
  for (const auto& conn : conns) {
    char connection_type;
    // Set to wall if connected to nowhere.
    if (maze.GetCell(maze_generation::TextMaze::kEntityLayer,
                     conn.first + conn.second) == '*') {
      connection_type = '*';
    } else if (has_doors) {
      connection_type = (conn.second.d_col == 0) ? 'H' : 'I';
    } else {
      connection_type = ' ';
    }
    maze.SetCell(maze_generation::TextMaze::kEntityLayer, conn.first,
                 connection_type);
  }

  CreateObject(L, maze);
  return 1;
}

lua::NResultsOr LuaMazeGeneration::Rotate(lua_State* L) {
  int rotation = 0;
  if (lua_gettop(L) != 2 || !lua::Read(L, 2, &rotation)) {
    return "[rotate] - Must provide rotation";
  }
  maze_generation::TextMaze rotated = text_maze_.Rotate(rotation);
  CreateObject(L, maze_generation::TextMaze(rotated));
  return 1;
}

lua::NResultsOr LuaMazeGeneration::Paste(lua_State* L) {
  int row = 0;
  int col = 0;
  if (lua_gettop(L) != 4 || !lua::Read(L, 2, &row) || !lua::Read(L, 3, &col)) {
    return "[insert] - Must provide row, col";
  }
  LuaMazeGeneration* pasteMaze = ReadObject(L, 4);
  if (!pasteMaze) {
    return "[insert] - Must provide maze";
  }

  text_maze_.Paste(maze_generation::TextMaze::kEntityLayer, {row - 1, col - 1},
                   pasteMaze->text_maze_);
  return 1;
}

lua::NResultsOr LuaMazeGeneration::EntityLayer(lua_State* L) {
  lua::Push(L, text_maze_.Text(maze_generation::TextMaze::kEntityLayer));
  return 1;
}

lua::NResultsOr LuaMazeGeneration::VariationsLayer(lua_State* L) {
  lua::Push(L, text_maze_.Text(maze_generation::TextMaze::kVariationsLayer));
  return 1;
}

lua::NResultsOr LuaMazeGeneration::GetEntityCell(lua_State* L) {
  int row = 0;
  int col = 0;
  if (lua_gettop(L) != 3 || !lua::Read(L, 2, &row) || !lua::Read(L, 3, &col)) {
    return "[getEntityCell] - Must provide row, col";
  }
  char result[2] = "\0";
  result[0] = text_maze_.GetCell(maze_generation::TextMaze::kEntityLayer,
                                 {row - 1, col - 1});
  lua::Push(L, result);
  return 1;
}

lua::NResultsOr LuaMazeGeneration::SetEntityCell(lua_State* L) {
  int row = 0;
  int col = 0;
  std::string character;
  if (lua_gettop(L) != 4 || !lua::Read(L, 2, &row) || !lua::Read(L, 3, &col) ||
      !lua::Read(L, 4, &character) || character.size() != 1) {
    return "[setEntityCell] - Must provide row, col, character";
  }

  text_maze_.SetCell(maze_generation::TextMaze::kEntityLayer,
                     {row - 1, col - 1}, character[0]);
  return 0;
}

lua::NResultsOr LuaMazeGeneration::FillEntityRect(lua_State* L) {
  lua::TableRef table;
  lua::Read(L, -1, &table);

  int row = 0;
  int col = 0;
  int height = 0;
  int width = 0;
  std::string character;
  if (!table.LookUp("row", &row) ||
      !table.LookUp("col", &col) ||
      !table.LookUp("height", &height) ||
      !table.LookUp("width", &width) ||
      !table.LookUp("character", &character)) {
    return "[setEntityRect] - Must provide row, col, height, width, character";
  }

  text_maze_.FillRect(maze_generation::TextMaze::kEntityLayer,
                      {{row - 1, col - 1}, {height, width}}, character[0]);
  return 0;
}

lua::NResultsOr LuaMazeGeneration::GetVariationsCell(lua_State* L) {
  int row = 0;
  int col = 0;
  if (lua_gettop(L) != 3 || !lua::Read(L, 2, &row) || !lua::Read(L, 3, &col)) {
    return "[getVariationsCell] - Must provide row, col";
  }
  char result[2] = "\0";
  result[0] = text_maze_.GetCell(maze_generation::TextMaze::kVariationsLayer,
                                 {row - 1, col - 1});
  lua::Push(L, result);
  return 1;
}

lua::NResultsOr LuaMazeGeneration::SetVariationsCell(lua_State* L) {
  int row = 0;
  int col = 0;
  std::string character;
  if (lua_gettop(L) != 4 || !lua::Read(L, 2, &row) || !lua::Read(L, 3, &col) ||
      !lua::Read(L, 4, &character) || character.size() != 1) {
    return "[setVariationsCell] - Must provide row, col, character";
  }

  text_maze_.SetCell(maze_generation::TextMaze::kVariationsLayer,
                     {row - 1, col - 1}, character[0]);
  return 0;
}

lua::NResultsOr LuaMazeGeneration::FindRooms(lua_State* L) {
  std::vector<char> wall_chars;
  if (lua_gettop(L) == 1) {
    wall_chars = std::vector<char>{'*'};
  } else if (lua_gettop(L) == 2) {
    std::string user_wall_chars;
    if (!lua::Read(L, 2, &user_wall_chars)) {
      return "[findRooms] - Only accepts one optional argument; a string of "
             "wall characters.";
    }
    wall_chars.assign(user_wall_chars.begin(), user_wall_chars.end());
  } else {
    return "[findRooms] - Only accepts one optional argument; a string of wall "
           "characters.";
  }
  auto rooms = maze_generation::FindRooms(text_maze_, wall_chars);
  lua_createtable(L, rooms.size(), 0);
  for (std::size_t i = 0, e = rooms.size(); i != e; ++i) {
    LuaRoom::CreateObject(L, std::move(rooms[i]));
    lua_rawseti(L, -2, i + 1);
  }
  return 1;
}

lua::NResultsOr LuaMazeGeneration::Size(lua_State* L) {
  lua::Push(L, text_maze_.Area().size.height);
  lua::Push(L, text_maze_.Area().size.width);
  return 2;
}

lua::NResultsOr LuaMazeGeneration::ToWorldPos(lua_State* L) {
  int row = 0;
  int col = 0;
  if (lua_gettop(L) != 3 || !lua::Read(L, 2, &row) || !lua::Read(L, 3, &col)) {
    return "[toWorldPos] - Must provide row, col";
  }
  lua::Push(L, (col - 1) * kGridWidth + kGridWidth / 2);
  lua::Push(
      L, (text_maze_.Area().size.height - row) * kGridWidth + kGridWidth / 2);
  return 2;
}

lua::NResultsOr LuaMazeGeneration::FromWorldPos(lua_State* L) {
  double x = 0.0;
  double y = 0.0;
  if (lua_gettop(L) != 3 || !lua::Read(L, 2, &x) || !lua::Read(L, 3, &y)) {
    return "[fromWorldPos] - Must provide x, y";
  }

  // y = (text_maze_.Area().size.height - row) * kGridWidth + kGridWidth/2
  //  ==>  row = text_maze_.Area().size.height - (y - kGridWidth/2) / kGridWidth
  //           = text_maze_.Area().size.height - y/kGridWidth - 1/2
  //           = text_maze_.Area().size.height - floor(y/kGridWidth)
  lua::Push(L, text_maze_.Area().size.height -
                   static_cast<int>(std::floor(y / kGridWidth)));

  // x = (col - 1) * kGridWidth + kGridWidth/2
  //  ==>  col = (x - kGridWidth/2) / kGridWidth + 1
  //           = x / kGridWidth - 1/2 + 1
  //           = x / kGridWidth + 1/2
  //           = floor(x / kGridWidth + 1)
  lua::Push(L, static_cast<int>(std::floor(x / kGridWidth + 1.0)));
  return 2;
}

lua::NResultsOr LuaMazeGeneration::CountCharacters(
    lua_State* L, maze_generation::TextMaze::Layer layer) {
  std::string characters;
  if (!lua::Read(L, -1, &characters)) {
    return "Must supply a <string> as first argument.";
  }

  std::size_t sum = 0;
  const auto& text = text_maze_.Text(layer);
  for (const char ent : characters) {
    sum += std::count(text.begin(), text.end(), ent);
  }
  lua::Push(L, sum);
  return 1;
}

lua::NResultsOr LuaMazeGeneration::CountEntities(lua_State* L) {
  return CountCharacters(L, maze_generation::TextMaze::kEntityLayer);
}

lua::NResultsOr LuaMazeGeneration::CountVariations(lua_State* L) {
  return CountCharacters(L, maze_generation::TextMaze::kVariationsLayer);
}

lua::NResultsOr LuaMazeGeneration::VisitFill(lua_State* L) {
  lua::TableRef table;
  if (lua_gettop(L) < 2 || !lua::Read(L, 2, &table)) {
    return "[visitFill] - must supply table";
  }
  std::array<int, 2> cell;
  if (!table.LookUp("cell", &cell)) {
    return "[visitFill] - must supply 'cell' with an array of two integers.";
  }
  std::vector<char> wall_chars = {'*'};
  std::string wall_string;
  if (table.Contains("wall")) {
    if (!table.LookUp("wall", &wall_string)) {
      return "[visitFill] - must supply 'wall' with a string of wall "
             "characters.";
    }
    wall_chars.assign(wall_string.begin(), wall_string.end());
  }

  if (!table.Contains("func")) {
    return "[visitFill] - must supply 'func' which is called with each row, "
           "and distance from 'cell'";
  }

  maze_generation::FloodFill fill(text_maze_,
                                  maze_generation::TextMaze::kEntityLayer,
                                  {cell[0] - 1, cell[1] - 1}, wall_chars);

  lua::NResultsOr first_error(0);
  fill.Visit([&table, L, &first_error](int row, int col, int distance) {
    if (first_error.ok()) {
      table.LookUpToStack("func");
      lua::Push(L, row + 1);
      lua::Push(L, col + 1);
      lua::Push(L, distance);
      first_error = lua::Call(L, 3);
      lua_pop(L, first_error.n_results());
    }
  });
  return first_error;
}

lua::NResultsOr LuaMazeGeneration::VisitRandomPath(lua_State* L) {
  lua::TableRef table;
  if (lua_gettop(L) < 2 || !lua::Read(L, 2, &table)) {
    return "[visitRandomPath] - must supply table";
  }
  std::mt19937_64 seeded_rng;
  std::mt19937_64* prng =
      GetRandomNumberGenerator(&table, &seeded_rng, mixer_seq_);
  if (prng == nullptr) {
    return "[visitRandomPath] - must supply 'random' with random number "
           "generator. ('seed' is deprecated.)";
  }
  std::array<int, 2> from;
  if (!table.LookUp("from", &from)) {
    return "[visitRandomPath] - must supply 'from' with an array of two "
           "integers.";
  }
  std::array<int, 2> to;
  if (!table.LookUp("to", &to)) {
    return "[visitRandomPath] - must supply 'to' with an array of two "
           "integers.";
  }
  std::vector<char> wall_chars = {'*'};
  if (table.Contains("wall")) {
    std::string wall_string;
    if (!table.LookUp("wall", &wall_string)) {
      return "[visitRandomPath] - must supply 'wall' with a string of wall "
             "characters.";
    }
    wall_chars.assign(wall_string.begin(), wall_string.end());
  }
  if (!table.Contains("func")) {
    return "[visitRandomPath] - must supply callback 'func' with a string of "
           "wall characters.";
  }

  std::vector<maze_generation::Pos> path = maze_generation::FindRandomPath(
      {from[0] - 1, from[1] - 1}, {to[0] - 1, to[1] - 1}, wall_chars,
      &text_maze_, prng);
  for (const auto& pos : path) {
    table.LookUpToStack("func");
    lua::Push(L, pos.row + 1);
    lua::Push(L, pos.col + 1);
    lua::NResultsOr error = lua::Call(L, 2);
    lua_pop(L, error.n_results());
    if (!error.ok()) return error;
  }
  lua::Push(L, !path.empty());
  return 1;
}

// Registers classes metatable with Lua.
// [0, 0, -]
void LuaMazeGeneration::Register(lua_State* L) {
  Class::Reg regs[] = {
      {"entityLayer", Class::Member<&LuaMazeGeneration::EntityLayer>},
      {"variationsLayer", Class::Member<&LuaMazeGeneration::VariationsLayer>},
      {"getEntityCell", Class::Member<&LuaMazeGeneration::GetEntityCell>},
      {"setEntityCell", Class::Member<&LuaMazeGeneration::SetEntityCell>},
      {"fillEntityRect", Class::Member<&LuaMazeGeneration::FillEntityRect>},
      {"getVariationsCell",
       Class::Member<&LuaMazeGeneration::GetVariationsCell>},
      {"setVariationsCell",
       Class::Member<&LuaMazeGeneration::SetVariationsCell>},
      {"countEntities", Class::Member<&LuaMazeGeneration::CountEntities>},
      {"countVariations", Class::Member<&LuaMazeGeneration::CountVariations>},
      {"rotate", Class::Member<&LuaMazeGeneration::Rotate>},
      {"paste", Class::Member<&LuaMazeGeneration::Paste>},
      {"findRooms", Class::Member<&LuaMazeGeneration::FindRooms>},
      {"size", Class::Member<&LuaMazeGeneration::Size>},
      {"toWorldPos", Class::Member<&LuaMazeGeneration::ToWorldPos>},
      {"fromWorldPos", Class::Member<&LuaMazeGeneration::FromWorldPos>},
      {"visitFill", Class::Member<&LuaMazeGeneration::VisitFill>},
      {"visitRandomPath", Class::Member<&LuaMazeGeneration::VisitRandomPath>},
  };
  Class::Register(L, regs);
  LuaRoom::Register(L);
}

}  // namespace lab
}  // namespace deepmind
