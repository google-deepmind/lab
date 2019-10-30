// Copyright 2016-2019 Google LLC
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

// The version MAJ.MIN relates to compatibility as follows. Assuming an
// environment that implements the API and a client that calls into the API,
// if both components are built separately, then the following restrictions
// apply:
//
// * There is no ABI compatibility: both environment and client need to be
//   compiled at the exact same version MAJ.MIN (e.g. even minor version
//   changes can reorder or add struct members).
//
// * There is no API stability for environments: an environment written
//   against API version MAJ1.MIN1 needs to be migrated when upgrading to
//   greater API version MAJ2.MIN2 (e.g. to provide new functions; there
//   are no "optional" functions).
//
// * There is API stability for clients within the minor version: A client
//   written for version MAJ.MIN1 can be recompiled without modification
//   against an upgraded API of version MAJ.MIN2, MIN2 >= MIN1, within the
//   same major version. However, when changing major versions, migration
//   may be required.
//
// In other words, evolution within one major version can add new features
// and deprecate existing features (and provide replacements), but cannot
// remove or change the meaning of an existing feature. Ideally, if a feature
// is changed incompatibly in a major version change, the previous version
// should provide a migration path so that clients and the API can be upgraded
// separately and incrementally.
//
// Environments that implement this API should check whether versions
// match, e.g. by making the user pass DEEPMIND_ENV_C_API_VERSION_MAJOR
// and DEEPMIND_ENV_C_API_VERSION_MINOR through the initial creation
// function call.
//
#define DEEPMIND_ENV_C_API_VERSION_MAJOR 2
#define DEEPMIND_ENV_C_API_VERSION_MINOR 0

#include <stdbool.h>
#include <stdint.h>

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

// A text action. Contains a string and its length.
typedef struct EnvCApi_TextAction_s EnvCApi_TextAction;

// The status of an environment. This status changes as the environment evolves.
// The meaning of the status values is as follows, but see also the "Run loop"
// section below for details.
//
// * Running:     An episode is currently running. Actions can be taken.
// * Terminated:  An episode has reached a terminal state and has thus ended.
// * Interrupted: A running episode has ended without reaching a terminal state.
// * Error:       An error has occurred, the episode is no longer active.
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
  EnvCApi_ObservationBytes,
  EnvCApi_ObservationString
};
typedef enum EnvCApi_ObservationType_enum EnvCApi_ObservationType;

// An observation is either a string or a multidimensional array of numbers. The
// data type of the numbers is described by "type". The data is serialized as a
// flat array of size shape[0] * shape[1] * ... * shape[dims - 1], laid out in
// strides in row-major (C-style) order. If the "type" is
// "EnvCApi_ObservationString" then the "dims" shall be 1 and "shape[0]" the
// length of the string.
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
    const char* string;
  } payload;
};

// An event consists of an opaque ID, whose meaning can be queried through the
// API, as well as a (possibly empty) set of observations.
struct EnvCApi_Event_s {
  int id;
  int observation_count;
  const EnvCApi_Observation* observations;
};

// A text action consists of a string and its length.
struct EnvCApi_TextAction_s {
  const char* data;
  uint64_t len;
};

// A property is a key-value pair of strings that represent a current
// environment's configuration. Some properties are editable and may take
// effect immediately or during the next call to advance() or start().
//
// Each property has attributes that describes what operations are available for
// a given property.
//
// * Readable:     Property can be read via a call to 'read_property'.
// * Writable:     Property can be set via a call to 'write_property'.
// * ReadWritable: Convenience flag to describe (Readable | Writable).
// * Listable:     Property has sub-properties and can be retrieved via a
//                 call to 'for_each_property_in_list'.
enum EnvCApi_PropertyAttributes_enum {
  EnvCApi_PropertyAttributes_Readable = 1 << 0,
  EnvCApi_PropertyAttributes_Writable = 1 << 1,
  EnvCApi_PropertyAttributes_ReadWritable =
      EnvCApi_PropertyAttributes_Readable | EnvCApi_PropertyAttributes_Writable,
  EnvCApi_PropertyAttributes_Listable = 1 << 2,
};
typedef enum EnvCApi_PropertyAttributes_enum EnvCApi_PropertyAttributes;

