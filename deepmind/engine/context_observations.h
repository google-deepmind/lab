// Copyright (C) 2017 Google Inc.
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

#ifndef DML_DEEPMIND_ENGINE_CONTEXT_OBSERVATIONS_H_
#define DML_DEEPMIND_ENGINE_CONTEXT_OBSERVATIONS_H_

#include <memory>
#include <string>
#include <vector>

#include "deepmind/lua/lua.h"
#include "deepmind/lua/n_results_or.h"
#include "deepmind/lua/table_ref.h"
#include "third_party/rl_api/env_c_api.h"

namespace deepmind {
namespace lab {

class ContextObservations {
 public:
  // Reads the custom observation spec from the table passed in.
  // Keeps a reference to the table for further calls. Returns 0 on success
  // and non-zero on failure.
  int ReadSpec(lua::TableRef script_table_ref);

  // Script observation count.
  int Count() const { return infos_.size(); }

  // Script observation name. `idx` shall be in [0, Count()).
  const char* Name(int idx) const {
    return infos_[idx].name.c_str();
  }

  // Script observation spec. `idx` shall be in [0, Count()).
  void Spec(int idx, EnvCApi_ObservationSpec* spec) const;

  // Script observation. `idx` shall be in [0, Count()).
  void Observation(int idx, EnvCApi_Observation* observation);

 private:
  // Entry for a custom observation spec.
  struct SpecInfo {
    std::string name;
    EnvCApi_ObservationType type;
    std::vector<int> shape;
  };

  lua::TableRef script_table_ref_;

  // Storage of supplementary observation types from script.
  std::vector<SpecInfo> infos_;

  // Used to hold the EnvCApi_ObservationSpec::shape values until the next call
  // of observation.
  std::vector<int> tensor_shape_;

  // Used to hold a reference to the observation tensor until the next call of
  // observation.
  lua::TableRef tensor_;

  // Used to store the observation string until the next call of observation.
  std::string string_;
};

}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_ENGINE_CONTEXT_OBSERVATIONS_H_
