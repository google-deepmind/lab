#include "deepmind/lua/stack_resetter.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "deepmind/lua/call.h"
#include "deepmind/lua/n_results_or.h"
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
using ::testing::HasSubstr;

using StackResetterTest = testing::TestWithVm;

TEST_F(StackResetterTest, Reset) {
  ASSERT_EQ(lua_gettop(L), 0);
  {
    StackResetter stack_resetter(L);
    Push(L, 10);
    {
      StackResetter stack_resetter(L);
      Push(L, 20);
      EXPECT_EQ(lua_gettop(L), 2);
    }
    EXPECT_EQ(lua_gettop(L), 1);
  }
  EXPECT_EQ(lua_gettop(L), 0);
}

constexpr const char kFunctionScript[] = R"(
local arg1 = ...
if not arg1 then
  error("No arg1")
end
return arg1
)";

TEST_F(StackResetterTest, ResetOverError) {
  ASSERT_EQ(lua_gettop(L), 0);
  {
    StackResetter stack_resetter(L);
    EXPECT_THAT(PushScript(L, kFunctionScript, "kFunctionScript"),
                IsOkAndHolds(1));
    EXPECT_THAT(Call(L, 0), StatusIs(HasSubstr("No arg1")));
  }
  EXPECT_EQ(lua_gettop(L), 0);
}

TEST_F(StackResetterTest, ResetWithReturnValues) {
  ASSERT_EQ(lua_gettop(L), 0);
  {
    StackResetter stack_resetter(L);
    EXPECT_THAT(PushScript(L, kFunctionScript, "kFunctionScript"),
                IsOkAndHolds(1));
    Push(L, 10);
    EXPECT_THAT(Call(L, 1), IsOkAndHolds(1));
    int value = 0;
    EXPECT_TRUE(IsFound(Read(L, 1, &value)));
    EXPECT_EQ(value, 10);
    EXPECT_EQ(lua_gettop(L), 1);
  }
  EXPECT_EQ(lua_gettop(L), 0);
}

}  // namespace
}  // namespace lua
}  // namespace lab
}  // namespace deepmind
