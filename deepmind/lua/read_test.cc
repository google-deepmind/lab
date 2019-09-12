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
#include "absl/container/flat_hash_map.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "absl/types/variant.h"
#include "deepmind/lua/push.h"
#include "deepmind/lua/vm_test_util.h"

namespace deepmind {
namespace lab {
namespace lua {
namespace {

using ::testing::HasSubstr;

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
  Push(L, false);
  lua_CFunction c_function;
  ASSERT_TRUE(IsFound(Read(L, 1, &c_function)));
  EXPECT_TRUE(TestFunction == c_function);
  EXPECT_TRUE(IsTypeMismatch(Read(L, 2, &c_function)));
  EXPECT_TRUE(IsNotFound(Read(L, 3, &c_function)));
  EXPECT_TRUE(TestFunction == c_function);
}

TEST_F(ReadTest, RawStringRead) {
  Push(L, kTestString);
  absl::string_view result = RawStringRead(L, 1);
  EXPECT_EQ(kTestString, result);
}

TEST_F(ReadTest, ReadString) {
  Push(L, kTestString);
  Push(L, false);
  std::string result;
  ASSERT_TRUE(IsFound(Read(L, 1, &result)));
  EXPECT_EQ(kTestString, result);
  EXPECT_TRUE(IsTypeMismatch(Read(L, 2, &result)));
  EXPECT_TRUE(IsNotFound(Read(L, 3, &result)));
  EXPECT_EQ(kTestString, result);
}

TEST_F(ReadTest, ReadStringView) {
  Push(L, kTestString);
  Push(L, false);
  absl::string_view result;
  ASSERT_TRUE(IsFound(Read(L, 1, &result)));
  EXPECT_EQ(kTestString, result);
  EXPECT_TRUE(IsTypeMismatch(Read(L, 2, &result)));
  EXPECT_TRUE(IsNotFound(Read(L, 3, &result)));
  EXPECT_EQ(kTestString, result);
}

TEST_F(ReadTest, ReadNumber) {
  Push(L, kTestValue);
  Push(L, false);
  double result;
  EXPECT_TRUE(IsFound(Read(L, 1, &result)));
  EXPECT_TRUE(IsTypeMismatch(Read(L, 2, &result)));
  EXPECT_TRUE(IsNotFound(Read(L, 3, &result)));
  EXPECT_EQ(kTestValue, result);
}

TEST_F(ReadTest, ReadUnsigned) {
  Push(L, 20);
  Push(L, -20);
  Push(L, false);
  std::size_t result;
  ASSERT_TRUE(IsFound(Read(L, 1, &result)));
  EXPECT_EQ(20, result);
  // Should not read a negative value.
  ASSERT_TRUE(IsTypeMismatch(Read(L, 2, &result)));
  EXPECT_EQ(20, result);
  // Should not read a negative value.
  EXPECT_TRUE(IsTypeMismatch(Read(L, 3, &result)));
  EXPECT_TRUE(IsNotFound(Read(L, 4, &result)));
  EXPECT_EQ(20, result);
}

TEST_F(ReadTest, ReadBoolean) {
  Push(L, true);
  Push(L, false);
  Push(L, 1);
  bool result = false;
  ASSERT_TRUE(IsFound(Read(L, 1, &result)));
  EXPECT_EQ(true, result);
  ASSERT_TRUE(IsFound(Read(L, 2, &result)));
  EXPECT_EQ(false, result);
  EXPECT_TRUE(IsTypeMismatch(Read(L, 3, &result)));
  EXPECT_TRUE(IsNotFound(Read(L, 4, &result)));
}

TEST_F(ReadTest, ReadLightUserData) {
  int test_value = 10;
  Push(L, &test_value);
  Push(L, false);
  int* result;
  ASSERT_TRUE(IsFound(Read(L, 1, &result)));
  ASSERT_EQ(&test_value, result);
  EXPECT_EQ(test_value, *result);

  EXPECT_TRUE(IsTypeMismatch(Read(L, 2, &result)));
  EXPECT_TRUE(IsNotFound(Read(L, 3, &result)));
}

TEST_F(ReadTest, ReadVector) {
  std::vector<double> test = {1, 2, 3, 4, 5};
  Push(L, "Junk");
  Push(L, test);
  Push(L, "Junk");
  std::vector<double> result;
  ASSERT_TRUE(IsFound(Read(L, 2, &result)));
  EXPECT_EQ(result, test);
  ASSERT_TRUE(IsFound(Read(L, -2, &result)));
  EXPECT_EQ(result, test);
  EXPECT_TRUE(IsTypeMismatch(Read(L, 1, &result)));
  EXPECT_TRUE(IsTypeMismatch(Read(L, 3, &result)));
  EXPECT_TRUE(IsNotFound(Read(L, 4, &result)));
}

TEST_F(ReadTest, ReadArray) {
  std::array<double, 5> test{{1, 2, 3, 4, 5}};
  Push(L, "Junk");
  Push(L, test);
  Push(L, "Junk");
  std::array<double, 5> result;
  ASSERT_TRUE(IsFound(Read(L, 2, &result)));
  EXPECT_EQ(result, test);
  ASSERT_TRUE(IsFound(Read(L, -2, &result)));
  EXPECT_EQ(result, test);
  EXPECT_EQ(3, lua_gettop(L));
  EXPECT_TRUE(IsTypeMismatch(Read(L, 1, &result)));
  EXPECT_TRUE(IsTypeMismatch(Read(L, 3, &result)));
  EXPECT_TRUE(IsNotFound(Read(L, 4, &result)));
  EXPECT_EQ(result, test);
}

TEST_F(ReadTest, ReadArrayFloat) {
  std::array<float, 5> test{{1, 2, 3, 4, 5}};
  Push(L, "Junk");
  Push(L, test);
  Push(L, "Junk");
  std::array<float, 5> result;
  ASSERT_TRUE(IsFound(Read(L, 2, &result)));
  EXPECT_EQ(result, test);
  ASSERT_TRUE(IsFound(Read(L, -2, &result)));
  EXPECT_EQ(result, test);
  EXPECT_EQ(3, lua_gettop(L));
  std::array<bool, 5> result_mismatch_type;
  EXPECT_TRUE(IsTypeMismatch(Read(L, 2, &result_mismatch_type)));

  std::array<float, 6> result_mismatch_size;
  EXPECT_TRUE(IsTypeMismatch(Read(L, 2, &result_mismatch_size)));
}

TEST_F(ReadTest, ReadSpan) {
  const std::array<const float, 5> test{{1, 2, 3, 4, 5}};
  Push(L, test);
  Push(L, false);
  float result[] = {0, 0, 0, 0, 0};
  ASSERT_TRUE(IsFound(Read(L, 1, absl::MakeSpan(result))));
  EXPECT_EQ(absl::MakeSpan(result), test);
  EXPECT_EQ(2, lua_gettop(L));
  EXPECT_TRUE(IsTypeMismatch(Read(L, 2, absl::MakeSpan(result))));
  EXPECT_TRUE(IsNotFound(Read(L, 3, absl::MakeSpan(result))));
  bool result_mismatch[] = {false, false, false, false, false};
  EXPECT_TRUE(IsTypeMismatch(Read(L, 1, absl::MakeSpan(result_mismatch))));
}

TEST_F(ReadTest, ReadTable) {
  absl::flat_hash_map<std::string, double> test = {
      {"one", 1},  //
      {"2", 2.0},  //
      {"3", 3.0},  //
      {"4", 4.0},  //
      {"5", 5.0},  //
  };

  Push(L, "Junk");
  Push(L, test);
  Push(L, "Junk");
  absl::flat_hash_map<std::string, double> result;
  ASSERT_TRUE(IsFound(Read(L, 2, &result)));
  EXPECT_EQ(result, test);
  ASSERT_TRUE(IsFound(Read(L, -2, &result)));
  EXPECT_EQ(result, test);
  EXPECT_EQ(3, lua_gettop(L));
  EXPECT_TRUE(IsTypeMismatch(Read(L, 1, &result)));
  EXPECT_TRUE(IsNotFound(Read(L, 4, &result)));

  absl::flat_hash_map<std::string, std::string> result_missmatch_value;
  EXPECT_TRUE(IsTypeMismatch(Read(L, 2, &result_missmatch_value)));

  absl::flat_hash_map<double, double> result_missmatch_key;
  EXPECT_TRUE(IsTypeMismatch(Read(L, 2, &result_missmatch_value)));
}

TEST_F(ReadTest, ReadVariant) {
  absl::variant<absl::string_view, int, bool> var;

  Push(L, kTestString);
  ASSERT_TRUE(IsFound(lua::Read(L, 1, &var)));
  ASSERT_EQ(var.index(), 0);
  EXPECT_EQ(kTestString, absl::get<0>(var));

  Push(L, 10);
  ASSERT_TRUE(IsFound(lua::Read(L, 2, &var)));
  ASSERT_EQ(var.index(), 1);
  EXPECT_EQ(10, absl::get<1>(var));

  Push(L, true);
  ASSERT_TRUE(IsFound(lua::Read(L, 3, &var)));
  ASSERT_EQ(var.index(), 2);
  EXPECT_EQ(true, absl::get<2>(var));
}

TEST_F(ReadTest, ReadVariantTypeMismatch) {
  absl::variant<int, bool> var;
  Push(L, kTestString);
  EXPECT_TRUE(IsTypeMismatch(lua::Read(L, 1, &var)));
}

TEST_F(ReadTest, ReadVariantMissing) {
  absl::variant<int, bool> var;
  EXPECT_TRUE(IsNotFound(lua::Read(L, 1, &var)));
  lua_pushnil(L);
  EXPECT_TRUE(IsNotFound(lua::Read(L, 1, &var)));
}

TEST_F(ReadTest, ReadVariantVectorFail) {
  std::vector<absl::variant<bool, int>> vars;
  std::array<const char*, 5> string_args{{"0", "1", "2", "3", "4"}};
  Push(L, string_args);
  EXPECT_TRUE(IsTypeMismatch(lua::Read(L, 1, &vars)));
}

TEST_F(ReadTest, ToString) {
  Push(L, kTestString);
  EXPECT_EQ(kTestString, ToString(L, 1));
  lua_pop(L, 1);

  Push(L, 1);
  EXPECT_EQ("1", ToString(L, 1));
  lua_pop(L, 1);

  Push(L, TestFunction);
  EXPECT_THAT(ToString(L, 1), HasSubstr("function"));
  lua_pop(L, 1);

  int i = 10;
  Push(L, &i);
  EXPECT_THAT(ToString(L, 1), HasSubstr("pointer"));
  lua_pop(L, 1);

  Push(L, std::vector<int>());
  EXPECT_THAT(ToString(L, 1), HasSubstr("table"));
  lua_pop(L, 1);

  Push(L, true);
  EXPECT_EQ("true", ToString(L, 1));
  lua_pop(L, 1);

  Push(L, false);
  EXPECT_EQ("false", ToString(L, 1));
  lua_pop(L, 1);

  lua_pushnil(L);
  EXPECT_THAT(ToString(L, 1), HasSubstr("nil"));
  lua_pop(L, 1);

  EXPECT_THAT(ToString(L, 1), HasSubstr("none"));

  int* value = static_cast<int*>(lua_newuserdata(L, sizeof(int)));
  *value = 0;
  EXPECT_THAT(lua::ToString(L, 1), HasSubstr("user pointer"));
  lua_pop(L, 1);
}

TEST_F(ReadTest, UserDataToString) {
  int* value = static_cast<int*>(lua_newuserdata(L, sizeof(int)));
  lua_createtable(L, 0, 1);
  lua_pushstring(L, "__tostring");
  lua_pushcclosure(
      L,
      +[](lua_State* L) {
        int* val = static_cast<int*>(lua_touserdata(L, 1));
        if (val != nullptr) {
          lua::Push(L, absl::StrCat("MyClass - ", *val));
        } else {
          lua::Push(L, absl::StrCat("Invalid call to __tostring"));
        }
        return 1;
      },
      0);
  lua_settable(L, 2);
  lua_setmetatable(L, 1);
  *value = 5;
  EXPECT_THAT(lua::ToString(L, 1), HasSubstr("MyClass - 5"));
  *value = 10;
  EXPECT_THAT(lua::ToString(L, 1), HasSubstr("MyClass - 10"));
}

}  // namespace
}  // namespace lua
}  // namespace lab
}  // namespace deepmind
