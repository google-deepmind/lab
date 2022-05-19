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

#include "deepmind/engine/context_actions.h"

#include <iostream>
#include <utility>
#include <vector>

#include "absl/log/check.h"
#include "absl/types/span.h"
#include "deepmind/lua/call.h"
#include "deepmind/lua/n_results_or.h"
#include "deepmind/lua/push.h"
#include "deepmind/lua/read.h"
#include "deepmind/lua/stack_resetter.h"
#include "deepmind/lua/table_ref.h"
#include "deepmind/tensor/lua_tensor.h"

namespace deepmind {
namespace lab {

int ContextActions::ReadSpec(lua::TableRef script_table_ref) {
  script_table_ref_ = std::move(script_table_ref);
  lua_State* L = script_table_ref_.LuaState();
  lua::StackResetter stack_resetter(L);
  script_table_ref_.PushMemberFunction("customDiscreteActionSpec");
  if (lua_isnil(L, -2)) {
    return 0;
  }
  auto result = lua::Call(L, 1);
  if (!result.ok()) {
    std::cerr << result.error() << '\n';
    return 1;
  }
  lua::TableRef actions;
  lua::Read(L, -1, &actions);
  auto action_count = actions.ArraySize();
  infos_.clear();
  infos_.reserve(action_count);
  for (std::size_t i = 0, c = action_count; i != c; ++i) {
    lua::TableRef info;
    actions.LookUp(i + 1, &info);
    ActionInfo action_info;
    if (!info.LookUp("name", &action_info.name)) {
      std::cerr << "[customDiscreteActionSpec] - Missing 'name = <string>'.\n";
      return 1;
    }
    if (!IsFound(info.LookUp("min", &action_info.min_value))) {
      std::cerr << "[customDiscreteActionSpec] - Missing 'min = number'.\n";
      return 1;
    }

    if (!IsFound(info.LookUp("max", &action_info.max_value))) {
      std::cerr << "[customDiscreteActionSpec] - Missing 'max = number'.\n";
      return 1;
    }
    infos_.push_back(std::move(action_info));
  }
  return 0;
}

void ContextActions::DiscreteApply(const int* actions) {
  if (infos_.empty()) { return; }
  lua_State* L = script_table_ref_.LuaState();
  lua::StackResetter stack_resetter(L);
  script_table_ref_.PushMemberFunction("customDiscreteActions");
  // Function must exist.
  CHECK(!lua_isnil(L, -2))
      << "Custom action spec set but no customDiscreteActions member function";
  lua::Push(L, absl::MakeConstSpan(actions, infos_.size()));
  auto result = lua::Call(L, 2);
  CHECK(result.ok()) << "[customDiscreteActions] - " << result.error();
}

void ContextActions::DiscreteBounds(int idx, int* min_value_out,
                            int* max_value_out) const {
  const auto& info = infos_[idx];
  *min_value_out = info.min_value;
  *max_value_out = info.max_value;
}

}  // namespace lab
}  // namespace deepmind
