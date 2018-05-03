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

#include "gtest/gtest.h"
#include "deepmind/support/test_srcdir.h"
#include "public/dmlab.h"

namespace deepmind {
namespace lab {
namespace {

const char* test_script = nullptr;

TEST(LuaUnitTest, RunsTest) {
  const std::string runfiles_path = TestSrcDir();
  const std::string test_script_path = runfiles_path + "/" + test_script;

  DeepMindLabLaunchParams params = {};
  params.runfiles_path = runfiles_path.c_str();
  params.renderer = DeepMindLabRenderer_Software;

  EnvCApi env_c_api;
  void* context;
  ASSERT_EQ(dmlab_connect(&params, &env_c_api, &context), 0);
  ASSERT_EQ(env_c_api.setting(context, "levelName", test_script_path.c_str()),
            0);
  ASSERT_EQ(env_c_api.setting(context, "datasetPath", "dummy"), 0);
  if (env_c_api.init(context) != 0) {
    ADD_FAILURE() << env_c_api.error_message(context);
  }
  env_c_api.release_context(context);
}

}  // namespace
}  // namespace lab
}  // namespace deepmind

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  deepmind::lab::test_script = argv[1];

  return RUN_ALL_TESTS();
}
