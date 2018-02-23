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

#include "deepmind/lua/bind.h"

#include "gtest/gtest.h"
#include "deepmind/lua/push.h"
#include "deepmind/lua/read.h"
#include "deepmind/lua/vm_test_util.h"

namespace deepmind {
namespace lab {
namespace lua {
namespace {

constexpr char kNegativeError[] = "Value was negative!";
constexpr char kFailedToConvertError[] = "Value was not a number";

// Returns value passed in if >=0 or an error message.
NResultsOr NonNegativeOrError(lua_State* L) {
  double value;
  if (Read(L, 1, &value)) {
    if (value >= 0) {
      return 1;
    } else {
      return kNegativeError;
    }
  }
  return kFailedToConvertError;
}

using BindTest = testing::TestWithVm;

TEST_F(BindTest, Success) {
  Push(L, Bind<NonNegativeOrError>);
  Push(L, 10.0);
  ASSERT_EQ(0, lua_pcall(L, 1, 1, 0)) << ToString(L, -1);
  double value_out;
  ASSERT_TRUE(IsFound(Read(L, -1, &value_out))) << "Result was not a double.";
  EXPECT_EQ(10.0, value_out);
}

TEST_F(BindTest, FailLessThanZero) {
  Push(L, Bind<NonNegativeOrError>);
  Push(L, -10.0);
  ASSERT_NE(0, lua_pcall(L, 1, 1, 0)) << "No error message!";
  std::string error;
  ASSERT_TRUE(IsFound(Read(L, -1, &error))) << "Error was not available.";
  EXPECT_EQ(kNegativeError, error);
}

TEST_F(BindTest, FailNotADouble) {
  Push(L, Bind<NonNegativeOrError>);
  Push(L, "-10.0");
  ASSERT_NE(0, lua_pcall(L, 1, 1, 0)) << "No error message!";
  std::string error;
  ASSERT_TRUE(IsFound(Read(L, -1, &error))) << "Error was not available.";
  EXPECT_EQ(kFailedToConvertError, error);
}

}  // namespace
}  // namespace lua
}  // namespace lab
}  // namespace deepmind
