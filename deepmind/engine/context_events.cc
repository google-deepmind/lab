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

#include "deepmind/engine/context_events.h"

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "absl/log/check.h"
#include "absl/log/log.h"
#include "absl/container/node_hash_map.h"
#include "absl/meta/type_traits.h"
#include "deepmind/lua/class.h"
#include "deepmind/lua/lua.h"
#include "deepmind/lua/n_results_or.h"
#include "deepmind/lua/read.h"
#include "deepmind/tensor/lua_tensor.h"
#include "deepmind/tensor/tensor_view.h"
#include "third_party/rl_api/env_c_api.h"

namespace deepmind {
namespace lab {
namespace {

class LuaEventsModule : public lua::Class<LuaEventsModule> {
  friend class Class;
  static const char* ClassName() { return "deepmind.lab.Events"; }

 public:
  // '*ctx' owned by the caller and should out-live this object.
  explicit LuaEventsModule(ContextEvents* ctx) : ctx_(ctx) {}

  // Registers classes metatable with Lua.
  static void Register(lua_State* L) {
    const Class::Reg methods[] = {{"add", Member<&LuaEventsModule::Add>}};
    Class::Register(L, methods);
  }

 private:
  template <typename T>
  void AddTensorObservation(int id, const tensor::TensorView<T>& view) {
    const auto& shape = view.shape();
    std::vector<int> out_shape(shape.begin(), shape.end());

    std::vector<T> out_values;
    out_values.reserve(view.num_elements());
    view.ForEach([&out_values](T v) { out_values.push_back(v); });
    ctx_->AddObservation(id, std::move(out_shape), std::move(out_values));
  }

  // Signature events:add(eventName, [obs1, [obs2 ...] ...])
  // Called with an event name and a list of observations. Each observation
  // maybe one of string, ByteTensor or DoubleTensor.
  // [-(2 + #observations), 0, e]
  lua::NResultsOr Add(lua_State* L) {
    int top = lua_gettop(L);
    std::string name;
    if (!lua::Read(L, 2, &name)) {
      return "Event name must be a string";
    }
    int id = ctx_->Add(std::move(name));
    for (int i = 3; i <= top; ++i) {
      std::string string_arg;
      if (lua::Read(L, i, &string_arg)) {
        ctx_->AddObservation(id, std::move(string_arg));
      } else if (auto* double_tensor =
                     tensor::LuaTensor<double>::ReadObject(L, i)) {
        AddTensorObservation(id, double_tensor->tensor_view());
      } else if (auto* byte_tensor =
                     tensor::LuaTensor<unsigned char>::ReadObject(L, i)) {
        AddTensorObservation(id, byte_tensor->tensor_view());
      } else {
        return "[event] - Observation type not supported. Must be one of "
               "string|ByteTensor|DoubleTensor.";
      }
    }
    return 0;
  }

  ContextEvents* ctx_;
};

}  // namespace

lua::NResultsOr ContextEvents::Module(lua_State* L) {
  if (auto* ctx =
          static_cast<ContextEvents*>(lua_touserdata(L, lua_upvalueindex(1)))) {
    LuaEventsModule::Register(L);
    LuaEventsModule::CreateObject(L, ctx);
    return 1;
  } else {
    return "Missing event context!";
  }
}

int ContextEvents::Add(std::string name) {
  auto iter_inserted = name_to_id_.emplace(std::move(name), names_.size());
  if (iter_inserted.second) {
    names_.push_back(iter_inserted.first->first.c_str());
  }

  int id = events_.size();
  events_.push_back(Event{iter_inserted.first->second});
  return id;
}

void ContextEvents::AddObservation(int event_id, std::string string_value) {
  Event& event = events_[event_id];
  event.observations.emplace_back();
  auto& observation = event.observations.back();
  observation.type = EnvCApi_ObservationString;

  observation.shape_id = shapes_.size();
  std::vector<int> shape(1);
  shape[0] = string_value.size();
  shapes_.emplace_back(std::move(shape));

  observation.array_id = strings_.size();
  strings_.push_back(std::move(string_value));
}

void ContextEvents::AddObservation(int event_id, std::vector<int> shape,
                                   std::vector<double> double_tensor) {
  Event& event = events_[event_id];
  event.observations.emplace_back();
  auto& observation = event.observations.back();
  observation.type = EnvCApi_ObservationDoubles;

  observation.shape_id = shapes_.size();
  shapes_.push_back(std::move(shape));

  observation.array_id = doubles_.size();
  doubles_.push_back(std::move(double_tensor));
}

void ContextEvents::AddObservation(int event_id, std::vector<int> shape,
                                   std::vector<unsigned char> byte_tensor) {
  Event& event = events_[event_id];
  event.observations.emplace_back();
  auto& observation = event.observations.back();
  observation.type = EnvCApi_ObservationBytes;

  observation.shape_id = shapes_.size();
  shapes_.push_back(std::move(shape));

  observation.array_id = bytes_.size();
  bytes_.push_back(std::move(byte_tensor));
}

void ContextEvents::Clear() {
  events_.clear();
  strings_.clear();
  shapes_.clear();
  doubles_.clear();
  bytes_.clear();
}

void ContextEvents::Export(int event_idx, EnvCApi_Event* event) {
  const auto& internal_event = events_[event_idx];
  observations_.clear();
  observations_.reserve(internal_event.observations.size());
  for (const auto& observation : internal_event.observations) {
    observations_.emplace_back();
    auto& observation_out = observations_.back();
    observation_out.spec.type = observation.type;

    const auto& shape = shapes_[observation.shape_id];
    observation_out.spec.dims = shape.size();
    observation_out.spec.shape = shape.data();

    switch (observation.type) {
      case EnvCApi_ObservationBytes: {
        const auto& tensor = bytes_[observation.array_id];
        observation_out.payload.bytes = tensor.data();
        break;
      }
      case EnvCApi_ObservationDoubles: {
        const auto& tensor = doubles_[observation.array_id];
        observation_out.payload.doubles = tensor.data();
        break;
      }
      case EnvCApi_ObservationString: {
        const auto& string_value = strings_[observation.array_id];
        observation_out.payload.string = string_value.c_str();
        break;
      }
      default:
        LOG(FATAL) << "Observation type: " << observation.type
                   << " not supported";
    }
  }
  event->id = internal_event.type_id;
  event->observations = observations_.data();
  event->observation_count = observations_.size();
}

}  // namespace lab
}  // namespace deepmind
