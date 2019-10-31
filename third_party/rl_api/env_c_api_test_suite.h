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
//
// A conformance test library for EnvCApi.
//
// This library exposes a test suite for implementations of EnvCApi. The test
// suite is parameterized on an EvCApiTestParam (see below), which contains a
// function to instantiate the environment and a list of settings to be passed
// to it.
//
// Usage example:
//
// my_env_test.cc:
//
//   #include "my/project/my_env.h"
//   #include "third_party/rl_api/env_c_api.h"
//   #include "third_party/rl_api/env_c_api_test_suite.h"
//
//   namespace myproject {
//   namespace {
//
//   using ::rl_api::EnvCApiConformanceTestSuite;
//
//   int test_connect(EnvCApi* api, void** context) { /* ... */ }
//
//   INSTANTIATE_TEST_SUITE_P(
//       MyEnvConformanceTest, EnvCApiConformanceTestSuite,
//       testing::Values(EnvCApiTestParam{
//          test_connect, {{"key1", "value1"}, {"key2", "value2"}}}));
//
//   }  // namespace
//   }  // namespace myproject

#ifndef DEEPMIND_ENV_C_API_TEST_SUITE_H_
#define DEEPMIND_ENV_C_API_TEST_SUITE_H_

#include <functional>
#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "third_party/rl_api/env_c_api.h"

namespace rl_api {

struct EnvCApiTestParam {
  struct Setting { std::string key, value; };

  std::function<int(EnvCApi*, void**)> connect_fn;
  std::vector<Setting> settings;
};

using EnvCApiConformanceTestSuite = ::testing::TestWithParam<EnvCApiTestParam>;

}  // namespace rl_api

#endif  // DEEPMIND_ENV_C_API_TEST_SUITE_H_
