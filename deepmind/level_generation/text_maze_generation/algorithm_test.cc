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

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "deepmind/level_generation/text_maze_generation/text_maze.h"

namespace deepmind {
namespace lab {
namespace maze_generation {
namespace {

using ::testing::AnyOf;
using ::testing::Eq;

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

TEST(AlgorithmTest, MakeLargeOneRoom) {
  std::mt19937_64 prbg(10);
  SeparateRectangleParams params{};
  params.min_size = Size{9, 13};
  params.max_size = Size{9, 13};
  params.retry_count = 1;
  params.max_rects = 1;
  params.density = 1.0;
  auto rects =
      MakeSeparateRectangles(Rectangle{{0, 0}, {11, 15}}, params, &prbg);
  ASSERT_EQ(rects.size(), 1);
  const auto& rect = rects.front();
  EXPECT_EQ(rect.pos.col, 1);
  EXPECT_EQ(rect.pos.row, 1);
  EXPECT_EQ(rect.size.height, 9);
  EXPECT_EQ(rect.size.width, 13);
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

// The utility of the following tests:
// - FillSpaceWithMaze
// - RandomConnectRegions
// - RemoveAllHorseshoeBends
// - AddNEntitiesToEachRoom
// is somewhat limited: Their outputs depend on the specific implementation of
// various random algorithms, and thus they are really only reliable on a fixed
// platform. When moving platforms, you will most likely need to generate new
// expected outputs.
TEST(AlgorithmTest, FillSpaceWithMaze) {
  std::mt19937_64 prbg(0);
  TextMaze maze({11, 11});
  FillSpaceWithMaze(1, 0, &maze, &prbg);
  constexpr char kMaze0[] =
      "***********\n"
      "* *       *\n"
      "* *** *** *\n"
      "*   * * * *\n"
      "*** * * * *\n"
      "* *   *   *\n"
      "* ***** ***\n"
      "*     * * *\n"
      "* * *** * *\n"
      "* *       *\n"
      "***********\n";
  constexpr char kMaze1[] =
      "***********\n"
      "* *   *   *\n"
      "* * * * * *\n"
      "*   * * * *\n"
      "***** * * *\n"
      "*     * * *\n"
      "* ******* *\n"
      "* *     * *\n"
      "* * *** * *\n"
      "*   *     *\n"
      "***********\n";
  EXPECT_THAT(maze.Text(TextMaze::kEntityLayer), AnyOf(Eq(kMaze0), Eq(kMaze1)));
}

TEST(AlgorithmTest, RandomConnectRegions) {
  std::mt19937_64 prbg(0);
  TextMaze maze =
      FromCharGrid(CharGrid("***********\n"
                            "*     *****\n"
                            "*     *****\n"
                            "*     *   *\n"
                            "*******   *\n"
                            "*   ***   *\n"
                            "*   *******\n"
                            "*   *     *\n"
                            "*   *     *\n"
                            "*   *     *\n"
                            "***********\n"));
  auto rooms = FindRooms(maze, {'*'});
  EXPECT_EQ(4, rooms.size());
  for (unsigned int i = 0; i < rooms.size(); ++i) {
    const auto& room = rooms[i];
    for (const auto& cell : room) {
      maze.SetCellId(cell, i + 1);
    }
  }
  RandomConnectRegions('#', 0.5, &maze, &prbg);
  constexpr char kMaze0[] =
      "***********\n"
      "*     *****\n"
      "*     *****\n"
      "*     #   *\n"
      "*#*#***   *\n"
      "*   ***   *\n"
      "*   ***#***\n"
      "*   *     *\n"
      "*   *     *\n"
      "*   #     *\n"
      "***********\n";
  constexpr char kMaze1[] =
      "***********\n"
      "*     *****\n"
      "*     *****\n"
      "*     #   *\n"
      "*#*#***   *\n"
      "*   ***   *\n"
      "*   *****#*\n"
      "*   *     *\n"
      "*   *     *\n"
      "*   #     *\n"
      "***********\n";

  EXPECT_THAT(maze.Text(TextMaze::kEntityLayer), AnyOf(Eq(kMaze0), Eq(kMaze1)));
}

TEST(AlgorithmTest, RemoveAllHorseshoeBends) {
  std::mt19937_64 prbg(0);
  TextMaze maze =
      FromCharGrid(CharGrid("***********\n"
                            "*       * *\n"
                            "*     * * *\n"
                            "*     *   *\n"
                            "* ******* *\n"
                            "*   *     *\n"
                            "* * ***** *\n"
                            "* *       *\n"
                            "* *****   *\n"
                            "*     *   *\n"
                            "***********\n"));
  auto rooms = FindRooms(maze, {'*'});
  for (unsigned int i = 0; i < rooms.size(); ++i) {
    const auto& room = rooms[i];
    for (const auto& cell : room) {
      maze.SetCellId(cell, i + 1);
    }
  }
  RemoveDeadEnds(' ', '*', {}, &maze);
  RemoveAllHorseshoeBends('*', {}, &maze);
  EXPECT_EQ(
      "***********\n"
      "*       ***\n"
      "*     * ***\n"
      "*     *   *\n"
      "* ******* *\n"
      "*   ***** *\n"
      "*** ***** *\n"
      "***       *\n"
      "*******   *\n"
      "*******   *\n"
      "***********\n",
      maze.Text(TextMaze::kEntityLayer));
}

TEST(AlgorithmTest, RemoveAllHorseshoeBendsDoors) {
  std::mt19937_64 prbg(0);
  TextMaze maze =
      FromCharGrid(CharGrid("**********\n"
                            "*****    *\n"
                            "*****    *\n"
                            "*   #    *\n"
                            "* ******#*\n"
                            "*        *\n"
                            "**********\n"));
  auto rooms = FindRooms(maze, {'*'});
  ASSERT_EQ(1, rooms.size());
  ASSERT_EQ(12, rooms.front().size());
  for (unsigned int i = 0; i < rooms.size(); ++i) {
    const auto& room = rooms[i];
    for (const auto& cell : room) {
      maze.SetCellId(cell, i + 1);
    }
  }
  RemoveDeadEnds(' ', '*', {}, &maze);
  RemoveAllHorseshoeBends('*', {}, &maze);

  EXPECT_EQ(
      "**********\n"
      "*****    *\n"
      "*****    *\n"
      "*** #    *\n"
      "*** ****#*\n"
      "***      *\n"
      "**********\n",
      maze.Text(TextMaze::kEntityLayer));
}

TEST(AlgorithmTest, AddNEntitiesToEachRoom) {
  std::mt19937_64 prbg(0);
  TextMaze maze =
      FromCharGrid(CharGrid("***********\n"
                            "*       * *\n"
                            "*     * * *\n"
                            "*     *   *\n"
                            "* ******* *\n"
                            "*   *     *\n"
                            "* * ***** *\n"
                            "* *       *\n"
                            "* *****   *\n"
                            "*     *   *\n"
                            "***********\n"));
  std::vector<Rectangle> rooms = {{{1, 1}, {3, 5}}, {{7, 7}, {3, 3}}};
  AddNEntitiesToEachRoom(rooms, 3, 'A', ' ', &maze, &prbg);

  constexpr char kMaze0[] =
      "***********\n"
      "*       * *\n"
      "*     * * *\n"
      "* AA A*   *\n"
      "* ******* *\n"
      "*   *     *\n"
      "* * ***** *\n"
      "* *     A *\n"
      "* *****AA *\n"
      "*     *   *\n"
      "***********\n";

  constexpr char kMaze1[] =
      "***********\n"
      "*A   A  * *\n"
      "* A   * * *\n"
      "*     *   *\n"
      "* ******* *\n"
      "*   *     *\n"
      "* * ***** *\n"
      "* *     A *\n"
      "* *****   *\n"
      "*     *AA *\n"
      "***********\n";
  EXPECT_THAT(maze.Text(TextMaze::kEntityLayer), AnyOf(Eq(kMaze0), Eq(kMaze1)));
}

}  // namespace
}  // namespace maze_generation
}  // namespace lab
}  // namespace deepmind
