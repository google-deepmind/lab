#include <string>

#include "absl/log/check.h"
#include "deepmind/support/test_srcdir.h"
#include "deepmind/util/files.h"
#include "gtest/gtest.h"
#include "public/dmlab.h"
#include "third_party/rl_api/env_c_api.h"

namespace deepmind {
namespace lab {
namespace {

const char* test_script = nullptr;

// Invokes seed_test.lua to check that the random seed is initialised in the
// start method of the level.
TEST(SeedLevelTest, RandomSeedTest) {
  const std::string runfiles_path = TestSrcDir();

  DeepMindLabLaunchParams params = {};
  params.runfiles_path = runfiles_path.c_str();
  params.renderer = DeepMindLabRenderer_Software;

  EnvCApi env_c_api;
  void* context;
  ASSERT_EQ(dmlab_connect(&params, &env_c_api, &context), 0);
  ASSERT_EQ(env_c_api.setting(context, "levelName", "tests/seed_test"), 0)
      << env_c_api.error_message(context);
  ASSERT_EQ(env_c_api.setting(context, "testLevelScript", test_script), 0)
      << env_c_api.error_message(context);

// When running under TSAN, switch to the interpreted VM ("1"), which is
// instrumentable.
#ifndef __has_feature
#define __has_feature(x) 0
#endif
#if __has_feature(thread_sanitizer)
  ASSERT_EQ(env_c_api.setting(context, "vmMode", "interpreted"), 0)
      << env_c_api.error_message(context);
#endif

  ASSERT_EQ(env_c_api.init(context), 0) << env_c_api.error_message(context);
  env_c_api.release_context(context);
}

}  // namespace
}  // namespace lab
}  // namespace deepmind

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  QCHECK_EQ(argc, 2) << "Usage: load_level_test <filename>";
  deepmind::lab::test_script = argv[1];

  return RUN_ALL_TESTS();
}
