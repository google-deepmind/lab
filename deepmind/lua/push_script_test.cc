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

#include "deepmind/lua/push_script.h"

#include "file/util/temp_file.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "deepmind/lua/bind.h"
#include "deepmind/lua/call.h"
#include "deepmind/lua/push.h"
#include "deepmind/lua/vm_test_util.h"

namespace deepmind {
namespace lab {
namespace lua {
namespace {

constexpr char kTestGoodScript[] = R"(
local args = {...}
local function TestFunction(is_success)
  assert(is_success, "Random Error Message!")
end
TestFunction(args[1])
return "Success"
)";

using PushScriptTest = testing::TestWithVm;

TEST_F(PushScriptTest, GoodScript) {
  auto script_result = PushScript(L, kTestGoodScript, "kTestGoodScript");
  EXPECT_TRUE(script_result.ok());
}

constexpr char kTestBadScript[] = R"(
local function TestFunction(is_success)
  -- Assert is missing trailing bracket.
  assert(is_success, "Random Error Message!"
end
TestFunction(args[1])
return "Success"
)";

TEST_F(PushScriptTest, BadScript) {
  auto script_result = PushScript(L, kTestBadScript, "kTestBadScript");
  ASSERT_FALSE(script_result.ok());
  EXPECT_THAT(script_result.error(),
              ::testing::HasSubstr("')' expected (to close '('"));
  EXPECT_THAT(script_result.error(), ::testing::HasSubstr("kTestBadScript"));
}

TEST_F(PushScriptTest, GoodScriptFile) {
  const std::string filename =
      TempFile::CreateAndCloseOrDie(FLAGS_test_tmpdir.c_str(), kTestGoodScript);
  auto script_result = PushScriptFile(L, filename);
  EXPECT_TRUE(script_result.ok()) << script_result.error();
}

TEST_F(PushScriptTest, FileMissing) {
  const std::string filename = "Error";
  auto script_result = PushScriptFile(L, filename);
  ASSERT_FALSE(script_result.ok());
  EXPECT_THAT(script_result.error(), ::testing::HasSubstr("open"));
}

TEST_F(PushScriptTest, BadScriptFile) {
  const std::string filename =
      TempFile::CreateAndCloseOrDie(FLAGS_test_tmpdir.c_str(), kTestBadScript);
  auto script_result = PushScriptFile(L, filename);
  ASSERT_FALSE(script_result.ok());
  EXPECT_THAT(script_result.error(),
              ::testing::HasSubstr("')' expected (to close '('"));
}

}  // namespace
}  // namespace lua
}  // namespace lab
}  // namespace deepmind
