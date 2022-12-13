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

#include "deepmind/lua/push.h"

#include <set>
#include <string>
#include <utility>

#include "gtest/gtest.h"
#include "absl/container/flat_hash_map.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "deepmind/lua/vm_test_util.h"

namespace deepmind {
namespace lab {
namespace lua {
namespace {

constexpr char kTestString[] = "TestTest";
constexpr double kTestValue = 32.0;
constexpr double kTestRetValue = 32.0;
constexpr double kTestIntValue = 64;

extern "C" {
static int TestFunction(lua_State* L) {
  Push(L, kTestRetValue);
  return 1;
}
}  // extern "C"

using PushTest = testing::TestWithVm;

TEST_F(PushTest, PushFunction) {
  Push(L, TestFunction);
  ASSERT_EQ(LUA_TFUNCTION, lua_type(L, -1));
  ASSERT_EQ(0, lua_pcall(L, 0, 1, 0)) << lua_tostring(L, -1);
  ASSERT_EQ(LUA_TNUMBER, lua_type(L, -1));
  auto result = lua_tonumber(L, 1);
  EXPECT_EQ(kTestRetValue, result);
}

TEST_F(PushTest, PushCString) {
  Push(L, kTestString);
  ASSERT_EQ(LUA_TSTRING, lua_type(L, -1));
  std::size_t size;
  const char* result_ctr = lua_tolstring(L, 1, &size);
  std::string result(result_ctr, size);
  EXPECT_EQ(kTestString, result);
}

TEST_F(PushTest, PushString) {
  Push(L, std::string(kTestString));
  ASSERT_EQ(LUA_TSTRING, lua_type(L, -1));
  std::size_t size;
  const char* result_ctr = lua_tolstring(L, 1, &size);
  std::string result(result_ctr, size);
  EXPECT_EQ(kTestString, result);
}

TEST_F(PushTest, PushStringView) {
  Push(L, absl::string_view(kTestString));
  ASSERT_EQ(LUA_TSTRING, lua_type(L, -1));
  std::size_t size;
  const char* result_ctr = lua_tolstring(L, 1, &size);
  std::string result(result_ctr, size);
  EXPECT_EQ(kTestString, result);
}

TEST_F(PushTest, PushNumber) {
  Push(L, kTestValue);
  ASSERT_EQ(LUA_TNUMBER, lua_type(L, 1));
  auto result = lua_tonumber(L, 1);
  EXPECT_EQ(kTestValue, result);
}

TEST_F(PushTest, PushInteger) {
  Push(L, kTestIntValue);
  ASSERT_EQ(LUA_TNUMBER, lua_type(L, 1));
  auto result = lua_tointeger(L, 1);
  EXPECT_EQ(kTestIntValue, result);
}

TEST_F(PushTest, PushBool) {
  Push(L, true);
  ASSERT_EQ(LUA_TBOOLEAN, lua_type(L, 1));
  auto result = lua_toboolean(L, 1);
  EXPECT_EQ(true, result);
}

TEST_F(PushTest, PushVoidStar) {
  int test_value = 10;
  Push(L, &test_value);
  ASSERT_EQ(LUA_TLIGHTUSERDATA, lua_type(L, 1));
  int* result = static_cast<int*>(lua_touserdata(L, 1));
  EXPECT_EQ(test_value, *result);
}

TEST_F(PushTest, PushVector) {
  std::vector<double> test = {1, 2, 3, 4, 5};
  Push(L, test);
  ASSERT_EQ(LUA_TTABLE, lua_type(L, 1));

  std::size_t count = ArrayLength(L, 1);
  ASSERT_EQ(test.size(), count);
  for (std::size_t i = 0; i < count; ++i) {
    lua_rawgeti(L, 1, i + 1);
    ASSERT_EQ(LUA_TNUMBER, lua_type(L, -1));
    double value = lua_tonumber(L, -1);
    EXPECT_EQ(test[i], value);
    lua_pop(L, 1);
  }
}

TEST_F(PushTest, PushSpan) {
  const double data[] = {1.0, 2.0, 3.0, 4.0};
  Push(L, absl::MakeConstSpan(data));
  ASSERT_EQ(LUA_TTABLE, lua_type(L, 1));
  const std::size_t count = ArrayLength(L, 1);
  ASSERT_EQ(count, 4);

  for (std::size_t i = 0; i < count; ++i) {
    lua_rawgeti(L, 1, i + 1);
    ASSERT_EQ(LUA_TNUMBER, lua_type(L, -1));
    const double value = lua_tonumber(L, -1);
    EXPECT_EQ(data[i], value);
    lua_pop(L, 1);
  }
}

TEST_F(PushTest, PushFixedSizeArray) {
  std::array<double, 3> test_d{{1.0, 2.0, 3.0}};
  std::array<std::size_t, 3> test_sz{{1, 2, 3}};
  Push(L, test_d);
  ASSERT_EQ(LUA_TTABLE, lua_type(L, 1));
  Push(L, test_sz);
  ASSERT_EQ(LUA_TTABLE, lua_type(L, 2));

  std::size_t count = ArrayLength(L, 1);
  ASSERT_EQ(test_d.size(), count);

  ASSERT_EQ(test_sz.size(), ArrayLength(L, 2));

  for (std::size_t i = 0; i < count; ++i) {
    lua_rawgeti(L, 1, i + 1);
    ASSERT_EQ(LUA_TNUMBER, lua_type(L, -1));
    double value = lua_tonumber(L, -1);
    EXPECT_EQ(test_d[i], value);
    lua_pop(L, 1);
  }
}

TEST_F(PushTest, PushTable) {
  absl::flat_hash_map<std::string, double> test = {
      {"one", 1},  //
      {"2", 2.0},  //
      {"3", 3.0},  //
      {"4", 4.0},  //
      {"5", 5.0},  //
  };

  Push(L, test);
  ASSERT_EQ(LUA_TTABLE, lua_type(L, 1));

  std::size_t count = ArrayLength(L, 1);
  ASSERT_EQ(0, count);
  std::set<std::string> visited;

  lua_pushnil(L);
  while (lua_next(L, -2) != 0) {
    ASSERT_EQ(LUA_TSTRING, lua_type(L, -2));
    std::size_t key_size;
    const char* key_ctr = lua_tolstring(L, -2, &key_size);
    std::string key(key_ctr, key_size);

    ASSERT_EQ(LUA_TNUMBER, lua_type(L, -1));
    double value = lua_tonumber(L, -1);
    auto it = test.find(key);
    ASSERT_TRUE(it != test.end());
    EXPECT_EQ(it->second, value);
    visited.insert(key);
    lua_pop(L, 1);
  }

  EXPECT_EQ(visited.size(), test.size());
}

TEST_F(PushTest, PushVariant) {
  absl::variant<absl::string_view, int, double> value;

  // Push default constructed string_view.
  Push(L, value);
  ASSERT_EQ(LUA_TSTRING, lua_type(L, 1));
  EXPECT_STREQ(lua_tostring(L, 1), "");

  value = "2 - hello";
  Push(L, value);
  ASSERT_EQ(LUA_TSTRING, lua_type(L, 2));
  EXPECT_STREQ(lua_tostring(L, 2), "2 - hello");

  value = 3;
  Push(L, value);
  ASSERT_EQ(LUA_TNUMBER, lua_type(L, 3));
  EXPECT_EQ(lua_tonumber(L, 3), 3);

  value = 4.5;
  Push(L, value);
  ASSERT_EQ(LUA_TNUMBER, lua_type(L, 4));
  EXPECT_EQ(lua_tonumber(L, 4), 4.5);
  lua_settop(L, 0);
}

TEST_F(PushTest, PushVariantMonostate) {
  absl::variant<absl::monostate, int> value;
  Push(L, value);
  ASSERT_EQ(LUA_TNIL, lua_type(L, 1));
  value = 10;
  Push(L, value);
  ASSERT_EQ(LUA_TNUMBER, lua_type(L, 2));
  EXPECT_EQ(lua_tonumber(L, 2), 10);
  lua_settop(L, 0);
}


TEST_F(PushTest, PushVariantArray) {
  std::vector<absl::variant<absl::string_view, double>> values;
  values.emplace_back(10.5);
  values.emplace_back("Hello");
  Push(L, values);
  ASSERT_EQ(LUA_TTABLE, lua_type(L, 1));
  std::size_t count = ArrayLength(L, 1);
  ASSERT_EQ(count, 2);

  lua_rawgeti(L, 1, 1);
  ASSERT_EQ(LUA_TNUMBER, lua_type(L, 2));
  EXPECT_EQ(lua_tonumber(L, 2), 10.5);

  lua_rawgeti(L, 1, 2);
  ASSERT_EQ(LUA_TSTRING, lua_type(L, 3));
  EXPECT_STREQ(lua_tostring(L, 3), "Hello");
}

}  // namespace
}  // namespace lua
}  // namespace lab
}  // namespace deepmind
