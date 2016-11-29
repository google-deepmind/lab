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

#include "deepmind/level_generation/text_maze_generation/text_maze.h"

#include "gtest/gtest.h"

namespace deepmind {
namespace lab {
namespace maze_generation {
namespace {

TEST(TextMazeTest, RectangleOverlap) {
  Rectangle rect{{0, 0}, {10, 10}};
  EXPECT_EQ(100, rect.Area());
  Rectangle rect_intersect{{6, 5}, {10, 10}};
  Rectangle overlap = Overlap(rect, rect_intersect);
  EXPECT_EQ(20, overlap.Area());
  EXPECT_EQ(6, overlap.pos.row);
  EXPECT_EQ(5, overlap.pos.col);
  EXPECT_EQ(4, overlap.size.height);
  EXPECT_EQ(5, overlap.size.width);
  Rectangle rect_no_intersect{{-20, 20}, {10, 10}};
  Rectangle no_overlap = Overlap(rect, rect_no_intersect);
  EXPECT_EQ(0, no_overlap.Area());
}

TEST(TextMazeTest, RectangleIsSeparate) {
  Rectangle rect{{0, 0}, {10, 10}};
  EXPECT_EQ(100, rect.Area());
  Rectangle rect_intersect_1{{6, 5}, {10, 10}};
  EXPECT_FALSE(IsSeparate(rect, rect_intersect_1));
  Rectangle rect_intersect_2{{10, 0}, {10, 10}};
  EXPECT_TRUE(IsSeparate(rect, rect_intersect_2));
  Rectangle rect_intersect_3{{0, 10}, {10, 10}};
  EXPECT_TRUE(IsSeparate(rect, rect_intersect_3));
  Rectangle rect_intersect_4{{-10, 0}, {10, 10}};
  EXPECT_TRUE(IsSeparate(rect, rect_intersect_4));
  Rectangle rect_intersect_5{{0, -10}, {10, 10}};
  EXPECT_TRUE(IsSeparate(rect, rect_intersect_5));
}

TEST(TextMazeTest, RectangleVisit) {
  Rectangle rect{{0, 0}, {10, 10}};
  int counter = 0;
  auto count = [&counter](int i, int j) { ++counter; };
  rect.Visit(count);
  EXPECT_EQ(100, counter);
  counter = 0;
  rect.VisitNeighbours({0, 0}, count);
  EXPECT_EQ(2, counter);
  counter = 0;
  rect.VisitNeighbours({1, 0}, count);
  EXPECT_EQ(3, counter);
  counter = 0;
  rect.VisitNeighbours({0, 1}, count);
  EXPECT_EQ(3, counter);
  counter = 0;
  rect.VisitNeighbours({1, 1}, count);
  EXPECT_EQ(4, counter);
  counter = 0;
  rect.VisitNeighbours({9, 9}, count);
  EXPECT_EQ(2, counter);
  counter = 0;
  rect.VisitNeighbours({9, 8}, count);
  EXPECT_EQ(3, counter);
  counter = 0;
  rect.VisitNeighbours({8, 9}, count);
  EXPECT_EQ(3, counter);
  counter = 0;
  rect.VisitNeighbours({8, 8}, count);
  EXPECT_EQ(4, counter);
}

constexpr char kStar4x3[] =
    "***\n"
    "***\n"
    "***\n"
    "***\n";

constexpr char kDot4x3[] =
    "...\n"
    "...\n"
    "...\n"
    "...\n";

TEST(TextMazeTest, CreateLayers) {
  TextMaze maze({4, 3});
  EXPECT_EQ(kStar4x3, maze.Text(TextMaze::kEntityLayer));
  EXPECT_EQ(kDot4x3, maze.Text(TextMaze::kVariationsLayer));
  EXPECT_EQ(4, maze.Area().size.height);
  EXPECT_EQ(3, maze.Area().size.width);
}

constexpr char kCorners4x3[] =
    "X*X\n"
    "***\n"
    "***\n"
    "X*X\n";

TEST(TextMazeTest, CheckCorners) {
  TextMaze maze({4, 3});
  maze.SetCell(TextMaze::kEntityLayer, {0, 0}, 'X');
  maze.SetCell(TextMaze::kEntityLayer, {3, 0}, 'X');
  maze.SetCell(TextMaze::kEntityLayer, {0, 2}, 'X');
  maze.SetCell(TextMaze::kEntityLayer, {3, 2}, 'X');
  EXPECT_EQ(kCorners4x3, maze.Text(TextMaze::kEntityLayer));
}

TEST(TextMazeTest, GetCell) {
  TextMaze maze({4, 3});
  EXPECT_EQ('*', maze.GetCell(TextMaze::kEntityLayer, {0, 0}));
  EXPECT_EQ('*', maze.GetCell(TextMaze::kEntityLayer, {0, 2}));
  EXPECT_EQ('*', maze.GetCell(TextMaze::kEntityLayer, {3, 0}));
  EXPECT_EQ('*', maze.GetCell(TextMaze::kEntityLayer, {3, 2}));
  EXPECT_EQ('\0', maze.GetCell(TextMaze::kEntityLayer, {-1, 1}));
  EXPECT_EQ('\0', maze.GetCell(TextMaze::kEntityLayer, {1, -1}));
}

TEST(TextMazeTest, SetCell) {
  TextMaze maze({4, 3});
  maze.SetCell(TextMaze::kEntityLayer, {0, 0}, '1');
  EXPECT_EQ('1', maze.GetCell(TextMaze::kEntityLayer, {0, 0}));
  EXPECT_EQ('.', maze.GetCell(TextMaze::kVariationsLayer, {0, 0}));

  maze.SetCell(TextMaze::kEntityLayer, {0, 2}, '2');
  EXPECT_EQ('2', maze.GetCell(TextMaze::kEntityLayer, {0, 2}));

  maze.SetCell(TextMaze::kEntityLayer, {3, 0}, '3');
  EXPECT_EQ('3', maze.GetCell(TextMaze::kEntityLayer, {3, 0}));

  maze.SetCell(TextMaze::kEntityLayer, {3, 2}, '4');
  EXPECT_EQ('4', maze.GetCell(TextMaze::kEntityLayer, {3, 2}));

  maze.SetCell(TextMaze::kEntityLayer, {-1, 1}, '6');
  EXPECT_EQ('\0', maze.GetCell(TextMaze::kEntityLayer, {-1, 1}));

  maze.SetCell(TextMaze::kEntityLayer, {1, -1}, '7');
  EXPECT_EQ('\0', maze.GetCell(TextMaze::kEntityLayer, {1, -1}));

  maze.SetCell(TextMaze::kVariationsLayer, {4, 0}, '8');
  EXPECT_EQ('\0', maze.GetCell(TextMaze::kVariationsLayer, {4, 0}));
}

constexpr char kAlpha4x3[] =
    "ABC\n"
    "DEF\n"
    "GHI\n"
    "JKL\n";

TEST(TextMazeTest, Visit) {
  TextMaze maze({4, 3});
  int loc = 0;
  maze.VisitMutable(TextMaze::kEntityLayer,
                    [&loc](int, int, char* c) { *c = 'A' + loc++; });
  EXPECT_EQ(kAlpha4x3, maze.Text(TextMaze::kEntityLayer));
  int i_guess = 0;
  int j_guess = 0;
  loc = 0;
  maze.Visit(TextMaze::kEntityLayer,
             [&loc, &i_guess, &j_guess](int i, int j, char c) {
               EXPECT_EQ(i_guess, i);
               EXPECT_EQ(j_guess, j);
               EXPECT_EQ('A' + loc++, c);
               if (++j_guess == 3) {
                 j_guess = 0;
                 ++i_guess;
               }
             });

  EXPECT_EQ(4, i_guess);
  EXPECT_EQ(0, j_guess);
}

constexpr char kAlphaCorner4x3[] =
    "***\n"
    "***\n"
    "*AB\n"
    "*CD\n";

TEST(TextMazeTest, VisitIntersection) {
  TextMaze maze({4, 3});
  int loc = 0;
  maze.VisitMutableIntersection(
      TextMaze::kEntityLayer, {{2, 1}, {4, 2}},
      [&loc](int, int, char* c) { *c = 'A' + loc++; });
  EXPECT_EQ(kAlphaCorner4x3, maze.Text(TextMaze::kEntityLayer));

  loc = 0;
  int i_guess = 2;
  int j_guess = 1;
  maze.VisitIntersection(TextMaze::kEntityLayer, {{2, 1}, {4, 2}},
                         [&loc, &i_guess, &j_guess](int i, int j, char c) {
                           EXPECT_EQ(i_guess, i);
                           EXPECT_EQ(j_guess, j);
                           EXPECT_EQ('A' + loc++, c);
                           if (++j_guess == 3) {
                             j_guess = 1;
                             ++i_guess;
                           }
                         });

  EXPECT_EQ(4, i_guess);
  EXPECT_EQ(1, j_guess);
}

}  // namespace
}  // namespace maze_generation
}  // namespace lab
}  // namespace deepmind
