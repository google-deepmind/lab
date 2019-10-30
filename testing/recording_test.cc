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

#include <sys/stat.h>

#include <cstdlib>
#include <string>

#include "gtest/gtest.h"
#include "absl/container/flat_hash_map.h"
#include "deepmind/support/test_srcdir.h"
#include "deepmind/util/files.h"
#include "public/dmlab.h"
#include "testing/env_observation.h"
#include "testing/env_observation_util.h"

namespace deepmind {
namespace lab {
namespace {

using ::testing::UnitTest;

// A map used to simplify passing settings to dmlab before initialization.
using SettingsMap = absl::flat_hash_map<std::string, std::string>;

const int kFramesPerSecond = 30;

class RecordingTest : public ::testing::Test {
  std::string GetTestEnv(const char* env_name) {
    char* env = std::getenv(env_name);
    return env ? env : "";
  }

 protected:
  const std::string test_name;
  const std::string demo_path;
  const std::string runfiles_path;

  RecordingTest()
      : test_name(UnitTest::GetInstance()->current_test_info()->name()),
        demo_path(util::GetTempDirectory() + "/dmlab"),
        runfiles_path(TestSrcDir()) {}

  bool ApplyAllSettingsSuccessfully(const SettingsMap& settings, EnvCApi* api,
                                    void* context) {
    auto set_setting = [api, context](const SettingsMap::value_type& k_v) {
      return api->setting(context, k_v.first.c_str(), k_v.second.c_str()) == 0;
    };
    return std::all_of(settings.begin(), settings.end(), set_setting);
  }

  int ConfigureVM(EnvCApi* api, void* context) {
    // When running under TSAN, switch to the interpreted VM ("1"), which is
    // instrumentable.
#ifndef __has_feature
#  define __has_feature(x) 0
#endif
#if __has_feature(thread_sanitizer)
    return api->setting(context, "vmMode", "interpreted");
#else
    return 0;
#endif
  }

  SettingsMap DefaultSettings() {
    return {
      {"width", "64"},
      {"height", "64"},
      {"demofiles", demo_path},
      {"logToStdErr", "true"},
      {"fps", std::to_string(kFramesPerSecond)},
    };
  }

  ~RecordingTest() {
    util::RemoveDirectory(demo_path);
  }

