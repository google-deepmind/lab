// Copyright 2016 Google Inc.
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

#include "gtest/gtest.h"
#include "third_party/rl_api/env_c_api.h"
#include "third_party/rl_api/env_c_api_example.h"

TEST(EnvCApiExampleTest, HasDiscreteCount) {
  EnvCApi env_c_api;
  void* context;

  env_c_api_example_connect(&env_c_api, &context);
  env_c_api.init(context);
  int discrete_count = env_c_api.action_discrete_count(context);
  EXPECT_GT(discrete_count, 0);
  for (int i = 0; i < discrete_count; ++i) {
    std::string name = env_c_api.action_discrete_name(context, i);
    EXPECT_FALSE(name.empty());
  }

  env_c_api.release_context(context);
}

extern "C" {
  int odr_example;
  void violate_odr_maybe() { --odr_example; }
}

TEST(EnvCApiExampleTest, ODR) {
  // This test demonstrates that we can use global symbols without regard to
  // the global symbols used by the library that provides the RL environment.
  violate_odr_maybe();
}
