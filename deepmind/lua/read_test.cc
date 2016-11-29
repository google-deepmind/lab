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

#include "deepmind/lua/read.h"

#include <cstddef>
#include <cstring>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "deepmind/lua/push.h"
#include "deepmind/lua/vm_test_util.h"

namespace deepmind {
namespace lab {
namespace lua {
namespace {

constexpr char kTestString[] = "TestTest";
constexpr double kTestValue = 32.0;
constexpr double kTestRetValue = 32.0;

extern "C" {
static int TestFunction(lua_State* L) {
  Push(L, kTestRetValue);
  return 1;
}
}  // extern "C"

using ReadTest = testing::TestWithVm;

TEST_F(ReadTest, ReadFunction) {
  Push(L, TestFunction);
  lua_CFunction c_function;
  ASSERT_TRUE(Read(L, 1, &c_function));
  EXPECT_TRUE(TestFunction == c_function);
}

TEST_F(ReadTest, ReadString) {
  Push(L, kTestString);
  std::string result;
  ASSERT_TRUE(Read(L, 1, &result));
  EXPECT_EQ(kTestString, result);
}

TEST_F(ReadTest, ReadNumber) {
  Push(L, kTestValue);
  double result;
  ASSERT_TRUE(Read(L, 1, &result));
  EXPECT_EQ(kTestValue, result);
}

TEST_F(ReadTest, ReadUnsigned) {
  Push(L, 20);
  Push(L, -20);
  std::size_t result;
  ASSERT_TRUE(Read(L, 1, &result));
  EXPECT_EQ(20, result);
  // Should not read a negative value.
  ASSERT_FALSE(Read(L, 2, &result));
  EXPECT_EQ(20, result);
}

TEST_F(ReadTest, ReadBoolean) {
  Push(L, true);
  Push(L, false);
  Push(L, 1);
  bool result = false;
  ASSERT_TRUE(Read(L, 1, &result));
  EXPECT_EQ(true, result);
  ASSERT_TRUE(Read(L, 2, &result));
  EXPECT_EQ(false, result);
  ASSERT_FALSE(Read(L, 3, &result));
}

TEST_F(ReadTest, ReadLightUserData) {
  int test_value = 10;
  Push(L, &test_value);
  int* result;
  ASSERT_TRUE(Read(L, 1, &result));
  ASSERT_EQ(&test_value, result);
  EXPECT_EQ(test_value, *result);
}

TEST_F(ReadTest, ReadVector) {
  std::vector<double> test = {1, 2, 3, 4, 5};
  Push(L, "Junk");
  Push(L, test);
  Push(L, "Junk");
  std::vector<double> result;
  ASSERT_TRUE(Read(L, 2, &result));
  EXPECT_EQ(result, test);
  ASSERT_TRUE(Read(L, -2, &result));
  EXPECT_EQ(result, test);
}

TEST_F(ReadTest, ReadArray) {
  std::array<double, 5> test{{1, 2, 3, 4, 5}};
  Push(L, "Junk");
  Push(L, test);
  Push(L, "Junk");
  std::array<double, 5> result;
  ASSERT_TRUE(Read(L, 2, &result));
  EXPECT_EQ(result, test);
  ASSERT_TRUE(Read(L, -2, &result));
  EXPECT_EQ(result, test);
  EXPECT_EQ(3, lua_gettop(L));
}

TEST_F(ReadTest, ReadArrayFloat) {
  std::array<float, 5> test{{1, 2, 3, 4, 5}};
  Push(L, "Junk");
  Push(L, test);
  Push(L, "Junk");
  std::array<float, 5> result;
  ASSERT_TRUE(Read(L, 2, &result));
  EXPECT_EQ(result, test);
  ASSERT_TRUE(Read(L, -2, &result));
  EXPECT_EQ(result, test);
  EXPECT_EQ(3, lua_gettop(L));
}

TEST_F(ReadTest, ReadTable) {
  std::unordered_map<std::string, double> test = {
      {"one", 1},  //
      {"2", 2.0},  //
      {"3", 3.0},  //
      {"4", 4.0},  //
      {"5", 5.0},  //
  };

  Push(L, "Junk");
  Push(L, test);
  Push(L, "Junk");
  std::unordered_map<std::string, double> result;
  ASSERT_TRUE(Read(L, 2, &result));
  EXPECT_EQ(result, test);
  ASSERT_TRUE(Read(L, -2, &result));
  EXPECT_EQ(result, test);
  EXPECT_EQ(3, lua_gettop(L));
}

TEST_F(ReadTest, ToString) {
  Push(L, kTestString);
  EXPECT_EQ(kTestString, ToString(L, 1));
  lua_pop(L, 1);

  Push(L, 1);
  EXPECT_EQ("1", ToString(L, 1));
  lua_pop(L, 1);

  Push(L, TestFunction);
  EXPECT_THAT(ToString(L, 1), ::testing::HasSubstr("function"));
  lua_pop(L, 1);

  int i = 10;
  Push(L, &i);
  EXPECT_THAT(ToString(L, 1), ::testing::HasSubstr("pointer"));
  lua_pop(L, 1);

  Push(L, std::vector<int>());
  EXPECT_THAT(ToString(L, 1), ::testing::HasSubstr("table"));
  lua_pop(L, 1);

  Push(L, true);
  EXPECT_EQ("true", ToString(L, 1));
  lua_pop(L, 1);

  Push(L, false);
  EXPECT_EQ("false", ToString(L, 1));
  lua_pop(L, 1);

  lua_pushnil(L);
  EXPECT_THAT(ToString(L, 1), ::testing::HasSubstr("nil"));
  lua_pop(L, 1);

  EXPECT_THAT(ToString(L, 1), ::testing::HasSubstr("none"));
}

}  // namespace
}  // namespace lua
}  // namespace lab
}  // namespace deepmind
