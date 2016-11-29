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

#include <cassert>
#include <cstdlib>
#include <functional>

#include "third_party/rl_api/env_c_api.h"

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

const int kObservationCount = 2;
enum ObservationTypes { ObservationTypes_Bytes, ObservationTypes_Doubles };
const char* const kObservationNames[] = {"BYTES", "DOUBLES"};

const int kObservationBytesDim[] = {10, 2};
const int kObservationDoublesDim[] = {7};

const EnvCApi_ObservationSpec kObservationSpecs[] = {
    {EnvCApi_ObservationBytes, 2, kObservationBytesDim},
    {EnvCApi_ObservationDoubles, 1, kObservationDoublesDim}};

class MyGame {
 public:
  int setting(const char* key, const char* value) {
    // Doesn't accept any settings.
    return 1;
  }

  int init() { return 0; }

  // Launch level using episode id and seed.
  int start(int episode_id, int seed) {
    episode_id_ = episode_id;
    seed_ = seed;
    for (auto& byte_ref : observation_bytes_) {
      byte_ref = 127;
    }
    for (auto& double_ref : observation_doubles_) {
      double_ref = 127;
    }
    return 0;
  }

  const char* environment_name() const { return "example_environment"; }

  // The number of discrete actions.
  int action_discrete_count() const { return kDiscreteActionCount; }

  // Discrete action name. 'discrete_idx' < action_discrete_count().
  const char* action_discrete_name(int discrete_idx) const {
    assert(discrete_idx < kDiscreteActionCount);
    return kDiscreteActionNames[discrete_idx];
  }

  // The discrete action inclusive range.
  void action_discrete_bounds(int discrete_idx, int* min_value,
                              int* max_value) const {
    *min_value = kDiscreteActionBounds[discrete_idx].min_value;
    *max_value = kDiscreteActionBounds[discrete_idx].max_value;
  }

  // The number of continuous actions.
  int action_continuous_count() const { return kContinuousActionCount; }

  // Continuous action name. 'continuous_idx' < action_continuous_count().
  const char* action_continuous_name(int continuous_idx) const {
    assert(continuous_idx < kContinuousActionCount);
    return kContinuousActionNames[continuous_idx];
  }

  // The continuous action inclusive range.
  void action_continuous_bounds(int continuous_idx, double* min_value_out,
                                double* max_value_out) const {
    *min_value_out = kContinuousActionBounds[continuous_idx].min_value;
    *max_value_out = kContinuousActionBounds[continuous_idx].max_value;
  }

  int observation_count() const { return kObservationCount; }
  const char* observation_name(int observation_idx) {
    assert(observation_idx < kObservationCount);
    return kObservationNames[observation_idx];
  }

  // The shape of the observation parameter.
  void observation_spec(int observation_idx,
                        EnvCApi_ObservationSpec* spec) const {
    assert(observation_idx < kObservationCount);
    *spec = kObservationSpecs[observation_idx];
  }

  int event_type_count() const { return 0; }
  const char* event_type_name(int event_type_idx) const { std::abort(); }

  int fps() { return 60; }

  void observation(int observation_idx, EnvCApi_Observation* observation) {
    observation_spec(observation_idx, &observation->spec);
    switch (observation_idx) {
      case ObservationTypes_Bytes:
        observation->payload.bytes = observation_bytes_;
        break;
      case ObservationTypes_Doubles:
        observation->payload.doubles = observation_doubles_;
        break;
    }
  }

  int event_count() { return 0; }

  void event(int event_idx, EnvCApi_Event* event) {}

  void act(const int actions_discrete[], const double actions_continuous[]) {}

  EnvCApi_EnvironmentStatus advance(int num_steps, double* reward) {
    steps_ += num_steps;
    *reward = reward_;
    reward_ = 0;
    return EnvCApi_EnvironmentStatus_Running;
  }

 private:
  int episode_id_;
  int seed_;
  int steps_;
  double reward_;
  unsigned char observation_bytes_[10 * 2];
  double observation_doubles_[7];
};

// Memory cleanup.
static void release_context(void* context) {
  delete static_cast<MyGame*>(context);
}

template <typename T, typename Ret, typename... Args>
struct CtxBinder {
  template <Ret (T::*MemFunc)(Args...)>
  static Ret Func(void* context, Args... args) {
    assert(context != nullptr);
    return (static_cast<T*>(context)->*MemFunc)(args...);
  }

  template <Ret (T::*MemFunc)(Args...) const>
  static Ret Func(void* context, Args... args) {
    assert(context != nullptr);
    return (static_cast<T*>(context)->*MemFunc)(args...);
  }
};

template <typename T, typename Ret, typename... Args>
CtxBinder<T, Ret, Args...> CtxBind(Ret (T::*MemFunc)(Args...));

template <typename T, typename Ret, typename... Args>
CtxBinder<T, Ret, Args...> CtxBind(Ret (T::*MemFunc)(Args...) const);

}  // namespace

// An example for isolating global symbols: This translation unit may be linked
// in such a way that global symbols like the following two do not clash with
// symbols in the client application. This kind of isolation is valuable when
// the environment is built from code that was not designed with reusablity in
// mind.
extern "C" {
int odr_example;
int violate_odr_maybe() { return ++odr_example; }
}

// Portability note. This has not got C-language linkage but seems to work.
#define BIND_C(class_member) \
  decltype(CtxBind(&class_member))::Func<&class_member>

extern "C" int env_c_api_example_connect(EnvCApi* env_c_api, void** context) {
  violate_odr_maybe();

  *context = new MyGame();
  env_c_api->setting = BIND_C(MyGame::setting);
  env_c_api->init = BIND_C(MyGame::init);
  env_c_api->start = BIND_C(MyGame::start);
  env_c_api->environment_name = BIND_C(MyGame::environment_name);
  env_c_api->action_discrete_count = BIND_C(MyGame::action_discrete_count);
  env_c_api->action_discrete_name = BIND_C(MyGame::action_discrete_name);
  env_c_api->action_discrete_bounds = BIND_C(MyGame::action_discrete_bounds);
  env_c_api->action_continuous_count = BIND_C(MyGame::action_continuous_count);
  env_c_api->action_continuous_name = BIND_C(MyGame::action_continuous_name);
  env_c_api->action_continuous_bounds =
      BIND_C(MyGame::action_continuous_bounds);
  env_c_api->observation_count = BIND_C(MyGame::observation_count);
  env_c_api->observation_name = BIND_C(MyGame::observation_name);
  env_c_api->observation_spec = BIND_C(MyGame::observation_spec);
  env_c_api->event_type_count = BIND_C(MyGame::event_type_count);
  env_c_api->event_type_name = BIND_C(MyGame::event_type_name);
  env_c_api->fps = BIND_C(MyGame::fps);
  env_c_api->observation = BIND_C(MyGame::observation);
  env_c_api->event_count = BIND_C(MyGame::event_count);
  env_c_api->event = BIND_C(MyGame::event);
  env_c_api->act = BIND_C(MyGame::act);
  env_c_api->advance = BIND_C(MyGame::advance);
  env_c_api->release_context = release_context;
  return 0;
}
#undef BIND_C
