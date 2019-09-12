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

#ifndef DML_DEEPMIND_LUA_READ_H_
#define DML_DEEPMIND_LUA_READ_H_

#include <array>
#include <cstddef>
#include <cstdlib>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "absl/types/variant.h"
#include "deepmind/lua/lua.h"

namespace deepmind {
namespace lab {
namespace lua {

// Status of a read operation.
class ReadResult {
 public:
  enum ReadResultEnum {
    kFound = 0,
    kNotFound = 1,
    kTypeMismatch = 2,
  };

  constexpr ReadResult(const ReadResult&) = default;
  ReadResult& operator=(const ReadResult&) = default;

  // Legacy - please use 'IsFound(read_result)' instead.
  constexpr operator bool() const { return value_ == kFound; }

  constexpr ReadResultEnum Value() const { return value_; }

  // Read was successful.
  friend constexpr ReadResult ReadFound();

  // Read from Lua stack was none or nil.
  friend constexpr ReadResult ReadNotFound();

  // Item was present but not convertible to the desired type.
  friend constexpr ReadResult ReadTypeMismatch();

  friend constexpr bool operator==(ReadResult lhs, ReadResult rhs) {
    return lhs.value_ == rhs.value_;
  }

  friend constexpr bool IsFound(ReadResult read_result) {
    return read_result.value_ == kFound;
  }

  friend constexpr bool IsTypeMismatch(ReadResult read_result) {
    return read_result.value_ == kTypeMismatch;
  }

  friend constexpr bool IsNotFound(ReadResult read_result) {
    return read_result.value_ == kNotFound;
  }

