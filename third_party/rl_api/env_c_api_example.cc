// Copyright 2016-2017 Google Inc.
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

#include <cassert>
#include <cstdlib>
#include <functional>
#include <memory>
#include <string>

#include "third_party/rl_api/env_c_api.h"
#include "third_party/rl_api/env_c_api_bind.h"

namespace {

const int kDiscreteActionCount = 2;
const char* const kDiscreteActionNames[] = {"PADX", "PADY"};

const int kContinuousActionCount = 1;
const char* const kContinuousActionNames[] = {"TRIGGER"};

const struct {
  int min_value;
  int max_value;
} kDiscreteActionBounds[] = {{-1, 1}, {-1, 1}};

const struct {
  double min_value;
  double max_value;
} kContinuousActionBounds[] = {{-1.0, 1.0}};

const int kObservationCount = 3;
enum ObservationTypes {
  ObservationTypes_Bytes,
  ObservationTypes_Doubles,
  ObservationTypes_String
};

const char* const kObservationNames[] = {"BYTES", "DOUBLES", "STRING"};

const int kObservationBytesDim[] = {10, 2};
const int kObservationDoublesDim[] = {7};
const int kObservationStringDim[] = {0};

const EnvCApi_ObservationSpec kObservationSpecs[] = {
    {EnvCApi_ObservationBytes, 2, kObservationBytesDim},
    {EnvCApi_ObservationDoubles, 1, kObservationDoublesDim},
    {EnvCApi_ObservationString, 1, kObservationStringDim}};

class MyGame {
 public:
  int Setting(const char* key, const char* value) {
    // Doesn't accept any settings.
    return 1;
  }

  int Init() { return 0; }

  // Launch level using episode id and seed.
  int Start(int episode_id, int seed) {
    steps_ = 0;
    episode_id_ = episode_id;
    seed_ = seed;
    for (auto& byte_ref : observation_bytes_) {
      byte_ref = 127;
    }
    for (auto& double_ref : observation_doubles_) {
      double_ref = 127;
    }
    string_ = std::to_string(episode_id) + ":" + std::to_string(steps_);
    return 0;
  }

  const char* ErrorMessage() const { return "No error message."; }

  const char* EnvironmentName() const { return "example_environment"; }

  // The number of discrete actions.
  int ActionDiscreteCount() const { return kDiscreteActionCount; }

  // Discrete action name. 'discrete_idx' < action_discrete_count().
  const char* ActionDiscreteName(int discrete_idx) const {
    assert(discrete_idx < kDiscreteActionCount);
    return kDiscreteActionNames[discrete_idx];
  }

  // The discrete action inclusive range.
  void ActionDiscreteBounds(int discrete_idx, int* min_value,
                            int* max_value) const {
    *min_value = kDiscreteActionBounds[discrete_idx].min_value;
    *max_value = kDiscreteActionBounds[discrete_idx].max_value;
  }

  // The number of continuous actions.
  int ActionContinuousCount() const { return kContinuousActionCount; }

  // Continuous action name. 'continuous_idx' < action_continuous_count().
  const char* ActionContinuousName(int continuous_idx) const {
    assert(continuous_idx < kContinuousActionCount);
    return kContinuousActionNames[continuous_idx];
  }

  // The continuous action inclusive range.
  void ActionContinuousBounds(int continuous_idx, double* min_value_out,
                              double* max_value_out) const {
    *min_value_out = kContinuousActionBounds[continuous_idx].min_value;
    *max_value_out = kContinuousActionBounds[continuous_idx].max_value;
  }

  int ObservationCount() const { return kObservationCount; }
  const char* ObservationName(int observation_idx) {
    assert(observation_idx < kObservationCount);
    return kObservationNames[observation_idx];
  }

  // The shape of the observation parameter.
  void ObservationSpec(int observation_idx,
                       EnvCApi_ObservationSpec* spec) const {
    assert(observation_idx < kObservationCount);
    *spec = kObservationSpecs[observation_idx];
  }

  int EventTypeCount() const { return 0; }
  const char* EventTypeName(int event_type_idx) const { std::abort(); }

  int Fps() { return 60; }

  void Observation(int observation_idx, EnvCApi_Observation* observation) {
    ObservationSpec(observation_idx, &observation->spec);
    switch (observation_idx) {
      case ObservationTypes_Bytes:
        observation->payload.bytes = observation_bytes_;
        break;
      case ObservationTypes_Doubles:
        observation->payload.doubles = observation_doubles_;
        break;
      case ObservationTypes_String:
        string_shape_[0] = string_.size();
        observation->payload.string = string_.c_str();
        observation->spec.shape = string_shape_;
        break;
    }
  }

  int EventCount() { return 0; }

  void Event(int event_idx, EnvCApi_Event* event) {}

  void Act(const int actions_discrete[], const double actions_continuous[]) {}

  EnvCApi_EnvironmentStatus Advance(int num_steps, double* reward) {
    steps_ += num_steps;
    *reward = reward_;
    reward_ = 0;
    string_ = std::to_string(episode_id_) + ":" + std::to_string(steps_);
    return EnvCApi_EnvironmentStatus_Running;
  }

 private:
  int episode_id_;
  int seed_;
  int steps_;
  double reward_;
  unsigned char observation_bytes_[10 * 2];
  double observation_doubles_[7];
  int string_shape_[1];
  std::string string_;
};

}  // namespace

extern "C" int env_c_api_example_connect(EnvCApi* env_c_api, void** context) {
  auto game = std::unique_ptr<MyGame>(new MyGame());
  deepmind::rl_api::Bind(std::move(game), env_c_api, context);
  return 0;
}