// Result of property operations.
//
// * Success:          Operation was a success.
// * NotFound:         Property does not exist.
// * PermissionDenied: Operation not allowed on property.
// * InvalidArgument:  Property could not accept value provided.
enum EnvCApi_PropertyResult_enum {
  EnvCApi_PropertyResult_Success = 0,
  EnvCApi_PropertyResult_NotFound = 1,
  EnvCApi_PropertyResult_PermissionDenied = 2,
  EnvCApi_PropertyResult_InvalidArgument = 3,
};
typedef enum EnvCApi_PropertyResult_enum EnvCApi_PropertyResult;

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
  // function shall only be called before 'init', and it shall return zero
  // on success and non-zero on failure. On failure an associated error message
  // maybe retrieved by calling 'error_message'.
  int (*setting)(void* context, const char* key, const char* value);

  // Shall be called after all calls of 'setting' and before 'start'. It shall
  // return zero on success and non-zero on failure. Most functions from this
  // API shall only be called after a successful call of 'init'; see their
  // individual documentation. On failure an associated error message
  // maybe retrieved by calling 'error_message'.
  int (*init)(void* context);

  // Launches an episode using 'episode_id' and 'seed'. Returns zero on success,
  // or EAGAIN (from <errno.h>), or any other non-zero value on failure. If
  // EAGAIN is returned, this function may be called again until successful.
  // Some functions from this API shall only be called after a successful call
  // of 'start'; see their individual documentation. On failure an associated
  // error message maybe retrieved by calling 'error_message'.
  // This function shall only be called after a successful call of 'init'.
  int (*start)(void* context, int episode_id, int seed);

  // Releases the resources associated with 'context', which shall not be used
  // again after this call. (Note that this API does not specify how a context
  // is created, only how it is destroyed.)
  void (*release_context)(void* context);

  // Returns an error message associated with a failed call.
  // Shall only be called immediately a failed call to 'setting', 'init',
  // 'start' or 'advance'.
  const char* (*error_message)(void* context);

  // Meta data querying/setting
  /////////////////////
  //
  // Functions in this section shall only be called after a successful call of
  // 'init', but beyond that they may be called at any time, regardless of
  // whether 'start' has been called or whether the episode has terminated.

  // Writes 'value' to property `key`.
  // If the function returns:
  //
  // *   Success: property's value was updated. The value may not updated be
  //     immediately and there is no round-trip guarantee so subsequent calls to
  //     read may not produce the same value that was set.
  //
  // *   InvalidArgument: property's value was not updated. This could be caused
  //     by 'value' not being in the acceptable range of values for the
  //     property.
  //
  // *   PermissionDenied: property is not writable.
  //
  // *   NotFound: property does not exist.
  //
  // No other return values are valid.
  EnvCApi_PropertyResult (*write_property)(void* context, const char* key,
                                           const char* value);

  // Reads the value of property 'key'.
  // If the function returns:
  //
  // *   Success: '*value' is set to the property's value. The pointer to value
  //     will only be valid until the next call to the API.
  //
  // *   PermissionDenied: property is not readable. '*value' is left unchanged.
  //
  // *   NotFound: property does not exist. '*value' is left unchanged.
  //
  // No other return values are valid.
  EnvCApi_PropertyResult (*read_property)(void* context, const char* key,
                                          const char** value);

  // List sub-properties of property 'list_key'.
  // If the function returns:
  //
  // *   Success: 'prop_callback' will be called for each sub-property.
  //     `userdata` will be passed through to the callback. No other calls to
  //     the API are allowed during the callback. Callback order is unspecified
  //     and may change at any time. The string 'key' is only valid for the
  //     duration of the callback.
  //
  // *   PermissionDenied: property is not listable.
  //
  // *   NotFound: property does not exist.
  //
  // No other return values are valid.
  EnvCApi_PropertyResult (*list_property)(
      void* context, void* userdata, const char* list_key,
      void (*prop_callback)(void* userdata, const char* key,
                            EnvCApi_PropertyAttributes attributes));

  // Name of the environment.
  const char* (*environment_name)(void* context);

  // The number of discrete actions.
  int (*action_discrete_count)(void* context);

  // The number of continuous actions.
  int (*action_continuous_count)(void* context);

  // The number of text actions.
  int (*action_text_count)(void* context);

  // Retrieves the name associated with a discrete or continuous action.
  //
  // 'discrete_idx' shall be in the range [0, action_discrete_count()).
  // 'continuous_idx' shall be in the range [0, action_continuous_count()).
  // 'text_idx' shall be in the range [0, action_text_count()).
  const char* (*action_discrete_name)(void* context, int discrete_idx);
  const char* (*action_continuous_name)(void* context, int continuous_idx);
  const char* (*action_text_name)(void* context, int text_idx);

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

  // Run loop
  ///////////
  //
  // Functions in this section shall only be called after a successful call of
  // 'start'. Moreover, if 'advance' is called, no further function from this
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

  // Sets the discrete actions to use by future calls of 'advance'.
  // Actions are "sticky", and the same action values will continue to be used
  // by 'advance' until the actions are changed by this function.
  //
  // 'actions_discrete' shall be an array of size 'action_discrete_count()'.
  void (*act_discrete)(void* context, const int actions_discrete[]);

  // Sets the continuous actions to use by future calls of 'advance'.
  // Actions are "sticky", and the same action values will continue to be used
  // by 'advance' until the actions are changed by this function.
  //
  // 'actions_continuous' shall be an array of size 'action_continuous_count()'.
  void (*act_continuous)(void* context, const double actions_continuous[]);

  // Sets the text to use by future calls of 'advance'.
  // Each text action must be a null-terminated string.
  //
  // 'actions_text' shall be an array of size 'action_text_count()'.
  void (*act_text)(void* context, const EnvCApi_TextAction actions_text[]);

  // Advances the environment by 'num_steps' steps, applying the currently
  // active set of actions. 'num_steps' shall be positive.
  //
  // The reward accrued during this call shall be stored in '*reward' (even if
  // it is zero). The return value describes the new status of the environment.
  // If it is anything other than "Running", the episode is over, and 'start'
  // needs to be called to start a new episode. If the returned status was
  // Error, an associated message maybe retrieved by calling 'error_message'.
  EnvCApi_EnvironmentStatus (*advance)(void* context, int num_steps,
                                       double* reward);
};

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // DEEPMIND_ENV_C_API_H_
