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
// A C-language RL environment interface.
//
// An environment exposes this API by returning an opaque context and filling in
// the function pointer fields of an EnvCApi object. Users can then interact
// with the environment by calling those function pointers and passing the
// context along.
//
// The functions pointed to by the EnvCApi fields shall satisfy the requirements
// that are described below.
//
// The file env_c_api_example.cc shows an example environment and bindings to
// this API.
//
//
// RL Environment Basics
// ---------------------
//
// An *environment* in the sense of this API is a state which evolves in
// discrete steps. At every step, an external *agent* may perform *actions*
// on the environment. The agent may also request a set of *observations*
// from the environment at any point, and query for *events*.
//
// An action consists of a tuple of discrete (integral) values and a tuple
// of continuous (floating-point) values. The sizes of both these tuples
// makes up the "shape" of the action, which can be queried through the API.
//
// An observation consists of a multidimensional array of either ints or
// doubles. This "shape" of the observation is described by an "observation
// spec", which is part of the observation. (However, the API may additionally
// expose a fixed spec that is always valid, which simplifies observation
// handling for the caller.) The environment returns a tuple of observations,
// whose size can be queried through the API.
//
// An event is an optional piece of data (see below).

#ifndef DEEPMIND_ENV_C_API_H_
#define DEEPMIND_ENV_C_API_H_

#define DEEPMIND_ENV_C_API_VERSION_MAJOR 1
#define DEEPMIND_ENV_C_API_VERSION_MINOR 0

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// The main API data structure.
typedef struct EnvCApi_s EnvCApi;

// Shape and type of an observation.
typedef struct EnvCApi_ObservationSpec_s EnvCApi_ObservationSpec;

// An observation. Contains a payload and observation spec that describes its
// shape.
typedef struct EnvCApi_Observation_s EnvCApi_Observation;

// An event. Contains identifying information as well as associated
// observations. (These observations are unrelated to the environment
// observations.)
typedef struct EnvCApi_Event_s EnvCApi_Event;

// The status of an environment. This status changes as the environment evolves.
// If the status is not 'EnvCApi_EnvironmentStatus_Running' or
// 'EnvCApi_EnvironmentStatus_Interrupted' the environment has to be reset
// before further use, or destroyed.
//
// If the status is 'EnvCApi_EnvironmentStatus_Interrupted' 'act' and 'advance'
// can no longer be called.
// See below for details.
enum EnvCApi_EnvironmentStatus_enum {
  EnvCApi_EnvironmentStatus_Running,
  EnvCApi_EnvironmentStatus_Interrupted,
  EnvCApi_EnvironmentStatus_Error,
  EnvCApi_EnvironmentStatus_Terminated
};
typedef enum EnvCApi_EnvironmentStatus_enum EnvCApi_EnvironmentStatus;

// The fundamental data type of an observation; part of the observation spec.
enum EnvCApi_ObservationType_enum {
  EnvCApi_ObservationDoubles,
  EnvCApi_ObservationBytes
};
typedef enum EnvCApi_ObservationType_enum EnvCApi_ObservationType;

// An observation is a multidimensional array of numbers. The data type of the
// numbers is described by "type" (int or double). The data is serialized as a
// flat array of size shape[0] * shape[1] * ... * shape[dims - 1], laid out in
// strides in row-major (C-style) order.
struct EnvCApi_ObservationSpec_s {
  EnvCApi_ObservationType type;
  int dims;
  const int* shape;
};

// The data in payload is interpreted according to spec; see above.
struct EnvCApi_Observation_s {
  EnvCApi_ObservationSpec spec;
  union {
    const double* doubles;
    const unsigned char* bytes;
  } payload;
};

// An event consists of an opaque ID, whose meaning can be queried through the
// API, as well as a (possibly empty) set of observations.
struct EnvCApi_Event_s {
  int id;
  int observation_count;
  const EnvCApi_Observation* observations;
};

///////////////
//  The API  //
///////////////

// General remarks
//
// Terminology: the phrase "any other API call" is a shorthand for "any call
// of any other function in this concrete API object with the same context".
//
// Several functions produce pointer values that become invalidated by "any
// other API call". (This allows implementations to use a fixed storage for
// those values.)
//
// All functions shall be called with a valid context, that is, a context that
// was created successfully and has not been passed to release_context. (This
// API does not specify how contexts are created.)
//
// All functions that take pointer parameters shall be called with corresponding
// valid and non-null arguments. Functions that return pointers to const char
// shall not return null.
//
// Returned pointers never convey ownership; the implementation manages the
// resources of all exposed objects.

struct EnvCApi_s {
  // Setup, start and termination
  ///////////////////////////////

  // Applies a setting to the environment. Can be called multiple times. If the
  // same key is used more than once, the last key-value pair is used. This
  // function shall only be called before init, and it shall return zero
  // on success and non-zero on failure.
  int (*setting)(void* context, const char* key, const char* value);

