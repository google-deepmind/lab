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

#ifndef DML_DEEPMIND_LUA_PUSH_H_
#define DML_DEEPMIND_LUA_PUSH_H_

#include <array>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <string>
#include <type_traits>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "absl/types/variant.h"
#include "deepmind/lua/lua.h"

namespace deepmind {
namespace lab {
namespace lua {

// [0, +1, -]
inline void Push(lua_State* L, const absl::string_view value) {
  lua_pushlstring(L, value.data(), value.size());
}

// [0, +1, -]
inline void Push(lua_State* L, const char* value) {
  lua_pushlstring(L, value, std::strlen(value));
}

// [0, +1, -]
inline void Push(lua_State* L, lua_Number value) { lua_pushnumber(L, value); }

// [0, +1, -]
inline void Push(lua_State* L, lua_Integer value) { lua_pushinteger(L, value); }

// [0, +1, -]
inline void Push(lua_State* L, bool value) { lua_pushboolean(L, value); }

// [0, +1, -]
inline void Push(lua_State* L, lua_CFunction value) {
  lua_pushcfunction(L, value);
}

// [0, +1, -]
inline void Push(lua_State* L, void* value) { lua_pushlightuserdata(L, value); }

// Templated Push that takes any arithmetic values that don't have non-template
// overloads already.
//
// [0, +1, -]
template <typename T>
typename std::enable_if<std::is_floating_point<T>::value>::type
Push(lua_State* L, T value) {
  Push(L, static_cast<lua_Number>(value));
}
template <typename T>
typename std::enable_if<std::is_integral<T>::value &&
                        !std::is_same<T, bool>::value>::type
Push(lua_State* L, T value) {
  Push(L, static_cast<lua_Integer>(value));
}

// [0, +1, -]
template <typename T, typename A>
void Push(lua_State* L, const std::vector<T, A>& values);

// [0, +1, -]
template <typename T, std::size_t N>
void Push(lua_State* L, const std::array<T, N>& values);

// [0, +1, -]
template <typename K, typename T, typename H, typename C, typename A>
void Push(lua_State* L, const absl::flat_hash_map<K, T, H, C, A>& values);

// [0, +1, -]
template <typename T>
void Push(lua_State* L, absl::Span<T> values);

// [0, +1, -]
template <typename... T>
void Push(lua_State* L, const absl::variant<T...>& value);

// End of public header, implementation details follow.

template <typename T>
void Push(lua_State* L, absl::Span<T> values) {
  lua_createtable(L, values.size(), 0);
  for (std::size_t i = 0; i < values.size(); ++i) {
    Push(L, i + 1);
    Push(L, values[i]);
    lua_settable(L, -3);
  }
}

template <typename T, typename A>
void Push(lua_State* L, const std::vector<T, A>& values) {
  Push(L, absl::MakeConstSpan(values));
}

template <typename T, std::size_t N>
void Push(lua_State* L, const std::array<T, N>& values) {
  Push(L, absl::MakeConstSpan(values));
}

template <typename K, typename T, typename H, typename C, typename A>
void Push(lua_State* L, const absl::flat_hash_map<K, T, H, C, A>& values) {
  lua_createtable(L, 0, values.size());
  for (const auto& pair : values) {
    Push(L, pair.first);
    Push(L, pair.second);
    lua_settable(L, -3);
  }
}

namespace internal {

struct PushVariant {
  template <typename T>
  void operator()(const T& value) const {
    Push(L, value);
  }

  void operator()(const absl::monostate) const {
    lua_pushnil(L);
  }

  lua_State* L;
};

}  // namespace internal

template <typename... T>
void Push(lua_State* L, const absl::variant<T...>& value) {
  absl::visit(internal::PushVariant{L}, value);
}

}  // namespace lua
}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_LUA_PUSH_H_
