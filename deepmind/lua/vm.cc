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

#include "deepmind/lua/vm.h"

#include "deepmind/lua/lua.h"

#if LUA_VERSION_NUM == 501
static constexpr char kSearcher[] = "loaders";
#elif LUA_VERSION_NUM == 502
static constexpr char kSearcher[] = "searchers";
#else
#error Only Lua 5.1 and 5.2 are supported
#endif

using deepmind::lab::lua::internal::EmbeddedLuaFile;
using deepmind::lab::lua::internal::EmbeddedClosure;

extern "C" {
static int PackageLoader(lua_State* L) {
  do {
    int upidx_c = lua_upvalueindex(1);
    int upidx_lua = lua_upvalueindex(2);
    if (!lua_islightuserdata(L, upidx_c) ||
        !lua_islightuserdata(L, upidx_lua)) {
      lua_pushstring(L, "Missing searchers");
      break;
    }

    auto* embedded_c_modules =
        static_cast<const absl::flat_hash_map<std::string, EmbeddedClosure>*>(
            lua_touserdata(L, upidx_c));
    auto* embedded_lua_modules =
        static_cast<const absl::flat_hash_map<std::string, EmbeddedLuaFile>*>(
            lua_touserdata(L, upidx_lua));

    if (lua_type(L, 1) != LUA_TSTRING) {
      // Allow other searchers to deal with this.
      lua_pushstring(L, "'required' called with a non-string argument!");
      return 1;
    }

    std::size_t length = 0;
    const char* result_cstr = lua_tolstring(L, 1, &length);
    std::string name(result_cstr, length);
    auto it = embedded_c_modules->find(name);
    if (it != embedded_c_modules->end()) {
      for (void* light_value_data : it->second.up_values) {
        lua_pushlightuserdata(L, light_value_data);
      }
      lua_pushcclosure(L, it->second.function, it->second.up_values.size());
      return 1;
    } else {
      auto kt = embedded_lua_modules->find(name);
      if (kt != embedded_lua_modules->end()) {
        if (luaL_loadbuffer(L, kt->second.buff, kt->second.size,
                            name.c_str())) {
          // Error message is on stack. Let caller deal with it.
          break;
        } else {
          return 1;
        }
      } else {
        // Allow other searchers to deal with this.
        lua_pushstring(L, "Not found internally!");
        return 1;
      }
    }
  } while (false);
  return lua_error(L);
}
}  // extern "C"

namespace deepmind {
namespace lab {
namespace lua {

void Vm::AddPathToSearchers(const std::string& path) {
  lua_State* L = get();
  lua_getglobal(L, "package");
  lua_getfield(L, -1, "path");
  std::string new_path = lua_tostring(L, -1);
  lua_pop(L, 1);
  new_path += ";";
  new_path += path;
  new_path += "/?.lua";
  lua_pushlstring(L, new_path.c_str(), new_path.length());
  lua_setfield(L, -2, "path");
  lua_pop(L, 1);
}

void Vm::AddCModuleToSearchers(
    std::string module_name,
    lua_CFunction F,
    std::vector<void*> up_values) {
  (*embedded_c_modules_)[std::move(module_name)] = {F, std::move(up_values) };
}

void Vm::AddLuaModuleToSearchers(
    std::string module_name,
    const char* buf,
    std::size_t size) {
  (*embedded_lua_modules_)[std::move(module_name)] = {buf, size};
}

Vm::Vm(lua_State* L)
    : lua_state_(L),
      embedded_c_modules_(
          new absl::flat_hash_map<std::string, EmbeddedClosure>()),
      embedded_lua_modules_(
          new absl::flat_hash_map<std::string, EmbeddedLuaFile>()) {
  lua_getglobal(L, "package");
  lua_getfield(L, -1, kSearcher);
  int array_size = ArrayLength(L, -1);
  for (int e = array_size + 1; e > 1; e--) {
    lua_rawgeti(L, -1, e - 1);
    lua_rawseti(L, -2, e);
  }

  lua_pushlightuserdata(L, embedded_c_modules_.get());
  lua_pushlightuserdata(L, embedded_lua_modules_.get());
  lua_pushcclosure(L, &PackageLoader, 2);
  lua_rawseti(L, -2, 1);
  lua_pop(L, 2);
}

}  // namespace lua
}  // namespace lab
}  // namespace deepmind
