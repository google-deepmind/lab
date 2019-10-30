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

#ifndef THIRD_PARTY_RL_API_ENV_C_API_BIND_H_
#define THIRD_PARTY_RL_API_ENV_C_API_BIND_H_

#include <memory>

#include "third_party/rl_api/env_c_api.h"

namespace deepmind {
namespace rl_api {

// Binds all function pointers in `api` with the equivalent "CamelCased" member
// functions in `env`, with the exception of `release_context` (see below).

// Example:
//
//    EnvCApi api;
//    void* context;
//    std::unique_ptr<MyClass> env(new MyClass);
//    deepmind::rl_api::Bind(std::move(env), &env_c_api, &context);
//
// Will set the function pointers in `api`:
//
//     int (*setting) (void* context, const char* key, const char* value);
//     const char* (*environment_name) (void* context);
//     // etc.
//
// To functions equivalent to:
//
//     static int setting(void* context, const char* key, const char* value) {
//        return static_cast<MyClass*>(context)->Setting(key, value);
//     }
//
//     static const char* environment_name(void* context) {
//        return static_cast<MyClass*>(context)->EnvironmentName();
//     }
//     // etc.
//
// Portability note. The instantiated functions do not use C-language linkage
// and may not work.
//
// The `context` now owns the environment, and the environment must be released
// via api->release_context(`context`). This does not call MyClass::Release and
// instead deletes the `context`.
template <class T>
void Bind(std::unique_ptr<T> env, EnvCApi* api, void** context);

namespace internal {

template <typename T, typename Ret, typename... Args>
struct CtxBinder {
  template <Ret (T::*MemFunc)(Args...)>
  static Ret Func(void* context, Args... args) {
    return (static_cast<T*>(context)->*MemFunc)(args...);
  }

  template <Ret (T::*MemFunc)(Args...) const>
  static Ret Func(void* context, Args... args) {
    return (static_cast<T*>(context)->*MemFunc)(args...);
  }
};

template <typename T, typename Ret, typename... Args>
CtxBinder<T, Ret, Args...> CtxBind(Ret (T::*MemFunc)(Args...));

template <typename T, typename Ret, typename... Args>
CtxBinder<T, Ret, Args...> CtxBind(Ret (T::*MemFunc)(Args...) const);

}  // namespace internal

#define DEEPMIND_RL_API_BIND(member) \
  decltype(internal::CtxBind(&T::member))::template Func<&T::member>

template <class T>
void Bind(std::unique_ptr<T> env, EnvCApi* api, void** context) {
  *context = env.release();
  api->setting = DEEPMIND_RL_API_BIND(Setting);
  api->init = DEEPMIND_RL_API_BIND(Init);
  api->start = DEEPMIND_RL_API_BIND(Start);
  api->error_message = DEEPMIND_RL_API_BIND(ErrorMessage);
  api->read_property = DEEPMIND_RL_API_BIND(ReadProperty);
  api->write_property = DEEPMIND_RL_API_BIND(WriteProperty);
  api->list_property = DEEPMIND_RL_API_BIND(ListProperty);
  api->environment_name = DEEPMIND_RL_API_BIND(EnvironmentName);
  api->action_discrete_count = DEEPMIND_RL_API_BIND(ActionDiscreteCount);
  api->action_discrete_name = DEEPMIND_RL_API_BIND(ActionDiscreteName);
  api->action_discrete_bounds = DEEPMIND_RL_API_BIND(ActionDiscreteBounds);
  api->action_continuous_count = DEEPMIND_RL_API_BIND(ActionContinuousCount);
  api->action_continuous_name = DEEPMIND_RL_API_BIND(ActionContinuousName);
  api->action_continuous_bounds = DEEPMIND_RL_API_BIND(ActionContinuousBounds);
  api->action_text_count = DEEPMIND_RL_API_BIND(ActionTextCount);
  api->action_text_name = DEEPMIND_RL_API_BIND(ActionTextName);
  api->observation_count = DEEPMIND_RL_API_BIND(ObservationCount);
  api->observation_name = DEEPMIND_RL_API_BIND(ObservationName);
  api->observation_spec = DEEPMIND_RL_API_BIND(ObservationSpec);
  api->event_type_count = DEEPMIND_RL_API_BIND(EventTypeCount);
  api->event_type_name = DEEPMIND_RL_API_BIND(EventTypeName);
  api->observation = DEEPMIND_RL_API_BIND(Observation);
  api->event_count = DEEPMIND_RL_API_BIND(EventCount);
  api->event = DEEPMIND_RL_API_BIND(Event);
  api->act_discrete = DEEPMIND_RL_API_BIND(ActDiscrete);
  api->act_continuous = DEEPMIND_RL_API_BIND(ActContinuous);
  api->act_text = DEEPMIND_RL_API_BIND(ActText);
  api->advance = DEEPMIND_RL_API_BIND(Advance);
  api->release_context = [](void* context) { delete static_cast<T*>(context); };
}
#undef DEEPMIND_RL_API_BIND

}  // namespace rl_api
}  // namespace deepmind

#endif  // THIRD_PARTY_RL_API_ENV_C_API_BIND_H_
