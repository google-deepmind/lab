// Copyright (C) 2017-2018 Google Inc.
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

#include "deepmind/engine/context_observations.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "absl/log/check.h"
#include "absl/log/log.h"
#include "deepmind/lua/call.h"
#include "deepmind/lua/n_results_or.h"
#include "deepmind/lua/push.h"
#include "deepmind/lua/read.h"
#include "deepmind/lua/table_ref.h"
#include "deepmind/tensor/lua_tensor.h"
#include "deepmind/tensor/tensor_view.h"
#include "third_party/rl_api/env_c_api.h"

namespace deepmind {
namespace lab {

int ContextObservations::ReadSpec(lua::TableRef script_table_ref) {
  script_table_ref_ = std::move(script_table_ref);
  script_table_ref_.PushMemberFunction("customObservationSpec");
  lua_State* L = script_table_ref_.LuaState();
  if (lua_isnil(L, -2)) {
    lua_pop(L, 2);
    return 0;
  }
  auto result = lua::Call(L, 1);
  if (!result.ok()) {
    std::cerr << result.error() << '\n';
    return 1;
  }
  lua::TableRef observations;
  lua::Read(L, -1, &observations);
  auto spec_count = observations.ArraySize();
  infos_.clear();
  infos_.reserve(spec_count);
  for (std::size_t i = 0, c = spec_count; i != c; ++i) {
    lua::TableRef info;
    observations.LookUp(i + 1, &info);
    SpecInfo spec_info;
    if (!info.LookUp("name", &spec_info.name)) {
      std::cerr << "[customObservationSpec] - Missing 'name = <string>'.\n";
      return 1;
    }
    std::string type = "Doubles";
    info.LookUp("type", &type);
    if (type == "Bytes") {
      spec_info.type = EnvCApi_ObservationBytes;
    } else if (type == "Doubles") {
      spec_info.type = EnvCApi_ObservationDoubles;
    } else if (type == "String") {
      spec_info.type = EnvCApi_ObservationString;
    } else {
      std::cerr << "[customObservationSpec] - Missing 'type = "
                   "'Bytes'|'Doubles'|'String''.\n";
      return 1;
    }
    if (!info.LookUp("shape", &spec_info.shape)) {
      std::cerr
          << "[customObservationSpec] - Missing 'shape = {<int>, ...}'.\n";
      return 1;
    }
    infos_.push_back(std::move(spec_info));
  }
  lua_pop(L, result.n_results());
  return 0;
}

void ContextObservations::Observation(int idx,
                                      EnvCApi_Observation* observation) {
  lua_State* L = script_table_ref_.LuaState();
  script_table_ref_.PushMemberFunction("customObservation");
  // Function must exist.
  CHECK(!lua_isnil(L, -2))
      << "Observations Spec set but no observation member function";
  const auto& info = infos_[idx];
  lua::Push(L, info.name);
  auto result = lua::Call(L, 2);
  CHECK(result.ok()) << "[customObservation] - " << result.error();

  const tensor::Layout* layout = nullptr;
  observation->spec.type = info.type;
  switch (info.type) {
    case EnvCApi_ObservationDoubles: {
      const char error_message[] =
          "[customObservation] - Must return a contiguous DoubleTensor";
      CHECK_EQ(1, result.n_results()) << error_message;
      auto* double_tensor = tensor::LuaTensor<double>::ReadObject(L, -1);
      CHECK(double_tensor != nullptr) << error_message;
      const auto& view = double_tensor->tensor_view();
      CHECK(view.IsContiguous()) << error_message;
      layout = &view;
      observation->payload.doubles = view.storage() + view.start_offset();
      break;
    }
    case EnvCApi_ObservationBytes: {
      const char error_message[] =
          "[customObservation] - Must return a contiguous ByteTensor";
      CHECK_EQ(1, result.n_results()) << error_message;
      auto* byte_tensor = tensor::LuaTensor<std::uint8_t>::ReadObject(L, -1);
      CHECK(byte_tensor != nullptr) << error_message;
      const auto& view = byte_tensor->tensor_view();
      layout = &view;
      CHECK(view.IsContiguous()) << error_message;
      observation->payload.bytes = view.storage() + view.start_offset();
      break;
    }
    case EnvCApi_ObservationString: {
      const char error_message[] = "[customObservation] - Must return a string";
      CHECK_EQ(1, result.n_results()) << error_message;
      CHECK(lua::Read(L, -1, &string_)) << error_message;
      observation->payload.string = string_.data();
      tensor_shape_.assign(1, string_.length());
      observation->spec.dims = tensor_shape_.size();
      observation->spec.shape = tensor_shape_.data();
      break;
    }
    default:
      LOG(FATAL) << "Observation type: " << info.type << " not supported";
  }

  if (layout != nullptr) {
    tensor_shape_.resize(layout->shape().size());
    std::copy(layout->shape().begin(), layout->shape().end(),
              tensor_shape_.begin());
    observation->spec.dims = tensor_shape_.size();
    observation->spec.shape = tensor_shape_.data();
    // Prevent observation->payload from being destroyed during pop.
    lua::Read(L, -1, &tensor_);
  }
  lua_pop(L, result.n_results());
}

void ContextObservations::Spec(int idx, EnvCApi_ObservationSpec* spec) const {
  const auto& info = infos_[idx];
  spec->type = info.type;
  spec->dims = info.shape.size();
  spec->shape = info.shape.data();
}

}  // namespace lab
}  // namespace deepmind
