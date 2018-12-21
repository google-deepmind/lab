// Copyright (C) 2018 Google Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DML_DEEPMIND_ENGINE_CONTEXT_EVENTS_H_
#define DML_DEEPMIND_ENGINE_CONTEXT_EVENTS_H_

#include <string>
#include <vector>

#include "absl/container/node_hash_map.h"
#include "deepmind/lua/lua.h"
#include "deepmind/lua/n_results_or.h"
#include "third_party/rl_api/env_c_api.h"

namespace deepmind {
namespace lab {

// Support class for storing events generated from Lua. These events can be read
// out of DM Lab using the events part of the EnvCApi. (See: env_c_api.h.)
//
// Each event contains a list of observations. Each observation type is one of
// EnvCApi_Observation{Doubles,Bytes,String}.
class ContextEvents {
 public:
  // Returns an event module. A pointer to ContextEvents must exist in the up
  // value. [0, 1, -]
  static lua::NResultsOr Module(lua_State* L);

  // Adds event returning its index.
  int Add(std::string name);

  // Adds string observation to event at index 'event_id'.
  void AddObservation(int event_id, std::string string_value);

  // Adds DoubleTensor observation to event at index 'event_id'.
  void AddObservation(int event_id, std::vector<int> shape,
                      std::vector<double> double_tensor);

  // Adds ByteTensor observation to event at index 'event_id'.
  void AddObservation(int event_id, std::vector<int> shape,
                      std::vector<unsigned char> byte_tensor);

  // Exports an event at 'event_idx', which must be in range [0, Count()), to an
  // EnvCApi_Event structure. Observations within the `event` are invalidated by
  // calls to non-const methods.
  void Export(int event_idx, EnvCApi_Event* event);

  // Returns the number of events created since last call to ClearEvents().
  int Count() const { return events_.size(); }

  // Returns the number of event types.
  int TypeCount() const { return names_.size(); }

  // Returns the name of the event associated with event_type_id, which must be
  // in range [0, EventTypeCount()). New events types maybe added at any point
  // but the event_type_ids remain stable.
  const char* TypeName(int event_type_id) const {
    return names_[event_type_id];
  }

  // Clears all the events and their observations.
  void Clear();

 private:
  struct Event {
    int type_id;  // Event type id.

    struct Observation {
      EnvCApi_ObservationType_enum type;

      int shape_id;  // Index in shapes_ for shape of this observation.

      // Index of observation data. The array depends on type.
      // If type == EnvCApi_ObservationDoubles then index in doubles_.
      // If type == EnvCApi_ObservationBytes then index in bytes_.
      // If type == EnvCApi_ObservationString then index in string_.
      int array_id;
    };

    // List of observations associated with event.
    std::vector<Observation> observations;
  };

  // Events generated since construction or last call to Clear().
  std::vector<Event> events_;

  // Bidirectional lookup for the mapping between type_id and event_name.
  // Strings in the primary container (the map) need to be stable.
  std::vector<const char*> names_;
  absl::node_hash_map<std::string, int> name_to_id_;

  // Event observation storage.
  std::vector<std::vector<int>> shapes_;
  std::vector<std::vector<unsigned char>> bytes_;
  std::vector<std::vector<double>> doubles_;
  std::vector<std::string> strings_;

  // Temporary EnvCApi_Observation observation storage.
  std::vector<EnvCApi_Observation> observations_;
};

}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_ENGINE_CONTEXT_EVENTS_H_
