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

#include "deepmind/level_generation/text_maze_generation/algorithm.h"

#include <random>

#include "gtest/gtest.h"
#include "deepmind/level_generation/text_maze_generation/text_maze.h"

namespace deepmind {
namespace lab {
namespace maze_generation {
namespace {

TEST(AlgorithmTest, FromCharGrid) {
  TextMaze maze =
      FromCharGrid(CharGrid("*****\n"
                            "*   *   *\n"
                            "*   *   *\n"
                            "*   *   *\n"
                            "*****\n"));

  maze.VisitIntersection(TextMaze::kEntityLayer, {{1, 1}, {3, 3}},
                         [](int, int, char c) { EXPECT_EQ(' ', c); });
  maze.VisitIntersection(TextMaze::kEntityLayer, {{1, 5}, {3, 3}},
                         [](int, int, char c) { EXPECT_EQ(' ', c); });
  maze.VisitIntersection(TextMaze::kEntityLayer, {{1, 4}, {3, 1}},
                         [](int, int, char c) { EXPECT_EQ('*', c); });
}

TEST(AlgorithmTest, FromCharGrids) {
  TextMaze maze = FromCharGrid(CharGrid("*********\n"
                                        "*   *   *\n"
                                        "*   *   *\n"
                                        "*   *   *\n"
                                        "*********\n"),
                               CharGrid(".........\n"
                                        ".AAA.BBB.\n"
                                        ".AAA.BBB.\n"
                                        ".AAA.BBB.\n"));

  maze.VisitIntersection(TextMaze::kEntityLayer, {{1, 1}, {3, 3}},
                         [](int, int, char c) { EXPECT_EQ(' ', c); });
  maze.VisitIntersection(TextMaze::kEntityLayer, {{1, 5}, {3, 3}},
                         [](int, int, char c) { EXPECT_EQ(' ', c); });
  maze.VisitIntersection(TextMaze::kEntityLayer, {{1, 4}, {3, 1}},
                         [](int, int, char c) { EXPECT_EQ('*', c); });

  maze.VisitIntersection(TextMaze::kVariationsLayer, {{1, 1}, {3, 3}},
                         [](int, int, char c) { EXPECT_EQ('A', c); });
  maze.VisitIntersection(TextMaze::kVariationsLayer, {{1, 5}, {3, 3}},
                         [](int, int, char c) { EXPECT_EQ('B', c); });
  maze.VisitIntersection(TextMaze::kVariationsLayer, {{1, 4}, {3, 1}},
                         [](int, int, char c) { EXPECT_EQ('.', c); });
}

TEST(AlgorithmTest, FindRoomTJunction) {
  TextMaze maze =
      FromCharGrid(CharGrid("*********\n"
                            "*   *   *\n"
                            "*   *   *\n"
                            "** *** **\n"
                            "**     **\n"
                            "  ** ****\n"
                            "  ** ****\n"
                            "* ** ****\n"
                            "*       *\n"
                            "*       *\n"
                            "*********\n"));
  auto rooms = FindRooms(maze, {'*'});
  EXPECT_EQ(4, rooms.size());
  char start = 'A';
  for (const auto& room : rooms) {
    for (const auto& cell : room) {
      maze.SetCell(TextMaze::kVariationsLayer, cell, start);
    }
    ++start;
  }
  EXPECT_EQ(
      ".........\n"
      ".AAA.BBB.\n"
      ".AAA.BBB.\n"
      ".........\n"
      ".........\n"
      "CC.......\n"
      "CC.......\n"
      ".........\n"
      ".DDDDDDD.\n"
      ".DDDDDDD.\n"
      ".........\n",
      maze.Text(TextMaze::kVariationsLayer));
}

TEST(AlgorithmTest, FindRoomsDeadEnds) {
  TextMaze maze =
      FromCharGrid(CharGrid("*********\n"
                            "*   *   *\n"
                            "*   *   *\n"
                            "*       *\n"
                            "*** *****\n"
                            "*      **\n"
                            "*** * ***\n"
                            "*   *   *\n"
                            "*   *   *\n"
                            "*       *\n"
                            "* ** ****\n"
                            "* * *   *\n"
                            "*   *   *\n"
                            "*   *   *\n"
                            "*       *\n"
                            "*********\n"));
  auto rooms = FindRooms(maze, {'*'});
  EXPECT_EQ(6, rooms.size());

  char start = 'A';
  for (const auto& room : rooms) {
    for (const auto& cell : room) {
      maze.SetCell(TextMaze::kVariationsLayer, cell, start);
    }
    ++start;
  }
  EXPECT_EQ(
      ".........\n"
      ".AAA.BBB.\n"
      ".AAA.BBB.\n"
      ".AAA.BBB.\n"
      ".........\n"
      ".........\n"
      ".........\n"
      ".CCC.DDD.\n"
      ".CCC.DDD.\n"
      ".CCC.DDD.\n"
      ".........\n"
      ".....EEE.\n"
      ".FFF.EEE.\n"
      ".FFF.EEE.\n"
      ".FFF.EEE.\n"
      ".........\n",
      maze.Text(TextMaze::kVariationsLayer));
}

TEST(AlgorithmTest, MakeSeparateRooms) {
  std::mt19937_64 prbg(10);

  for (int j = 0; j < 1; ++j) {
    TextMaze maze(Size{40, 40});

    SeparateRectangleParams params{};
    params.min_size = Size{3, 3};
    params.max_size = Size{9, 9};
    params.retry_count = 100;
    params.max_rects = 26;
    params.density = 0.5;

    auto rects = MakeSeparateRectangles(maze.Area(), params, &prbg);
    char variation = 'A';
    for (const auto& rect : rects) {
      EXPECT_EQ(rect.Area(), Overlap(maze.Area(), rect).Area())
          << "Rectangles must fit";
      EXPECT_LE(rect.Area(), 9 * 9) << "Must not generate max area rectangles";
      maze.VisitMutableIntersection(
          TextMaze::kVariationsLayer, rect,
          [variation](int i, int j, char* cell) { *cell = variation; });
      ++variation;
    }
    EXPECT_LE(rects.size(), 26);
  }
}

TEST(AlgorithmTest, RemoveDeadEnds) {
  TextMaze maze =
      FromCharGrid(CharGrid("**********\n"
                            "*  ***** *\n"
                            "*        *\n"
                            "* ** *****\n"
                            "**** *** *\n"
                            "**       *\n"
                            "*  * *** *\n"
                            "**********\n"));
  RemoveDeadEnds(' ', '*', {}, &maze);
  EXPECT_EQ(
      "**********\n"
      "*  *******\n"
      "*  *******\n"
      "**********\n"
      "**********\n"
      "**********\n"
      "**********\n"
      "**********\n",
      maze.Text(TextMaze::kEntityLayer));
}

TEST(AlgorithmTest, RemoveDeadEndsLoop) {
  TextMaze maze =
      FromCharGrid(CharGrid("+--------+\n"
                            "|  ***** |\n"
                            "|        |\n"
                            "| ** ***P|\n"
                            "|*** *** |\n"
                            "|*       |\n"
                            "|  * *** |\n"
                            "+--------+\n"));
  RemoveDeadEnds(' ', '*', {'+', '-', '|'}, &maze);
  EXPECT_EQ(
      "+--------+\n"
      "|  ******|\n"
      "|        |\n"
      "|*** ***P|\n"
      "|*** *** |\n"
      "|***     |\n"
      "|********|\n"
      "+--------+\n",
      maze.Text(TextMaze::kEntityLayer));
}

TEST(AlgorithmTest, RemoveDeadEndsNone) {
  TextMaze maze =
      FromCharGrid(CharGrid("******\n"
                            "*AAAA*\n"
                            "*B  B*\n"
                            "*CCCC*\n"
                            "******\n"));
  RemoveDeadEnds(' ', '*', {}, &maze);
  EXPECT_EQ(
      "******\n"
      "*AAAA*\n"
      "*B  B*\n"
      "*CCCC*\n"
      "******\n",
      maze.Text(TextMaze::kEntityLayer));
}

TEST(AlgorithmTest, RemoveDeadEndsSingle) {
  TextMaze maze =
      FromCharGrid(CharGrid("******\n"
                            "* BBB*\n"
                            "******\n"));
  RemoveDeadEnds(' ', '*', {}, &maze);
  EXPECT_EQ(
      "******\n"
      "**BBB*\n"
      "******\n",
      maze.Text(TextMaze::kEntityLayer));
}

TEST(AlgorithmTest, RemoveDeadEndsHole) {
  TextMaze maze =
      FromCharGrid(CharGrid("******\n"
                            "* P* *\n"
                            "******\n"));
  RemoveDeadEnds(' ', '*', {}, &maze);
  EXPECT_EQ(
      "******\n"
      "**P***\n"
      "******\n",
      maze.Text(TextMaze::kEntityLayer));
}

TEST(AlgorithmTest, RemoveDeadEndsEdge) {
  TextMaze maze =
      FromCharGrid(CharGrid("******\n"
                            "  P***\n"
                            "******\n"));
  RemoveDeadEnds(' ', '*', {}, &maze);
  EXPECT_EQ(
      "******\n"
      "**P***\n"
      "******\n",
      maze.Text(TextMaze::kEntityLayer));
}

}  // namespace
}  // namespace maze_generation
}  // namespace lab
}  // namespace deepmind
