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

#include "deepmind/level_generation/text_maze_generation/flood_fill.h"

#include <tuple>

#include "gtest/gtest.h"
#include "deepmind/level_generation/text_maze_generation/algorithm.h"
#include "deepmind/level_generation/text_maze_generation/text_maze.h"

namespace deepmind {
namespace lab {
namespace maze_generation {
namespace {

TEST(FloodFillTest, DistanceLinear) {
  TextMaze maze = FromCharGrid(CharGrid("    "));
  FloodFill fill_info(maze, TextMaze::kEntityLayer, {0, 0}, {'*'});
  maze.Area().Visit([&fill_info](int i, int j) {
    const int distances[1][4] = {{0, 1, 2, 3}};
    EXPECT_EQ(distances[i][j], fill_info.DistanceFrom({i, j}));
  });

  fill_info = FloodFill(maze, TextMaze::kEntityLayer, {0, 1}, {'*'});
  maze.Area().Visit([&fill_info](int i, int j) {
    const int distances[1][4] = {{1, 0, 1, 2}};
    EXPECT_EQ(distances[i][j], fill_info.DistanceFrom({i, j}));
  });

  fill_info = FloodFill(maze, TextMaze::kEntityLayer, {0, 3}, {'*'});
  maze.Area().Visit([&fill_info](int i, int j) {
    const int distances[1][4] = {{3, 2, 1, 0}};
    EXPECT_EQ(distances[i][j], fill_info.DistanceFrom({i, j}));
  });

  fill_info = FloodFill(maze, TextMaze::kEntityLayer, {-1, 0}, {'*'});
  maze.Area().Visit([&fill_info](int i, int j) {
    EXPECT_EQ(-1, fill_info.DistanceFrom({i, j}));
  });
}

TEST(FloodFillTest, DistanceManhatten) {
  TextMaze maze =
      FromCharGrid(CharGrid("   \n"
                            "   \n"
                            "   \n"));

  FloodFill fill_info(maze, TextMaze::kEntityLayer, {0, 0}, {'*'});

  maze.Area().Visit([&fill_info](int i, int j) {
    const int distances[3][3] = {
        {0, 1, 2},  //
        {1, 2, 3},  //
        {2, 3, 4},  //
    };
    EXPECT_EQ(distances[i][j], fill_info.DistanceFrom({i, j}));
  });

  fill_info = FloodFill(maze, TextMaze::kEntityLayer, {1, 1}, {'*'});
  maze.Area().Visit([&fill_info](int i, int j) {
    const int distances[3][3] = {
        {2, 1, 2},  //
        {1, 0, 1},  //
        {2, 1, 2},
    };
    EXPECT_EQ(distances[i][j], fill_info.DistanceFrom({i, j}));
  });
}

TEST(FloodFillTest, DistanceWallsConnected) {
  TextMaze maze =
      FromCharGrid(CharGrid(" * \n"
                            "   \n"
                            "   \n"));

  FloodFill fill_info(maze, TextMaze::kEntityLayer, {0, 0}, {'*'});
  maze.Area().Visit([&fill_info](int i, int j) {
    const int distances[3][3] = {
        {0, -1, 4},  //
        {1, 2, 3},   //
        {2, 3, 4},
    };
    EXPECT_EQ(distances[i][j], fill_info.DistanceFrom({i, j}));
  });
}

TEST(FloodFillTest, DistanceWallsSeparated) {
  TextMaze maze =
      FromCharGrid(CharGrid(" * \n"
                            " * \n"
                            " * \n"));

  FloodFill fill_info(maze, TextMaze::kEntityLayer, {0, 0}, {'*'});
  maze.Area().Visit([&fill_info](int i, int j) {
    const int distances[3][3] = {
        {0, -1, -1},  //
        {1, -1, -1},  //
        {2, -1, -1},
    };
    EXPECT_EQ(distances[i][j], fill_info.DistanceFrom({i, j}));
  });
}

TEST(FloodFillTest, ShortestPathFrom) {
  TextMaze maze =
      FromCharGrid(CharGrid(" * \n"
                            " * \n"
                            "   \n"));
  FloodFill fill_info(maze, TextMaze::kEntityLayer, {0, 0}, {'*'});
  maze.Area().Visit([&fill_info](int i, int j) {
    const int distances[3][3] = {
        {0, -1, 6},  //
        {1, -1, 5},  //
        {2, 3, 4},
    };
    EXPECT_EQ(distances[i][j], fill_info.DistanceFrom({i, j}));
  });
  std::mt19937_64 gen(10);
  auto path = fill_info.ShortestPathFrom({1, 2}, &gen);
  Pos pos[] = {{1, 2}, {2, 2}, {2, 1}, {2, 0}, {1, 0}, {0, 0}};
  ASSERT_EQ(std::distance(std::begin(pos), std::end(pos)), path.size());
  EXPECT_TRUE(std::equal(
      path.begin(), path.end(), std::begin(pos), [](Pos lhs, Pos rhs) {
        return std::tie(lhs.row, lhs.col) == std::tie(rhs.row, rhs.col);
      }));

  Pos fill[] = {{0, 0}, {1, 0}, {2, 0}, {2, 1}, {2, 2}, {1, 2}, {0, 2}};
  fill_info.Visit([&fill](int i, int j, int distance) {
    EXPECT_EQ(i, fill[distance].row);
    EXPECT_EQ(j, fill[distance].col);
  });
}

TEST(FloodFillTest, ShortestPathFromRandom) {
  TextMaze maze({20, 20});
  FloodFill fill_info(maze, TextMaze::kEntityLayer, {19, 19}, {});
  std::mt19937_64 gen(10);
  auto path = fill_info.ShortestPathFrom({0, 0}, &gen);
  Pos prev = {-1, 0};
  for (auto p : path) {
    EXPECT_TRUE((p.row == prev.row + 1 && p.col == prev.col) ||
                (p.row == prev.row && p.col == prev.col + 1));
    prev = p;
  }
}

}  // namespace
}  // namespace maze_generation
}  // namespace lab
}  // namespace deepmind
