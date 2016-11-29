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

#include "deepmind/level_generation/text_level/char_grid.h"

#include "gtest/gtest.h"

namespace deepmind {
namespace lab {
namespace {

TEST(CharGridTest, Simple) {
  CharGrid grid("abc\n12345\n");
  EXPECT_EQ(5U, grid.width());
  EXPECT_EQ(2U, grid.height());

  EXPECT_EQ('c', grid.CellAt(0, 2));
  EXPECT_EQ('3', grid.CellAt(1, 2));

  EXPECT_EQ('\0', grid.CellAt(2, 0));
  EXPECT_EQ('\0', grid.CellAt(0, 4));
}

}  // namespace
}  // namespace lab
}  // namespace deepmind