  // Shall be called after all calls to setting and before init. It shall return
  // zero on success and non-zero on failure. Most functions from this API shall
  // only be called after a successful call of init; see their individual
  // documentation.
  int (*init)(void* context);

  // Launches an episode using 'episode_id' and 'seed'. Returns zero on success
  // and non-zero on failure. Some functions from this API shall only be called
  // after a successful call of start; see their individual documentation.
  // This function shall only be called after a successful call of init.
  int (*start)(void* context, int episode_id, int seed);

  // Releases the resources associated with 'context', which shall not be used
  // again after this call. (Note that this API does not specify how a context
  // is created, only how it is destroyed.)
  void (*release_context)(void* context);

  // Meta data querying
  /////////////////////
  //
  // Functions in this section shall only be called after a successful call of
  // init, but beyond that they may be called at any time, regardless of whether
  // start has been called or whether the episode has terminated.

  // Name of the environment.
  const char* (*environment_name)(void* context);

  // The number of discrete actions.
  int (*action_discrete_count)(void* context);

  // The number of continuous actions.
  int (*action_continuous_count)(void* context);

  // Retrieves the name associated with a discrete or continuous action.
  //
  // 'discrete_idx' shall be in the range [0, action_discrete_count()).
  // 'continuous_idx' shall be in the range [0, action_continuous_count()).
  const char* (*action_discrete_name)(void* context, int discrete_idx);
  const char* (*action_continuous_name)(void* context, int continuous_idx);

  // The range of acceptable values for an action.
  //
  // 'discrete_idx' shall be in the range [0, action_discrete_count()).
  // 'continuous_idx' shall be in the range [0, action_continuous_count()).
  //
  // The acceptable values are in the closed range [min_value, max_value] for a
  // discrete action and in the closed range [min_value, max_value] for a
  // continuous action
  void (*action_discrete_bounds)(void* context, int discrete_idx,
                                 int* min_value, int* max_value);
  void (*action_continuous_bounds)(void* context, int continuous_idx,
                                   double* min_value, double* max_value);

  // The number of observations.
  int (*observation_count)(void* context);

  // The name associated with an observation.
  //
  // 'observation_idx' shall be in range [0, observation_count()).
  const char* (*observation_name)(void* context, int observation_idx);

  // Optional: The shape of the given observation.
  //
  // 'observation_idx' shall be in range [0, observation_count()).
  // 'spec' is invalidated by any other API call.
  //
  // If dims is zero in the resulting spec, the shape of this observation has to
  // be taken from the spec field of each individual observation. Otherwise, the
  // shape of every individual observation shall be identical to the one
  // returned from this call. (That is, the shape of observations is independent
  // of the environment state.)
  void (*observation_spec)(void* context, int observation_idx,
                           EnvCApi_ObservationSpec* spec);

  // The number of types of events. The value of the type field of any event
  // produced by the environment shall be in the range [0, event_type_count()).
  int (*event_type_count)(void* context);

  // The name associated with an event type.
  //
  // 'event_type_idx' shall be in range [0, event_type_count()).
  const char* (*event_type_name)(void* context, int event_type_idx);

  // An advisory metric that correlates discrete environment steps ("steps")
  // with real (wallclock) time: the number of steps per (real) second.
  int (*fps)(void* context);

  // Run loop
  ///////////
  //
  // Functions in this section shall only be called after a successful call of
  // start. Moreover, if 'advance' is called, no further function from this
  // section shall be called unless the returned status was "Running".

  // Writes the observation at the given index to '*obs'
  //
  // 'observation_idx' shall be in range [0, observation_count()).
  // 'obs' is invalidated by any other API call.
  void (*observation)(void* context, int observation_idx,
                      EnvCApi_Observation* obs);

  // Returns number of "pending events", i.e. events that occurred since last
  // call of 'advance'.
  int (*event_count)(void* context);

  // Writes the pending event at the given index to '*event'.
  //
  // 'event_idx' shall be in range [0, event_count()).
  // 'event' is invalidated by any other API call.
  void (*event)(void* context, int event_idx, EnvCApi_Event* event);

  // Sets the actions to use by future calls of 'advance'. Actions are "sticky",
  // and the same action values will continue to be used by 'advance' until the
  // actions are changed by this function.
  //
  // 'actions_discrete' shall be an array of size 'action_discrete_count()'.
  // 'actions_continuous' shall be an array of size 'action_continuous_count()'.
  // Each action value shall satisfy the bounds given by the functions
  // action_{discrete,continuous}_bounds above.
  void (*act)(void* context, const int actions_discrete[],
              const double actions_continuous[]);

  // Advances the environment by 'num_steps' steps, applying the currently
  // active set of actions. 'num_steps' shall be positive.
  //
  // The reward accrued during this call shall be stored in '*reward' (even if
  // it is zero). The return value describes the new status of the environment.
  // If it is anything other than "Running", the episode is over, and 'start'
  // needs to be called to start a new episode.
  EnvCApi_EnvironmentStatus (*advance)(void* context, int num_steps,
                                       double* reward);
};

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // DEEPMIND_ENV_C_API_H_
