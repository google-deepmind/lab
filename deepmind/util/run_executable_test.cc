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