 private:
  constexpr explicit ReadResult(ReadResultEnum value) : value_(value) {}
  ReadResultEnum value_;
};

constexpr inline ReadResult ReadFound() {
  return ReadResult(ReadResult::kFound);
}

constexpr inline ReadResult ReadNotFound() {
  return ReadResult(ReadResult::kNotFound);
}

constexpr inline ReadResult ReadTypeMismatch() {
  return ReadResult(ReadResult::kTypeMismatch);
}

// Returns a string_view of the string at 'idx' in the Lua stack.
// Prerequisite: `lua_type(L, idx) == LUA_TSTRING`.
inline absl::string_view RawStringRead(lua_State* L, int idx) {
  std::size_t length = 0;
  const char* result_cstr = lua_tolstring(L, idx, &length);
  return absl::string_view(result_cstr, length);
}

template <typename Type>
Type* ReadUDT(lua_State* L, int idx, const char* tname) {
  if (!lua_isuserdata(L, idx)) {
    return nullptr;
  }
  void* p = lua_touserdata(L, idx);
  Type* t = nullptr;

  if (lua_getmetatable(L, idx)) {
    lua_pushstring(L, tname);
    lua_gettable(L, LUA_REGISTRYINDEX);
    if (lua_rawequal(L, -1, -2)) {
      t = static_cast<Type*>(p);
    }
    lua_pop(L, 2);
  }

  return t;
}

// In all Read overloads, '*result' is filled in if value at the stack
// location 'idx' is valid for the given type. The return value indicates
// whether the read was valid. If the read fails, '*result' is unmodified.
inline ReadResult Read(lua_State* L, int idx, std::string* result) {
  switch (lua_type(L, idx)) {
    case LUA_TSTRING:
      *result = std::string(RawStringRead(L, idx));
      return ReadFound();
    case LUA_TNIL:
    case LUA_TNONE:
      return ReadNotFound();
    default:
      return ReadTypeMismatch();
  }
}

// Warning result's data is still owned by Lua's stack and maybe garbage
// collected after being removed from that stack.
inline ReadResult Read(lua_State* L, int idx, absl::string_view* result) {
  switch (lua_type(L, idx)) {
    case LUA_TSTRING:
      *result = RawStringRead(L, idx);
      return ReadFound();
    case LUA_TNIL:
    case LUA_TNONE:
      return ReadNotFound();
    default:
      return ReadTypeMismatch();
  }
}

inline ReadResult Read(lua_State* L, int idx, bool* result) {
  switch (lua_type(L, idx)) {
    case LUA_TBOOLEAN:
      *result = lua_toboolean(L, idx);
      return ReadFound();
    case LUA_TNIL:
    case LUA_TNONE:
      return ReadNotFound();
    default:
      return ReadTypeMismatch();
  }
}

inline ReadResult Read(lua_State* L, int idx, lua_Integer* result) {
  switch (lua_type(L, idx)) {
    case LUA_TNUMBER:
      *result = lua_tointeger(L, idx);
      return ReadFound();
    case LUA_TNIL:
    case LUA_TNONE:
      return ReadNotFound();
    default:
      return ReadTypeMismatch();
  }
}

inline ReadResult Read(lua_State* L, int idx, lua_Number* result) {
  switch (lua_type(L, idx)) {
    case LUA_TNUMBER:
      *result = lua_tonumber(L, idx);
      return ReadFound();
    case LUA_TNIL:
    case LUA_TNONE:
      return ReadNotFound();
    default:
      return ReadTypeMismatch();
  }
}

// Convenience wrapper for arbitrary signed integral types.
template <typename T>
typename std::enable_if<std::is_integral<T>::value && std::is_signed<T>::value,
                        ReadResult>::type
Read(lua_State* L, int idx, T* out) {
  lua_Integer result;
  ReadResult read_result = Read(L, idx, &result);
  if (IsFound(read_result)) {
    *out = result;
  }
  return read_result;
}

// Convenience wrapper for arbitrary unsigned integral types.
// Read fails if value is negative.
template <typename T>
typename std::enable_if<std::is_unsigned<T>::value &&
                            !std::is_same<T, bool>::value,
                        ReadResult>::type
Read(lua_State* L, int idx, T* out) {
  lua_Integer result;
  ReadResult read_result = Read(L, idx, &result);
  if (IsFound(read_result)) {
    if (result < 0) {
      return ReadTypeMismatch();
    } else {
      *out = result;
    }
  }
  return read_result;
}

// Convenience wrapper for arbitrary floating-point types.
template <typename T>
typename std::enable_if<std::is_floating_point<T>::value, ReadResult>::type
Read(lua_State* L, int idx, T* out) {
  lua_Number result;
  ReadResult read_result = Read(L, idx, &result);
  if (IsFound(read_result)) {
    *out = result;
  }
  return read_result;
}

inline ReadResult Read(lua_State* L, int idx, lua_CFunction* result) {
  switch (lua_type(L, idx)) {
    case LUA_TFUNCTION:
      *result = lua_tocfunction(L, idx);
      return ReadFound();
    case LUA_TNIL:
    case LUA_TNONE:
      return ReadNotFound();
    default:
      return ReadTypeMismatch();
  }
}

template <typename T>
ReadResult Read(lua_State* L, int idx, T** result) {
  switch (lua_type(L, idx)) {
    case LUA_TLIGHTUSERDATA:
      *result = static_cast<T*>(lua_touserdata(L, idx));
      return ReadFound();
    case LUA_TNIL:
    case LUA_TNONE:
      return ReadNotFound();
    default:
      return ReadTypeMismatch();
  }
}

// Reads an array from the Lua stack. On success, the array is stored in
// '*result'; on failure, '*result' is unmodified. Returns whether the function
// succeeds.
//
// The function fails if the value on the stack at position 'idx' is not a
// table, or if any array element of the table is not readable as type T.
// Non-array table elements are silently ignored.
template <typename T, typename A>
ReadResult Read(lua_State* L, int idx, std::vector<T, A>* result);

// Reads a Lua array into 'values'. The failure conditions are the same as in
// the previous function, but 'values' may be modified even if this function
// fails.
template <typename T>
ReadResult Read(lua_State* L, int idx, absl::Span<T> values);

// Reads a Lua array into '*values'. The failure conditions are the same as in
// the previous function, but '*values' may be modified even if this function
// fails.
template <typename T, std::size_t N>
ReadResult Read(lua_State* L, int idx, std::array<T, N>* values);

// Reads a table from the Lua stack. On success, the table is stored in
// '*result'; on failure, '*result' is unmodified. Returns whether the function
// succeeds.
//
// The function fails if the value on the stack at position 'idx' is not a
// table, or if the table contains an entry whose key cannot be read as K or
// whose value cannot be read as T.
template <typename K, typename T, typename H, typename C, typename A>
ReadResult Read(lua_State* L, int idx,
                absl::flat_hash_map<K, T, H, C, A>* result);

// Reads value from the Lua stack. On success, the varant stores the result of
// the value on the stack. The reads are attempted in the order that the types
// are presented in the variant.
template <typename... T>
ReadResult Read(lua_State* L, int idx, absl::variant<T...>* result);

template <typename T>
ReadResult Read(lua_State* L, int idx, absl::Span<T> values) {
  switch (lua_type(L, idx)) {
    case LUA_TTABLE:
      break;
    case LUA_TNIL:
    case LUA_TNONE:
      return ReadNotFound();
    default:
      return ReadTypeMismatch();
  }

  const std::size_t count = ArrayLength(L, idx);
  if (count < values.size()) {
    return ReadTypeMismatch();
  }

  for (std::size_t i = 0; i < values.size(); ++i) {
    lua_rawgeti(L, idx, i + 1);
    if (!IsFound(Read(L, -1, &values[i]))) {
      lua_pop(L, 1);
      return ReadTypeMismatch();
    }
    lua_pop(L, 1);
  }
  return ReadFound();
}

template <typename T, std::size_t N>
ReadResult Read(lua_State* L, int idx, std::array<T, N>* values) {
  return Read(L, idx, absl::MakeSpan(*values));
}

template <typename T, typename A>
ReadResult Read(lua_State* L, int idx, std::vector<T, A>* result) {
  std::vector<T, A> local_result;
  switch (lua_type(L, idx)) {
    case LUA_TTABLE:
      break;
    case LUA_TNIL:
    case LUA_TNONE:
      return ReadNotFound();
    default:
      return ReadTypeMismatch();
  }

  std::size_t count = ArrayLength(L, idx);
  local_result.reserve(count);
  for (std::size_t i = 0; i < count; ++i) {
    lua_rawgeti(L, idx, i + 1);
    T value;
    if (IsFound(Read(L, -1, &value))) {
      local_result.push_back(std::move(value));
    } else {
      lua_pop(L, 1);
      return ReadTypeMismatch();
    }
    lua_pop(L, 1);
  }
  result->swap(local_result);
  return ReadFound();
}

template <typename K, typename T, typename H, typename C, typename A>
ReadResult Read(lua_State* L, int idx,
                absl::flat_hash_map<K, T, H, C, A>* result) {
  absl::flat_hash_map<K, T, H, C, A> local_result;

  switch (lua_type(L, idx)) {
    case LUA_TTABLE:
      break;
    case LUA_TNIL:
    case LUA_TNONE:
      return ReadNotFound();
    default:
      return ReadTypeMismatch();
  }

  if (idx < 0) {
    idx = lua_gettop(L) + idx + 1;
  }
  lua_pushnil(L);
  while (lua_next(L, idx) != 0) {
    K key;
    if (!IsFound(Read(L, -2, &key))) {
      lua_pop(L, 2);
      return ReadTypeMismatch();
    }
    T value;
    if (!IsFound(Read(L, -1, &value))) {
      lua_pop(L, 2);
      return ReadTypeMismatch();
    }
    local_result.try_emplace(std::move(key), std::move(value));
    lua_pop(L, 1);
  }
  result->swap(local_result);
  return ReadFound();
}

namespace internal {

template <typename Variant, typename T>
ReadResult TryReadValueRecursive(lua_State* L, int idx, Variant* result) {
  return Read(L, idx, &result->template emplace<T>());
}

// Attempts to read value in Lua stack at index 'idx' into 'result' when
// 'result' is with variant of type 'T0'. Returns ReadFound if successful.
// Otherwise continue attempting to read value with remaining types.
template <typename Variant, typename T0, typename T1, typename... TN>
ReadResult TryReadValueRecursive(lua_State* L, int idx, Variant* result) {
  if (IsFound(TryReadValueRecursive<Variant, T0>(L, idx, result))) {
    return ReadFound();
  } else {
    return TryReadValueRecursive<Variant, T1, TN...>(L, idx, result);
  }
}

}  // namespace internal

template <typename... T>
ReadResult Read(lua_State* L, int idx, absl::variant<T...>* result) {
  switch (lua_type(L, idx)) {
    case LUA_TNONE:
    case LUA_TNIL:
      return ReadNotFound();
    default:
      return internal::TryReadValueRecursive<absl::variant<T...>, T...>(L, idx,
                                                                        result);
  }
}

// Coerce result into human readable string. (Does not fail.)
inline std::string ToString(lua_State* L, int idx) {
  std::stringstream ss;
  switch (lua_type(L, idx)) {
    case LUA_TNONE:
      ss << "(none)";
      break;
    case LUA_TNIL:
      ss << "(nil)";
      break;
    case LUA_TBOOLEAN:
      ss << (lua_toboolean(L, idx) ? "true" : "false");
      break;
    case LUA_TLIGHTUSERDATA:
      ss << "pointer [" << lua_touserdata(L, idx) << "]";
      break;
    case LUA_TNUMBER:
      ss << lua_tonumber(L, idx);
      break;
    case LUA_TSTRING:
      ss << RawStringRead(L, idx);
      break;
    case LUA_TTABLE:
      ss << "(table)";
      break;
    case LUA_TFUNCTION:
      ss << "function [" << lua_tocfunction(L, idx) << "]";
      break;
    case LUA_TUSERDATA: {
      ss << "user pointer: [" << lua_touserdata(L, idx) << "]";
      int top = lua_gettop(L);
      if (luaL_callmeta(L, idx, "__tostring") && lua_isstring(L, -1)) {
        ss << RawStringRead(L, -1);
      }
      lua_settop(L, top);
      break;
    }
    default:
      ss << "(unknown)";
      break;
  }
  return ss.str();
}

}  // namespace lua
}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_LUA_READ_H_
