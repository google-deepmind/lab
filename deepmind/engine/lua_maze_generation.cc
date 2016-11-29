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

const char* LuaMazeGeneration::ClassName() {
  return "deepmind.lab.LuaMazeGeneration";
}

int LuaMazeGeneration::Require(lua_State* L) {
  auto table = lua::TableRef::Create(L);
  table.Insert("MazeGeneration", &lua::Bind<LuaMazeGeneration::Create>);
  lua::Push(L, table);
  return 1;
}

lua::NResultsOr LuaMazeGeneration::Create(lua_State* L) {
  lua::TableRef table;
  lua::Read(L, -1, &table);
  if (table.Contains("entity")) {
    std::string entity_layer;
    if (!table.LookUp("entity", &entity_layer) || entity_layer.empty()) {
      return "[MazeGeneration] - Must construct with non empty entity_layer";
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
      return "[MazeGeneration] - Must construct with positive width and height";
    }
    if (!table.LookUp("width", &width) || width <= 0) {
      return "[MazeGeneration] - Must construct with positive width and height";
    }
    CreateObject(L, maze_generation::Size{height, width});
  }
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

// Registers classes metatable with Lua.
// [0 0 -]
void LuaMazeGeneration::Register(lua_State* L) {
  Class::Reg regs[] = {
      {"entityLayer", Class::Member<&LuaMazeGeneration::EntityLayer>},
      {"variationsLayer", Class::Member<&LuaMazeGeneration::VariationsLayer>},
      {"getEntityCell", Class::Member<&LuaMazeGeneration::GetEntityCell>},
      {"setEntityCell", Class::Member<&LuaMazeGeneration::SetEntityCell>},
      {"getVariationsCell",
       Class::Member<&LuaMazeGeneration::GetVariationsCell>},
      {"setVariationsCell",
       Class::Member<&LuaMazeGeneration::SetVariationsCell>},
      {"findRooms", Class::Member<&LuaMazeGeneration::FindRooms>},
      {"size", Class::Member<&LuaMazeGeneration::Size>},
      {"visitFill", Class::Member<&LuaMazeGeneration::VisitFill>},
  };
  Class::Register(L, regs);
  LuaRoom::Register(L);
}

}  // namespace lab
}  // namespace deepmind
