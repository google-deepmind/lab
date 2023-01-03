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

#include "deepmind/level_generation/text_level/parse_text_level.h"

#include "gtest/gtest.h"

namespace deepmind {
namespace lab {
namespace {

constexpr char kEntities[] =
    "   ******\n"
    " ***    ****\n"
    " *    x    *\n"
    " **   ******\n"
    "  *****\n";

constexpr char kVariations[] =
    ".........\n"
    "....AAAAAAAAAAAAAA\n"
    ".\n"
    ".CCCCCCCCCCC\n"
    "$%^&*()\n"
    ".............\n";

TEST(ParseTextLevel, WallsAndVariations) {
  GridMaze maze = ParseTextLevel(kEntities, kVariations);

  EXPECT_EQ(5U, maze.height());
  EXPECT_EQ(12U, maze.width());

  std::string s;
  for (std::size_t i = 0; i != maze.height(); ++i) {
    s += std::string(maze.width() + 1, '\n');
  }

  std::string expected =
      "   #########\n"
      " ###AAAA####\n"
      " #         #\n"
      " ##CCC######\n"
      "  ##########\n";

  maze.Visit([&](std::size_t i, std::size_t j, const GridMaze::Cell& cell) {
      char & c = s[i * (maze.width() + 1) + j];

      if (cell.value == '*') {
        c = '#';
      } else if (cell.variation != '\0') {
        c = cell.variation;
      } else {
        c = ' ';
      }
    });

  EXPECT_EQ(s, expected);
}

TEST(ParseTextLevel, WallsOnly) {
  GridMaze maze = ParseTextLevel(kEntities, std::string());

  EXPECT_EQ(5U, maze.height());
  EXPECT_EQ(12U, maze.width());

  std::string s;
  for (std::size_t i = 0; i != maze.height(); ++i) {
    s += std::string(maze.width() + 1, '\n');
  }

  std::string expected =
      "   #########\n"
      " ###    ####\n"
      " #         #\n"
      " ##   ######\n"
      "  ##########\n";

  maze.Visit([&](std::size_t i, std::size_t j, const GridMaze::Cell& cell) {
      s[i * (maze.width() + 1) + j] = cell.value == '*' ? '#' : ' ';
    });

  EXPECT_EQ(s, expected);
}
TEST(ParseTextLevel, FindX) {
  GridMaze maze = ParseTextLevel(kEntities, kVariations);

  bool found_x = false;

  maze.Visit([&](std::size_t i, std::size_t j, const GridMaze::Cell& cell) {
      if (i == 2 && j == 6 && cell.value == 'x')
        found_x = true;
    });

  EXPECT_TRUE(found_x);
}

TEST(ParseTextLevel, Openings) {
  GridMaze maze = ParseTextLevel(kEntities, kVariations);

  std::vector<int> rooms(maze.height() * maze.width());

  // To test directions, we plot the dual graph (faces => vertices,
  // openings => edges).

  const char* kRoomVal[] = {
      "\u25A0",  // no opening
      "\u2575",  //  1 = North
      "\u2577",  //  2 = South
      "\u2502",  //  3 = North, South
      "\u2576",  //  4 = East
      "\u2514",  //  5 = North, East
      "\u250C",  //  6 = South, East
      "\u251C",  //  7 = North, South, East
      "\u2574",  //  8 = West
      "\u2518",  //  9 = North, West
      "\u2510",  // 10 = South, West
      "\u2524",  // 11 = North, South, West
      "\u2500",  // 12 = East, West
      "\u2534",  // 13 = North, East, West
      "\u252C",  // 14 = South, East, West
      "\u253C",  // 15 = North, South, East, West
  };

  maze.Visit([&](std::size_t i, std::size_t j, const GridMaze::Cell& cell) {
    rooms[i * maze.width() + j] =
        cell.value == '*' ? -1 : static_cast<unsigned char>(cell.opening);
  });

  std::string room_str;

  for (std::size_t k = 0; k != rooms.size(); ++k) {
    if (k != 0 && k % maze.width() == 0) room_str.push_back('\n');
    room_str.append(rooms[k] == -1 ? " " : kRoomVal[rooms[k]]);
  }

  std::string expected =
      "┌─╴         \n"
      "│   ┌┬┬┐    \n"
      "│ ╶┬┼┼┴┴──╴ \n"
      "│  └┴┘      \n"
      "└╴          ";

  EXPECT_EQ(expected, room_str);
}

}  // namespace
}  // namespace lab
}  // namespace deepmind