  // Advances through one second of steps and grabs an observation toward the
  // end.
  EnvObservation<unsigned char> AdvanceOneSecond(EnvCApi* env_c_api,
                                                 void* context) {
    // Depending on when the final demo snapshot was taken on the server, the
    // menu may appear before the last demo advances through the final step, so
    // take a comparison observation just short of the end.
    const int kNumStepsAfterObservation = 3;

    EnvCApi_Observation observation;
    double reward;
    env_c_api->advance(
        context, kFramesPerSecond - kNumStepsAfterObservation, &reward);
    env_c_api->observation(context, 0, &observation);
    EnvObservation<unsigned char> recording_observation(observation);
    env_c_api->advance(context, kNumStepsAfterObservation, &reward);
    return recording_observation;
  }
};

// Ensure dmlab saves recordings to /demos/ on the home path.
TEST_F(RecordingTest, SavesRecordingToDemoDirectory) {
  DeepMindLabLaunchParams params = {};
  params.runfiles_path = runfiles_path.c_str();
  params.renderer = DeepMindLabRenderer_Software;

  EnvCApi env_c_api;
  void* context;
  ASSERT_EQ(dmlab_connect(&params, &env_c_api, &context), 0);

  SettingsMap settings = DefaultSettings();
  settings["levelName"] = "tests/recording_test";
  settings["record"] = test_name;
  ASSERT_TRUE(ApplyAllSettingsSuccessfully(settings, &env_c_api, context));
  ASSERT_EQ(0, ConfigureVM(&env_c_api, context));

  env_c_api.init(context);
  ASSERT_EQ(env_c_api.start(context, 0 /*episode*/, 1 /*seed*/), 0)
      << "Unable to start recording";

  // Record one second of frames. Repeat twice to advance through two maps.
  double reward;
  env_c_api.advance(context, kFramesPerSecond, &reward);
  env_c_api.advance(context, kFramesPerSecond, &reward);

  env_c_api.release_context(context);

  // Check demo files exist.
  struct stat path_stat;
  std::string demo_1_path(demo_path + "/demos/" + test_name + "/00001.dm_71");
  EXPECT_EQ(stat(demo_1_path.c_str(), &path_stat), 0) <<
      "Demo 1 file does not exist: " << demo_1_path;
  std::string demo_2_path(demo_path + "/demos/" + test_name + "/00002.dm_71");
  EXPECT_EQ(stat(demo_2_path.c_str(), &path_stat), 0) <<
      "Demo 2 file does not exist: " << demo_2_path;
}

// Ensure dmlab plays a demo.
TEST_F(RecordingTest, PlaysDemoFromDemoDirectory) {
  DeepMindLabLaunchParams params = {};
  params.runfiles_path = runfiles_path.c_str();
  params.renderer = DeepMindLabRenderer_Software;

  EnvCApi env_c_api;
  void* context;
  ASSERT_EQ(dmlab_connect(&params, &env_c_api, &context), 0);

  SettingsMap settings = DefaultSettings();
  settings["levelName"] = "tests/recording_test";
  settings["record"] = test_name;
  ASSERT_TRUE(ApplyAllSettingsSuccessfully(settings, &env_c_api, context));
  ASSERT_EQ(0, ConfigureVM(&env_c_api, context));

  env_c_api.init(context);
  env_c_api.start(context, 0 /*episode*/, 1 /*seed*/);

  // Record one second of frames. Repeat twice to advance through two maps.
  EnvObservation<unsigned char> recording_1_observation =
      AdvanceOneSecond(&env_c_api, context);
  EnvObservation<unsigned char> recording_2_observation =
      AdvanceOneSecond(&env_c_api, context);

  env_c_api.release_context(context);

  // Start the context for the demo
  ASSERT_EQ(dmlab_connect(&params, &env_c_api, &context), 0);

  settings = DefaultSettings();
  settings["levelName"] = "tests/recording_test";
  settings["demo"] = test_name;
  ASSERT_TRUE(ApplyAllSettingsSuccessfully(settings, &env_c_api, context));
  ASSERT_EQ(0, ConfigureVM(&env_c_api, context));

  env_c_api.init(context);
  ASSERT_EQ(env_c_api.start(context, 0 /*episode*/, 1 /*seed*/), 0)
      << "Unable to start demo";

  // Play back one second of frames.
  EnvObservation<unsigned char> demo_1_observation =
      AdvanceOneSecond(&env_c_api, context);
  EnvObservation<unsigned char> demo_2_observation =
      AdvanceOneSecond(&env_c_api, context);

  EXPECT_LT(CompareInterlacedObservations(
      recording_1_observation, demo_1_observation, 2), 1.0);
  EXPECT_LT(CompareInterlacedObservations(
      recording_2_observation, demo_2_observation, 2), 1.0);

  EXPECT_GT(CompareInterlacedObservations(
      recording_1_observation, demo_2_observation, 2), 1.0);
  env_c_api.release_context(context);

  if (char* undeclared_outputs = std::getenv("TEST_UNDECLARED_OUTPUTS_DIR")) {
    std::string jpeg(std::string(undeclared_outputs) + "/" + test_name + ".");
    SaveInterlacedRGBObservationToJpg(recording_1_observation, jpeg + "R1.jpg");
    SaveInterlacedRGBObservationToJpg(recording_2_observation, jpeg + "R2.jpg");
    SaveInterlacedRGBObservationToJpg(demo_1_observation, jpeg + "D1.jpg");
    SaveInterlacedRGBObservationToJpg(demo_2_observation, jpeg + "D2.jpg");
  }
}

// Ensure dmlab records a video.
TEST_F(RecordingTest, RecordsMultipleVideos) {
  DeepMindLabLaunchParams params = {};
  params.runfiles_path = runfiles_path.c_str();
  params.renderer = DeepMindLabRenderer_Software;

  EnvCApi env_c_api;
  void* context;
  ASSERT_EQ(dmlab_connect(&params, &env_c_api, &context), 0);

  SettingsMap settings = DefaultSettings();
  settings["levelName"] = "tests/recording_test";
  settings["record"] = test_name;
  ASSERT_TRUE(ApplyAllSettingsSuccessfully(settings, &env_c_api, context));
  ASSERT_EQ(0, ConfigureVM(&env_c_api, context));

  env_c_api.init(context);
  env_c_api.start(context, 0 /*episode*/, 1 /*seed*/);

  // Record one second of frames. Repeat twice to advance through two maps.
  AdvanceOneSecond(&env_c_api, context);
  AdvanceOneSecond(&env_c_api, context);
  env_c_api.release_context(context);

  // Start the context for the demo
  ASSERT_EQ(dmlab_connect(&params, &env_c_api, &context), 0);

  settings = DefaultSettings();
  settings["levelName"] = "tests/recording_test";
  settings["demo"] = test_name;
  settings["video"] = test_name;
  ASSERT_TRUE(ApplyAllSettingsSuccessfully(settings, &env_c_api, context));
  ASSERT_EQ(0, ConfigureVM(&env_c_api, context));

  env_c_api.init(context);
  ASSERT_EQ(env_c_api.start(context, 0 /*episode*/, 1 /*seed*/), 0)
      << "Unable to start demo";

  // Play back one second of frames.
  AdvanceOneSecond(&env_c_api, context);
  AdvanceOneSecond(&env_c_api, context);

  env_c_api.release_context(context);

  // Check video files exist.
  struct stat path_stat;
  std::string video_1_path(demo_path + "/videos/" + test_name + "/00001.avi");
  EXPECT_EQ(stat(video_1_path.c_str(), &path_stat), 0) <<
      "Video 1 file does not exist: " << video_1_path;
  std::string video_2_path(demo_path + "/videos/" + test_name + "/00002.avi");
  EXPECT_EQ(stat(video_2_path.c_str(), &path_stat), 0) <<
      "Video 2 file does not exist: " << video_2_path;
}


TEST_F(RecordingTest, TotalScorePreserved) {
  DeepMindLabLaunchParams params = {};
  params.runfiles_path = runfiles_path.c_str();
  params.renderer = DeepMindLabRenderer_Software;

  EnvCApi env_c_api;
  void* context;
  ASSERT_EQ(dmlab_connect(&params, &env_c_api, &context), 0);

  SettingsMap settings = DefaultSettings();
  settings["levelName"] = "tests/recording_test";
  settings["record"] = test_name;
  ASSERT_TRUE(ApplyAllSettingsSuccessfully(settings, &env_c_api, context));
  ASSERT_EQ(0, ConfigureVM(&env_c_api, context));

  env_c_api.init(context);
  ASSERT_EQ(env_c_api.start(context, 0 /*episode*/, 1 /*seed*/), 0)
      << "Unable to start recording";

  // Record one second of frames. Repeat twice to advance through two maps.
  double recording_reward_1;
  const int move_forward[] = {0, 0, 0, 1, 0, 0, 0};
  env_c_api.act_discrete(context, move_forward);
  env_c_api.advance(context, kFramesPerSecond, &recording_reward_1);
  double recording_reward_2;
  env_c_api.advance(context, kFramesPerSecond, &recording_reward_2);

  env_c_api.release_context(context);

  // Start the context for the demo
  ASSERT_EQ(dmlab_connect(&params, &env_c_api, &context), 0);

  settings = DefaultSettings();
  settings["levelName"] = "tests/recording_test";
  settings["demo"] = test_name;
  ASSERT_TRUE(ApplyAllSettingsSuccessfully(settings, &env_c_api, context));
  ASSERT_EQ(0, ConfigureVM(&env_c_api, context));

  env_c_api.init(context);
  ASSERT_EQ(env_c_api.start(context, 0 /*episode*/, 1 /*seed*/), 0)
      << "Unable to start demo";

  // Play back one second of frames for two maps.
  double demo_reward_1;
  env_c_api.advance(context, kFramesPerSecond, &demo_reward_1);
  double demo_reward_2;
  env_c_api.advance(context, kFramesPerSecond, &demo_reward_2);
  env_c_api.release_context(context);

  EXPECT_EQ(recording_reward_1, 1);
  EXPECT_EQ(recording_reward_2, 2);
  EXPECT_EQ(demo_reward_1, recording_reward_1);
  EXPECT_EQ(demo_reward_2, recording_reward_2);
}

TEST_F(RecordingTest, MissingDemoFilesSetsError) {
  DeepMindLabLaunchParams params = {};
  params.runfiles_path = runfiles_path.c_str();
  params.renderer = DeepMindLabRenderer_Software;

  EnvCApi env_c_api;
  void* context;

  // Start the context for the demo
  ASSERT_EQ(dmlab_connect(&params, &env_c_api, &context), 0);

  SettingsMap settings = DefaultSettings();
  settings["levelName"] = "tests/recording_test";
  settings["demo"] = test_name;
  ASSERT_TRUE(ApplyAllSettingsSuccessfully(settings, &env_c_api, context));
  ASSERT_EQ(0, ConfigureVM(&env_c_api, context));

  env_c_api.init(context);
  ASSERT_EQ(env_c_api.start(context, 0 /*episode*/, 1 /*seed*/), 1)
      << "Should have failed to start demo";
}

}  // namespace
}  // namespace lab
}  // namespace deepmind
