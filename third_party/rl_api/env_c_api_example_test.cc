// Copyright 2016-2019 Google Inc.
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

#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "third_party/rl_api/env_c_api.h"
#include "third_party/rl_api/env_c_api_example.h"

namespace {

constexpr int kDiscreteActionsCount = 2;
constexpr int kDiscreteActions[] = {0, 0};

constexpr int kContinuousActionsCount = 1;
constexpr double kContinuousActions[] = {0};

constexpr int kTextActionsCount = 1;
constexpr char const* kTextActions[] = {"Hello World!"};

using ::testing::ElementsAre;
using ::testing::Pair;

TEST(EnvCApiExampleTest, HasDiscreteActions) {
  EnvCApi env_c_api;
  void* context;

  env_c_api_example_connect(&env_c_api, &context);
  EXPECT_EQ(0, env_c_api.init(context));
  int discrete_count = env_c_api.action_discrete_count(context);
  EXPECT_EQ(kDiscreteActionsCount, discrete_count);
  for (int i = 0; i < discrete_count; ++i) {
    EXPECT_STRNE("", env_c_api.action_discrete_name(context, i));
  }

  env_c_api.release_context(context);
}

TEST(EnvCApiExampleTest, HasContinuousActions) {
  EnvCApi env_c_api;
  void* context;

  env_c_api_example_connect(&env_c_api, &context);
  EXPECT_EQ(0, env_c_api.init(context));
  int continuous_count = env_c_api.action_continuous_count(context);
  EXPECT_EQ(kContinuousActionsCount, continuous_count);
  for (int i = 0; i < continuous_count; ++i) {
    EXPECT_STRNE("", env_c_api.action_continuous_name(context, i));
  }

  env_c_api.release_context(context);
}

TEST(EnvCApiExampleTest, HasTextActions) {
  EnvCApi env_c_api;
  void* context;

  env_c_api_example_connect(&env_c_api, &context);
  EXPECT_EQ(0, env_c_api.init(context));
  int text_count = env_c_api.action_text_count(context);
  EXPECT_EQ(kTextActionsCount, text_count);
  for (int i = 0; i < text_count; ++i) {
    EXPECT_STRNE("", env_c_api.action_text_name(context, i));
  }

  env_c_api.release_context(context);
}

TEST(EnvCApiExampleTest, HasObservations) {
  EnvCApi env_c_api;
  void* context;
  env_c_api_example_connect(&env_c_api, &context);
  EXPECT_EQ(0, env_c_api.init(context));
  int observation_count = env_c_api.observation_count(context);
  EXPECT_GT(observation_count, 0);
  for (int idx = 0; idx < observation_count; ++idx) {
    EXPECT_STRNE("", env_c_api.observation_name(context, idx));
  }

  env_c_api.release_context(context);
}

TEST(EnvCApiExampleTest, ListProperty) {
  EnvCApi env_c_api;
  void* context;
  env_c_api_example_connect(&env_c_api, &context);
  EXPECT_EQ(env_c_api.setting(context, "prop0", "0"), 0);
  EXPECT_EQ(env_c_api.setting(context, "prop1", "1"), 0);
  EXPECT_EQ(env_c_api.setting(context, "prop2_writeonly", "2"), 0);
  EXPECT_EQ(env_c_api.setting(context, "prop3_readonly", "3"), 0);
  EXPECT_EQ(env_c_api.init(context), 0);

  using PropAttributeMap = std::map<std::string, EnvCApi_PropertyAttributes>;
  PropAttributeMap properties;
  auto list_result = env_c_api.list_property(
      context, &properties, /*list_key=*/"",
      [](void* userdata, const char* key,
         EnvCApi_PropertyAttributes attributes) {
        auto* properties_ptr = static_cast<PropAttributeMap*>(userdata);
        (*properties_ptr)[key] = attributes;
      });
  EXPECT_EQ(list_result, EnvCApi_PropertyResult_Success);
  EXPECT_THAT(
      properties,
      ElementsAre(Pair("prop0", EnvCApi_PropertyAttributes_ReadWritable),
                  Pair("prop1", EnvCApi_PropertyAttributes_ReadWritable),
                  Pair("prop2_writeonly", EnvCApi_PropertyAttributes_Writable),
                  Pair("prop3_readonly", EnvCApi_PropertyAttributes_Readable)));
  env_c_api.release_context(context);
}

TEST(EnvCApiExampleTest, ListPropertyFail) {
  EnvCApi env_c_api;
  void* context;
  env_c_api_example_connect(&env_c_api, &context);
  EXPECT_EQ(env_c_api.setting(context, "prop0", "0"), 0);
  EXPECT_EQ(env_c_api.init(context), 0);

  auto list_result_not_listable =
      env_c_api.list_property(context, nullptr, /*list_key=*/"prop0",
                                  [](void* userdata, const char* key,
                                     EnvCApi_PropertyAttributes attributes) {});
  EXPECT_EQ(list_result_not_listable, EnvCApi_PropertyResult_PermissionDenied);

  auto list_result_not_found =
      env_c_api.list_property(context, nullptr, /*list_key=*/"Missing",
                                  [](void* userdata, const char* key,
                                     EnvCApi_PropertyAttributes attributes) {});
  EXPECT_EQ(list_result_not_found, EnvCApi_PropertyResult_NotFound);
  env_c_api.release_context(context);
}

TEST(EnvCApiExampleTest, ReadProperty) {
  EnvCApi env_c_api;
  void* context;
  env_c_api_example_connect(&env_c_api, &context);
  EXPECT_EQ(env_c_api.setting(context, "prop0", "0"), 0);
  EXPECT_EQ(env_c_api.setting(context, "prop1", "1"), 0);
  EXPECT_EQ(env_c_api.setting(context, "prop2_writeonly", "2"), 0);
  EXPECT_EQ(env_c_api.setting(context, "prop3_readonly", "3"), 0);
  EXPECT_EQ(env_c_api.init(context), 0);
  const char* value = nullptr;
  EXPECT_EQ(env_c_api.read_property(context, "prop0", &value),
            EnvCApi_PropertyResult_Success);
  EXPECT_STREQ(value, "0");
  EXPECT_EQ(env_c_api.read_property(context, "prop1", &value),
            EnvCApi_PropertyResult_Success);
  EXPECT_STREQ(value, "1");
  EXPECT_EQ(env_c_api.read_property(context, "prop3_readonly", &value),
            EnvCApi_PropertyResult_Success);
  EXPECT_STREQ(value, "3");
  env_c_api.release_context(context);
}

TEST(EnvCApiExampleTest, ReadPropertyFail) {
  EnvCApi env_c_api;
  void* context;
  env_c_api_example_connect(&env_c_api, &context);
  EXPECT_EQ(env_c_api.setting(context, "prop0", "0"), 0);
  EXPECT_EQ(env_c_api.setting(context, "prop1", "1"), 0);
  EXPECT_EQ(env_c_api.setting(context, "prop2_writeonly", "2"), 0);
  EXPECT_EQ(env_c_api.setting(context, "prop3_readonly", "3"), 0);
  EXPECT_EQ(env_c_api.init(context), 0);
  const char* value = nullptr;
  EXPECT_EQ(env_c_api.read_property(context, "prop", &value),
            EnvCApi_PropertyResult_NotFound);
  EXPECT_EQ(value, nullptr);
  EXPECT_EQ(env_c_api.read_property(context, "prop2_writeonly", &value),
            EnvCApi_PropertyResult_PermissionDenied);
  EXPECT_EQ(value, nullptr);
  env_c_api.release_context(context);
}

TEST(EnvCApiExampleTest, WriteProperty) {
  EnvCApi env_c_api;
  void* context;
  env_c_api_example_connect(&env_c_api, &context);
  EXPECT_EQ(env_c_api.setting(context, "prop0", "0"), 0);
  EXPECT_EQ(env_c_api.setting(context, "prop1", "1"), 0);
  EXPECT_EQ(env_c_api.setting(context, "prop2_writeonly", "2"), 0);
  EXPECT_EQ(env_c_api.setting(context, "prop3_readonly", "3"), 0);
  EXPECT_EQ(env_c_api.init(context), 0);

  const char* value = nullptr;
  EXPECT_EQ(env_c_api.read_property(context, "prop0", &value),
            EnvCApi_PropertyResult_Success);
  EXPECT_STREQ(value, "0");
  EXPECT_EQ(env_c_api.write_property(context, "prop0", "NewValue0"),
            EnvCApi_PropertyResult_Success);
  EXPECT_EQ(env_c_api.read_property(context, "prop0", &value),
            EnvCApi_PropertyResult_Success);
  EXPECT_STREQ(value, "NewValue0");
  EXPECT_EQ(
      env_c_api.write_property(context, "prop2_writeonly", "NewValue2"),
      EnvCApi_PropertyResult_Success);
  env_c_api.release_context(context);
}


TEST(EnvCApiExampleTest, WritePropertyFail) {
  EnvCApi env_c_api;
  void* context;
  env_c_api_example_connect(&env_c_api, &context);
  EXPECT_EQ(env_c_api.setting(context, "prop0", "0"), 0);
  EXPECT_EQ(env_c_api.setting(context, "prop1", "1"), 0);
  EXPECT_EQ(env_c_api.setting(context, "prop2_writeonly", "2"), 0);
  EXPECT_EQ(env_c_api.setting(context, "prop3_readonly", "3"), 0);
  EXPECT_EQ(env_c_api.init(context), 0);
  EXPECT_EQ(env_c_api.write_property(context, "prop", "NewValue"),
            EnvCApi_PropertyResult_NotFound);

  const char* value = nullptr;
  EXPECT_EQ(env_c_api.read_property(context, "prop3_readonly", &value),
            EnvCApi_PropertyResult_Success);
  EXPECT_STREQ(value, "3");
  EXPECT_EQ(
      env_c_api.write_property(context, "prop3_readonly", "NewValue3"),
      EnvCApi_PropertyResult_PermissionDenied);
  value = nullptr;
  EXPECT_EQ(env_c_api.read_property(context, "prop3_readonly", &value),
            EnvCApi_PropertyResult_Success);
  EXPECT_STREQ(value, "3");
  env_c_api.release_context(context);
}

TEST(EnvCApiExampleTest, ObservationString) {
  EnvCApi env_c_api;
  void* context;
  env_c_api_example_connect(&env_c_api, &context);
  EXPECT_EQ(0, env_c_api.init(context));
  int observation_count = env_c_api.observation_count(context);
  EXPECT_GT(observation_count, 0);
  std::vector<int> string_ids;
  for (int idx = 0; idx < observation_count; ++idx) {
    EnvCApi_ObservationSpec spec;
    env_c_api.observation_spec(context, idx, &spec);
    if (spec.type == EnvCApi_ObservationString) {
      string_ids.push_back(idx);
      ASSERT_EQ(1, spec.dims);
      EXPECT_EQ(0, spec.shape[0]);
    }
  }

  env_c_api.start(context, /*episode=*/1, /*seed=*/1234);

  ASSERT_EQ(1, string_ids.size());

  int string_id = string_ids.front();
  {
    EnvCApi_Observation obs;
    env_c_api.observation(context, string_id, &obs);
    EXPECT_EQ(EnvCApi_ObservationString, obs.spec.type);
    ASSERT_EQ(1, obs.spec.dims);
    EXPECT_NE(0, obs.spec.shape[0]);
    EXPECT_EQ("1:0", std::string(obs.payload.string, obs.spec.shape[0]));
  }

  double reward;
  EXPECT_EQ(EnvCApi_EnvironmentStatus_Running,
            env_c_api.advance(context, /*num_steps=*/5, &reward));
  EXPECT_EQ(0.0, reward);

  {
    EnvCApi_Observation obs;
    env_c_api.observation(context, string_id, &obs);
    EXPECT_EQ(EnvCApi_ObservationString, obs.spec.type);
    ASSERT_EQ(1, obs.spec.dims);
    EXPECT_NE(0, obs.spec.shape[0]);
    EXPECT_EQ("1:5", std::string(obs.payload.string, obs.spec.shape[0]));
  }

  env_c_api.release_context(context);
}

TEST(EnvCApiExampleTest, ObservationDoubles) {
  EnvCApi env_c_api;
  void* context;
  env_c_api_example_connect(&env_c_api, &context);
  EXPECT_EQ(0, env_c_api.init(context));
  int observation_count = env_c_api.observation_count(context);
  EXPECT_GT(observation_count, 0);
  std::vector<int> doubles_ids;
  for (int idx = 0; idx < observation_count; ++idx) {
    EnvCApi_ObservationSpec spec;
    env_c_api.observation_spec(context, idx, &spec);
    if (spec.type == EnvCApi_ObservationDoubles) {
      doubles_ids.push_back(idx);
      ASSERT_EQ(1, spec.dims);
      EXPECT_EQ(7, spec.shape[0]);
    }
  }

  env_c_api.start(context, /*episode=*/1, /*seed=*/1234);
  ASSERT_EQ(1, doubles_ids.size());

  int doubles_id = doubles_ids.front();
  {
    EnvCApi_Observation obs;
    env_c_api.observation(context, doubles_id, &obs);
    EXPECT_EQ(EnvCApi_ObservationDoubles, obs.spec.type);
    ASSERT_EQ(1, obs.spec.dims);
    EXPECT_EQ(7, obs.spec.shape[0]);
    std::vector<double> double_observation(
        obs.payload.doubles, obs.payload.doubles + obs.spec.shape[0]);

    for (double v : double_observation) {
      EXPECT_EQ(127.0, v);
    }
  }
  EnvCApi_TextAction textAction{ kTextActions[0], strlen(kTextActions[0])};
  env_c_api.act_discrete(context, kDiscreteActions);
  env_c_api.act_continuous(context, kContinuousActions);
  env_c_api.act_text(context, &textAction);
  double reward;
  EXPECT_EQ(EnvCApi_EnvironmentStatus_Running,
            env_c_api.advance(context, /*num_steps=*/5, &reward));
  EXPECT_EQ(0.0, reward);
  {
    EnvCApi_Observation obs;
    env_c_api.observation(context, doubles_id, &obs);
    EXPECT_EQ(EnvCApi_ObservationDoubles, obs.spec.type);
    ASSERT_EQ(1, obs.spec.dims);
    EXPECT_EQ(7, obs.spec.shape[0]);
    std::vector<double> double_observation(
        obs.payload.doubles, obs.payload.doubles + obs.spec.shape[0]);

    for (double v : double_observation) {
      EXPECT_EQ(127.0, v);
    }
  }

  env_c_api.release_context(context);
}

TEST(EnvCApiExampleTest, ObservationBytes) {
  EnvCApi env_c_api;
  void* context;
  env_c_api_example_connect(&env_c_api, &context);
  EXPECT_EQ(0, env_c_api.init(context));
  int observation_count = env_c_api.observation_count(context);
  EXPECT_GT(observation_count, 0);
  std::vector<int> bytes_ids;
  for (int idx = 0; idx < observation_count; ++idx) {
    EnvCApi_ObservationSpec spec;
    env_c_api.observation_spec(context, idx, &spec);
    if (spec.type == EnvCApi_ObservationBytes) {
      bytes_ids.push_back(idx);
      ASSERT_EQ(2, spec.dims);
      EXPECT_EQ(10, spec.shape[0]);
      EXPECT_EQ(2, spec.shape[1]);
    }
  }

  env_c_api.start(context, /*episode=*/1, /*seed=*/1234);
  ASSERT_EQ(1, bytes_ids.size());

  int bytes_id = bytes_ids.front();
  {
    EnvCApi_Observation obs;
    env_c_api.observation(context, bytes_id, &obs);
    EXPECT_EQ(EnvCApi_ObservationBytes, obs.spec.type);
    ASSERT_EQ(2, obs.spec.dims);
    EXPECT_EQ(10, obs.spec.shape[0]);
    EXPECT_EQ(2, obs.spec.shape[1]);
    std::vector<unsigned char> byte_observation(
        obs.payload.bytes,
        obs.payload.bytes + obs.spec.shape[0] * obs.spec.shape[1]);

    for (unsigned char v : byte_observation) {
      EXPECT_EQ(127, v);
    }
  }
  env_c_api.act_discrete(context, kDiscreteActions);
  env_c_api.act_continuous(context, kContinuousActions);
  double reward;
  EXPECT_EQ(EnvCApi_EnvironmentStatus_Running,
            env_c_api.advance(context, /*num_steps=*/5, &reward));
  EXPECT_EQ(0.0, reward);
  {
    EnvCApi_Observation obs;
    env_c_api.observation(context, bytes_id, &obs);
    EXPECT_EQ(EnvCApi_ObservationBytes, obs.spec.type);
    ASSERT_EQ(2, obs.spec.dims);
    EXPECT_EQ(10, obs.spec.shape[0]);
    EXPECT_EQ(2, obs.spec.shape[1]);
    std::vector<unsigned char> byte_observation(
        obs.payload.bytes,
        obs.payload.bytes + obs.spec.shape[0] * obs.spec.shape[1]);

    for (unsigned char v : byte_observation) {
      EXPECT_EQ(127, v);
    }
  }

  env_c_api.release_context(context);
}

}  // namespace
