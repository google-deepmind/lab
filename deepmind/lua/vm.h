// Copyright (C) 2016 Google Inc.
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

#ifndef DML_DEEPMIND_LUA_VM_H_
#define DML_DEEPMIND_LUA_VM_H_

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "deepmind/lua/lua.h"

namespace deepmind {
namespace lab {
namespace lua {
namespace internal {

struct EmbeddedLuaFile {
  const char* buff;
  std::size_t size;
};

struct EmbeddedClosure {
  lua_CFunction function;
  std::vector<void*> up_values;
};

struct Close {
  void operator()(lua_State* L) { lua_close(L); }
};

}  // namespace internal

class Vm {
 public:
  static Vm Create() {
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    return Vm(L);
  }

  // Maintain unique_ptr interface.
  lua_State* get() { return lua_state_.get(); }
  const lua_State* get() const { return lua_state_.get(); }
  bool operator!=(const std::nullptr_t) const { return lua_state_ != nullptr; }
  bool operator==(const std::nullptr_t) const { return lua_state_ == nullptr; }

  // The following two functions add modules to the Lua VM. That is, when Lua
  // executes the expression "require 'foo'", it passes the string 'foo' to the
  // module searchers until one of them succeeds. Registering the name 'foo'
  // using one of these functions will make the lookup succeed for that name.
  //
  // If the module was registered with AddCModuleToSearchers, the function F is
  // called, and the call produces the value of the "require" expression. (It is
  // advisable for the call to return a single table on the stack.)
  // The upvalues will be available when the module is called.
  //
  // If the module was registerd with AddLuaModuleToSearchers, the script
  // contained in the string [buf, buf + size) is executed as if it were the
  // body of a single function. (It is advisable for the script to return a
  // single table.) Any errors in the script are propagated to the calling
  // script.
  void AddCModuleToSearchers(
      std::string module_name,
      lua_CFunction F,
      std::vector<void*> up_values = {});

  void AddLuaModuleToSearchers(
      std::string module_name,
      const char* buf,
      std::size_t size);

  // Add a path to be included in search when calling require.
  void AddPathToSearchers(const std::string& path);

 private:
  // Takes ownership of lua_State.
  explicit Vm(lua_State* L);

  std::unique_ptr<lua_State, internal::Close> lua_state_;

  // These are unique_ptrs as the pointers are stored in upvalues for a Lua
  // module search function.
  std::unique_ptr<absl::flat_hash_map<std::string, internal::EmbeddedClosure>>
      embedded_c_modules_;
  std::unique_ptr<absl::flat_hash_map<std::string, internal::EmbeddedLuaFile>>
      embedded_lua_modules_;
};

inline Vm CreateVm() { return Vm::Create(); }

}  // namespace lua
}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_LUA_VM_H_
