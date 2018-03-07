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

#ifndef DML_DEEPMIND_ENGINE_CONTEXT_ACTIONS_H_
#define DML_DEEPMIND_ENGINE_CONTEXT_ACTIONS_H_

#include <memory>
#include <string>
#include <vector>

#include "deepmind/lua/lua.h"
#include "deepmind/lua/table_ref.h"

namespace deepmind {
namespace lab {

class ContextActions {
 public:
  // Reads the custom action spec from the table passed in.
  // Keeps a reference to the table for further calls. Returns 0 on success
  // and non-zero on failure.
  int ReadSpec(lua::TableRef script_table_ref);

  // Script action count.
  int DiscreteCount() const { return infos_.size(); }

  // Script action name. `idx` shall be in [0, Count()).
  const char* DiscreteName(int idx) const {
    return infos_[idx].name.c_str();
  }

  // Script action spec. `idx` shall be in [0, Count()).
  void DiscreteBounds(int idx, int* min_value_out, int* max_value_out) const;

  // Script action called with an array of actions.
  void DiscreteApply(const int* actions);

 private:
  // Entry for a custom action spec.
  struct ActionInfo {
    std::string name;
    int min_value;
    int max_value;
  };

  lua::TableRef script_table_ref_;

  // Storage of supplementary action types from script.
  std::vector<ActionInfo> infos_;
};

}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_ENGINE_CONTEXT_ACTIONS_H_
