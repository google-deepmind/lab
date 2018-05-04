// Copyright (C) 2017-2018 Google Inc.
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

#include "deepmind/util/run_executable.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace deepmind {
namespace lab {
namespace {

using ::testing::HasSubstr;

TEST(RunExecutableTest, Success) {
  std::string message;
  EXPECT_TRUE(util::RunExecutable("exit 0", &message));
  EXPECT_THAT(message, HasSubstr("success"));
}

TEST(RunExecutableTest, ReturnError) {
  std::string message;
  EXPECT_FALSE(util::RunExecutable("exit 13", &message));
  EXPECT_THAT(message, HasSubstr("fail"));
  EXPECT_THAT(message, HasSubstr("13"));
}

TEST(RunExecutableTest, CannotFindCommand) {
  std::string message;
  EXPECT_FALSE(util::RunExecutable("./invalid_command", &message));
  EXPECT_THAT(message, HasSubstr("fail"));
  EXPECT_THAT(message, HasSubstr("127"));
}

TEST(RunExecutableTest, WithOutput) {
  std::string message;
  std::string output;
  EXPECT_TRUE(util::RunExecutableWithOutput("echo Hello", &message, &output));
  EXPECT_EQ(output, "Hello\n");
  EXPECT_THAT(message, HasSubstr("success"));
}

TEST(RunExecutableTest, WithOutputAndError) {
  std::string message;
  std::string output;
  EXPECT_FALSE(util::RunExecutableWithOutput("(echo Hello; exit 17)", &message,
                                             &output));
  EXPECT_EQ(output, "Hello\n");
  EXPECT_THAT(message, HasSubstr("fail"));
  EXPECT_THAT(message, HasSubstr("17"));
}

}  // namespace
}  // namespace lab
}  // namespace deepmind
