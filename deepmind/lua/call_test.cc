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

#include "deepmind/lua/call.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "deepmind/lua/bind.h"
#include "deepmind/lua/n_results_or_test_util.h"
#include "deepmind/lua/push.h"
#include "deepmind/lua/push_script.h"
#include "deepmind/lua/read.h"
#include "deepmind/lua/vm_test_util.h"

namespace deepmind {
namespace lab {
namespace lua {
namespace {

using ::deepmind::lab::lua::testing::IsOkAndHolds;
using ::deepmind::lab::lua::testing::StatusIs;
using ::testing::AllOf;
using ::testing::HasSubstr;
using ::testing::Not;

constexpr char kTestAssert[] = R"(
local args = {...}
local function TestFunction(is_success)
  assert(is_success, "Random Error Message!")
end
TestFunction(args[1])
return "Success"
)";

using CallTest = testing::TestWithVm;

TEST_F(CallTest, CallsFunction) {
  int top = lua_gettop(L);

  NResultsOr n_or = PushScript(L, kTestAssert, "kTestAssert");
  ASSERT_THAT(n_or, IsOkAndHolds(lua_gettop(L) - top));

  Push(L, true);

  n_or = Call(L, 1);
  ASSERT_THAT(n_or, IsOkAndHolds(lua_gettop(L) - top));

  std::string result;
  ASSERT_TRUE(IsFound(Read(L, -1, &result)));
  EXPECT_EQ(result, "Success");
}

TEST_F(CallTest, FunctionErrors) {
  int top = lua_gettop(L);

  NResultsOr n_or = PushScript(L, kTestAssert, "kTestAssert");
  ASSERT_THAT(n_or, IsOkAndHolds(lua_gettop(L) - top));

  Push(L, false);

  EXPECT_THAT(Call(L, 1), StatusIs(AllOf(
      HasSubstr("Random Error Message!"),
      HasSubstr("TestFunction"),
      HasSubstr("kTestAssert"))));

  EXPECT_EQ(lua_gettop(L), top);
}

TEST_F(CallTest, FunctionErrorsNoStack) {
  int top = lua_gettop(L);

  NResultsOr n_or = PushScript(L, kTestAssert, "kTestAssert");
  ASSERT_THAT(n_or, IsOkAndHolds(lua_gettop(L) - top));

  Push(L, false);

  EXPECT_THAT(Call(L, 1, false), StatusIs(AllOf(
      HasSubstr("Random Error Message!"),
      Not(HasSubstr("TestFunction")))));

  EXPECT_EQ(lua_gettop(L), top);
}

NResultsOr TestCFuntion(lua_State* L) {
  bool should_be_success = false;
  if (IsTypeMismatch(Read(L, 1, &should_be_success))) {
    return "Type missmatch!";
  }
  Push(L, "What happens?");
  if (should_be_success) {
    return 1;
  }
  return "My error message!";
}

TEST_F(CallTest, FunctionBindErrors) {
  int top = lua_gettop(L);

  Push(L, Bind<TestCFuntion>);
  EXPECT_EQ(lua_gettop(L), top + 1);
  Push(L, false);

  auto call_result = Call(L, 1);
  EXPECT_THAT(call_result, StatusIs(HasSubstr("My error message!")));

  EXPECT_EQ(lua_gettop(L), top + call_result.n_results());
}

TEST_F(CallTest, FunctionBindSuccess) {
  int top = lua_gettop(L);

  Push(L, Bind<TestCFuntion>);
  EXPECT_EQ(lua_gettop(L), top + 1);
  Push(L, true);

  auto call_result = Call(L, 1);
  ASSERT_THAT(call_result, IsOkAndHolds(1));

  std::string message;
  ASSERT_TRUE(IsFound(Read(L, -1, &message)));
  EXPECT_EQ("What happens?", message);

  EXPECT_EQ(lua_gettop(L), top + call_result.n_results());
}

}  // namespace
}  // namespace lua
}  // namespace lab
}  // namespace deepmind
