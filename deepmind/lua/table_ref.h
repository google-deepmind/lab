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

#ifndef DML_DEEPMIND_LUA_TABLE_REF_H_
#define DML_DEEPMIND_LUA_TABLE_REF_H_

#include <utility>
#include <vector>

#include "deepmind/lua/push.h"
#include "deepmind/lua/read.h"

namespace deepmind {
namespace lab {
namespace lua {

class TableRef;

// Returns whether item at idx is a Lua table or userdata. If true '*table' is
// assigned a reference to it.
// [0, 0, -]
ReadResult Read(lua_State* L, int idx, TableRef* table);

// Push the table refered to by 'table' onto the stack.
// [1, 0, -] - Precondition: !table.is_unbound().
void Push(lua_State* L, const TableRef& table);

// An object of type TableRef, when bound, stores a reference to a table or
// userdata in the Lua registry. It releases that reference on destruction.
// When unbound it does not refer to or release anything.
class TableRef final {
 public:
  // Creates an new, empty table and returns a reference to it.
  // [0, 0, -]
  static TableRef Create(lua_State* L);

  // Creates an unbound reference.
  TableRef();

  // Transfers the reference held by other; other is left in an unbound state.
  TableRef(TableRef&& other) noexcept;

  // Creates a reference to the table referenced by other, if any.
  TableRef(const TableRef& other);

  // Equivalent to other.swap(*this).
  TableRef& operator=(TableRef other);

  // See class documentation above.
  ~TableRef();

  // Returns whether the two underlying tables are the same ("shallow
  // comparison"; does not compare contents).
  bool operator==(const TableRef& rhs) const;

  bool operator!=(const TableRef& rhs) const { return !(*this == rhs); }

  // Returns whether *this is unbound.
  bool is_unbound() const { return lua_state_ == nullptr; }

  void swap(TableRef& t1) {
    std::swap(lua_state_, t1.lua_state_);
    std::swap(table_reference_, t1.table_reference_);
  }

  // Size of the array portion of a Lua table. Equivalent to the length operator
  // (#) in Lua.
  // [0, 0, -] - Precondition: !this->is_unbound().
  std::size_t ArraySize() const;

  // Iterates the table to count the number of keys (slow).
  // If bound to a 'userdata' is counts keys in the first meta-table.
  // [0, 0, -] - Precondition: !this->is_unbound().
  std::size_t KeyCount() const;

  // Returns whether key is present.
  // [0, 0, -] - Precondition: !this->is_unbound().
  template <typename K>
  bool Contains(const K& key) const {
    PushTable();
    Push(lua_state_, key);
    lua_gettable(lua_state_, -2);
    bool is_unbound = lua_isnil(lua_state_, -1);
    lua_pop(lua_state_, 2);
    return !is_unbound;
  }

  // Returns all keys that can be Read with type K.
  // If 'userdata' iterates first meta-table.
  // [0, 0, -] - Precondition: !this->is_unbound().
  template <typename K>
  std::vector<K> Keys() const {
    std::vector<K> result;
    PushTable();
    if (lua_type(lua_state_, -1) == LUA_TUSERDATA) {
      if (lua_getmetatable(lua_state_, -1)) {
        lua_remove(lua_state_, -2);
      }
    }

    if (lua_type(lua_state_, -1) != LUA_TTABLE) {
      return result;
    }

    lua_pushnil(lua_state_);
    while (lua_next(lua_state_, -2) != 0) {
      K key;
      if (IsFound(Read(lua_state_, -2, &key))) {
        result.push_back(std::move(key));
      }
      lua_pop(lua_state_, 1);
    }
    lua_pop(lua_state_, 1);
    return result;
  }

  // If the table contains an entry of type T for index key, sets *value to
  // table[key] and returns whether the key was found and whether it is of the
  // correct type.
  // [0, 0, -] - Precondition: !this->is_unbound().
  template <typename K, typename T>
  ReadResult LookUp(const K& key, T value) const {
    PushTable();
    Push(lua_state_, key);
    lua_gettable(lua_state_, -2);
    auto read_result = Read(lua_state_, -1, value);
    lua_pop(lua_state_, 2);
    return read_result;
  }

  // Pushes the value as table[key] on to the stack.
  // If key is not there then it retrieves nil.
  // [0, +1, -] - Precondition: !this->is_unbound().
  template <typename K>
  void LookUpToStack(const K& key) const {
    PushTable();
    int table_idx = lua_gettop(lua_state_);
    Push(lua_state_, key);
    lua_gettable(lua_state_, -2);
    lua_remove(lua_state_, table_idx);
  }

  // Syntactic sugar for LookUpToStack.
  // [0, +1, -] - Precondition: !this->is_unbound().
  template <typename K>
  void PushFunction(const K& key) const {
    LookUpToStack(key);
  }

  // Adds to the stack [table[k], table]. Remember to add 1 to nargs of Call
  // to account for the table.
  // [0, +2, -] - Precondition: !this->is_unbound().
  template <typename K>
  void PushMemberFunction(const K& key) const {
    PushTable();
    int table_idx = lua_gettop(lua_state_);
    Push(lua_state_, key);
    lua_gettable(lua_state_, -2);
    lua_pushvalue(lua_state_, table_idx);
    lua_remove(lua_state_, table_idx);
  }

  // [0, 0, -] - Precondition: !this->is_unbound().
  template <typename K, typename T>
  void Insert(const K& key, const T& value) {
    PushTable();
    Push(lua_state_, key);
    Push(lua_state_, value);
    lua_settable(lua_state_, -3);
    lua_pop(lua_state_, 1);
  }

  // Pops the value on top of the stack and stores it into table[key].
  // [0, -1, -] - Precondition: !this->is_unbound().
  template <typename K>
  void InsertFromStackTop(const K& key) const {
    PushTable();
    Push(lua_state_, key);
    lua_pushvalue(lua_state_, -3);
    lua_settable(lua_state_, -3);
    lua_pop(lua_state_, 2);
  }

  // [0, 0, -] - Precondition: !this->is_unbound().
  template <typename K>
  TableRef CreateSubTable(const K& key) {
    TableRef subtable = Create(lua_state_);
    Insert(key, subtable);
    return subtable;
  }

  // Gets the internal Lua state.
  lua_State* LuaState() { return lua_state_; }

 private:
  friend ReadResult Read(lua_State* L, int idx, TableRef* table);
  friend void Push(lua_State* L, const TableRef& table);

  // Pushes a copy of the table referenced by *this onto the Lua stack.
  // [0, +1, -] - Precondition: !this->is_unbound()
  void PushTable() const;

  TableRef(lua_State* L, int table_reference);
  lua_State* lua_state_;
  int table_reference_;
};

}  // namespace lua
}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_LUA_TABLE_REF_H_
