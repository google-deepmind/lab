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
//
// An example use of the environment API:
//
// A function, typically called "connect", which populates a given EnvCApi
// struct and an opaque "context" (communicated as a void pointer).
//
// Example:
//
//    EnvCApi api;
//    void* context;
//
//    env_c_api_example_connect(&api, &context);

#ifndef DEEPMIND_ENV_C_API_EXAMPLE_H_
#define DEEPMIND_ENV_C_API_EXAMPLE_H_

#include "third_party/rl_api/env_c_api.h"

extern "C" int env_c_api_example_connect(EnvCApi* env_c_api, void** context);

#endif  // DEEPMIND_ENV_C_API_EXAMPLE_H_
