// Copyright (C) 2018-2019 Google Inc.
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

#ifndef DML_DEEPMIND_LUA_STACK_RESETTER_H_
#define DML_DEEPMIND_LUA_STACK_RESETTER_H_

#include "deepmind/lua/lua.h"

namespace deepmind {
namespace lab {
namespace lua {

// On construction stores the current Lua stack position.
// On destruction returns Lua stack to the position it was constructed in.
//
// Example:
//
//  {
//    StackResetter stack_resetter(L);
//    PushLuaFunctionOnToStack();
//    auto result = Call(L, 0);
//    if (result.n_results() > 0) {
//      return true; // No need to call lua_pop(L, result.n_results());
//    }
//  }
class StackResetter {
 public:
  // 'L' is stored along with current stack size.
  explicit StackResetter(lua_State* L)
      : lua_state_(L), stack_size_(lua_gettop(lua_state_)) {}
  ~StackResetter() { lua_settop(lua_state_, stack_size_); }

  StackResetter& operator=(const StackResetter&) = delete;
  StackResetter(const StackResetter&) = delete;

 private:
  lua_State* lua_state_;
  int stack_size_;
};

}  // namespace lua
}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_LUA_STACK_RESETTER_H_
