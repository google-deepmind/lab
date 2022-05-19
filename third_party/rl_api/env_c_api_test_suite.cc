// Copyright 2019 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
////////////////////////////////////////////////////////////////////////////////

#include "third_party/rl_api/env_c_api_test_suite.h"

#include <vector>

#include "absl/log/log.h"
#include "gmock/gmock.h"

namespace rl_api {
namespace {

TEST_P(EnvCApiConformanceTestSuite, RunConformanceTest) {
  EnvCApi api;
  void* context;

  const EnvCApiTestParam& p = GetParam();
  ASSERT_EQ(p.connect_fn(&api, &context), 0);

  // Setup

  ASSERT_TRUE(api.setting != nullptr);
  ASSERT_TRUE(api.start != nullptr);
  ASSERT_TRUE(api.init != nullptr);
  ASSERT_TRUE(api.start != nullptr);
  ASSERT_TRUE(api.release_context != nullptr);
  ASSERT_TRUE(api.error_message != nullptr);

  // Meta data functions , must exist unconditionally

  ASSERT_TRUE(api.environment_name != nullptr);

  ASSERT_TRUE(api.write_property != nullptr);
  ASSERT_TRUE(api.read_property != nullptr);
  ASSERT_TRUE(api.list_property != nullptr);

  ASSERT_TRUE(api.action_discrete_count != nullptr);
  ASSERT_TRUE(api.action_continuous_count != nullptr);
  ASSERT_TRUE(api.action_text_count != nullptr);
  ASSERT_TRUE(api.observation_count != nullptr);
  ASSERT_TRUE(api.event_type_count != nullptr);

  // Run loop functions, must exist unconditionally

  ASSERT_TRUE(api.act_discrete != nullptr);
  ASSERT_TRUE(api.act_continuous != nullptr);
  ASSERT_TRUE(api.act_text != nullptr);
  ASSERT_TRUE(api.advance != nullptr);

  // Conditional checks, depending on runtime behaviour of the environment.

  for (const auto& setting : p.settings) {
    ASSERT_EQ(
        api.setting(context, setting.key.c_str(), setting.value.c_str()), 0)
        << "API error: " << api.error_message(context);
  }

  ASSERT_EQ(api.init(context), 0)
      << "API error: " << api.error_message(context);

  const int action_discrete_count = api.action_discrete_count(context);
  const int action_continuous_count = api.action_continuous_count(context);
  const int action_text_count = api.action_text_count(context);

  if (action_discrete_count > 0) {
    ASSERT_TRUE(api.action_discrete_bounds != nullptr);
  }

  if (action_continuous_count > 0) {
    ASSERT_TRUE(api.action_continuous_bounds != nullptr);
  }

  // Raw observations have their shape array invalidated, so we need to copy
  // that array.
  struct ObsSpec {
    EnvCApi_ObservationSpec spec;
    std::vector<int> shape;
  };

  const int observation_count = api.observation_count(context);

  std::vector<ObsSpec> obs_specs(observation_count);

  if (observation_count > 0) {
    ASSERT_TRUE(api.observation_name != nullptr);
    ASSERT_TRUE(api.observation_spec != nullptr);
    ASSERT_TRUE(api.observation != nullptr);
  }

  for (int i = 0; i != observation_count; ++i) {
    ASSERT_TRUE(api.observation_name(context, i) != nullptr);
    api.observation_spec(context, i, &obs_specs[i].spec);

    // For non-dynamic, non-string shapes, copy the shape array.
    if (obs_specs[i].spec.dims != -1 &&
        obs_specs[i].spec.type != EnvCApi_ObservationString) {
      obs_specs[i].shape =
          std::vector<int>(obs_specs[i].spec.shape,
                           obs_specs[i].spec.shape + obs_specs[i].spec.dims);
    }
  }

  if (api.event_count(context) > 0) {
    ASSERT_TRUE(api.event_type_name != nullptr);
    ASSERT_TRUE(api.event != nullptr);
  }

  for (int i = 0; i != api.event_count(context); ++i) {
    ASSERT_TRUE(api.event_type_name(context, i) != nullptr);
  }

  // Start the environment and verify observations and events.

  ASSERT_EQ(api.start(context, 0, 0), 0)
      << "API error: " << api.error_message(context);

  for (int i = 0; i != observation_count; ++i) {
    EnvCApi_Observation obs;
    const char* const obs_name = api.observation_name(context, i);
    ASSERT_TRUE(obs_name != nullptr) << "Observation missing name - index " << i
                                     << " (" << observation_count << ")";
    api.observation(context, i, &obs);

    EXPECT_EQ(obs_specs[i].spec.type, obs.spec.type);

    if (obs.spec.type == EnvCApi_ObservationString) {
      // string
      EXPECT_EQ(1, obs.spec.dims);
      EXPECT_LE(0, obs.spec.shape[0]);
    } else if (obs_specs[i].spec.dims == -1) {
      // dynamic shape
      EXPECT_LE(0, obs.spec.dims);
    } else {
      // TODO(tkoeppe, cbeattie): Our spec seems to be missing the case of
      // shape[k] = 0 meaning "dynamic".
      // EXPECT_THAT(obs_specs[i].dims,
      //            ElementsAreArray(obs.spec.shape, obs.spec.dims));
      // The following is a workaround:
      ASSERT_EQ(obs_specs[i].spec.dims, obs.spec.dims)
          << "Observation mismatch spec - index " << i << " ("
          << observation_count << ") has name: " << obs_name;
      for (int k = 0; k != obs.spec.dims; ++k) {
        if (obs_specs[i].shape[k] != 0) {
          EXPECT_EQ(obs_specs[i].shape[k], obs.spec.shape[k])
              << "Observation mismatch spec - index " << i << " ("
              << observation_count << ") has name: " << obs_name;
        }
      }
    }

    LOG(INFO) << "Observation " << i << " (" << observation_count
              << ") has name: " << obs_name;
  }

  for (int i = 0, e = api.event_count(context); i != e; ++i) {
    EnvCApi_Event event;
    const char* const env_name = api.event_type_name(context, i);
    ASSERT_TRUE(env_name != nullptr);
    api.event(context, i, &event);
    LOG(INFO) << "Event type " << i << " / " << e << " has name: " << env_name;
  }

  // Run a step.

  std::vector<int> discrete_actions(action_discrete_count);
  std::vector<double> continuous_actions(action_continuous_count);
  std::vector<EnvCApi_TextAction> text_actions(action_text_count, {"foo", 3});

  for (int i = 0; i != action_discrete_count; ++i) {
    int a, b;
    api.action_discrete_bounds(context, i, &a, &b);
    discrete_actions[i] = (a + b) / 2;
  }
  for (int i = 0; i != action_continuous_count; ++i) {
    double a, b;
    api.action_continuous_bounds(context, i, &a, &b);
    continuous_actions[i] = (a + b) / 2.0;
  }

  api.act_discrete(context, discrete_actions.data());
  api.act_continuous(context, continuous_actions.data());
  api.act_text(context, text_actions.data());

  double reward;
  EnvCApi_EnvironmentStatus status = api.advance(context, 1, &reward);
  LOG(INFO) << "Reward after one step: " << reward;
  LOG(INFO) << "Status after one step: " << status;

  // Cleanup.

  api.release_context(context);
}

}  // namespace
}  // namespace rl_api
