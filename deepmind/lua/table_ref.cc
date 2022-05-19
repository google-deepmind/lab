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

#include "deepmind/lua/table_ref.h"

#include "absl/log/check.h"
#include "deepmind/lua/lua.h"
#include "deepmind/lua/read.h"

namespace deepmind {
namespace lab {
namespace lua {

TableRef::~TableRef() {
  if (lua_state_) {
    luaL_unref(lua_state_, LUA_REGISTRYINDEX, table_reference_);
  }
}

TableRef::TableRef() : TableRef(nullptr, 0) {}

// Private. 'table_reference' is now owned by this object.
TableRef::TableRef(lua_State* L, int table_reference)
    : lua_state_(L), table_reference_(table_reference) {}

TableRef::TableRef(TableRef&& other) noexcept
    : lua_state_(other.lua_state_), table_reference_(other.table_reference_) {
  other.lua_state_ = nullptr;
  other.table_reference_ = 0;
}

TableRef::TableRef(const TableRef& other)
    : lua_state_(other.lua_state_), table_reference_(0) {
  // Create our own internal reference.
  if (other.lua_state_) {
    other.PushTable();
    table_reference_ = luaL_ref(lua_state_, LUA_REGISTRYINDEX);
  }
}

bool TableRef::operator==(const TableRef& rhs) const {
  if (lua_state_ != rhs.lua_state_) {
    return false;
  } else if (lua_state_ == nullptr) {
    return true;
  } else if (table_reference_ == rhs.table_reference_) {
    return true;
  } else {
    PushTable();
    rhs.PushTable();
    bool equal = lua_rawequal(lua_state_, -1, -2);
    lua_pop(lua_state_, 2);
    return equal;
  }
}

std::size_t TableRef::ArraySize() const {
  PushTable();
  std::size_t count = ArrayLength(lua_state_, -1);
  lua_pop(lua_state_, 1);
  return count;
}

std::size_t TableRef::KeyCount() const {
  std::size_t result = 0;
  PushTable();
  if (lua_type(lua_state_, -1) == LUA_TUSERDATA) {
    if (lua_getmetatable(lua_state_, -1)) {
      lua_remove(lua_state_, -2);
    }
  }

  if (lua_type(lua_state_, -1) != LUA_TTABLE) {
    return 0;
  }

  lua_pushnil(lua_state_);
  while (lua_next(lua_state_, -2) != 0) {
    ++result;
    lua_pop(lua_state_, 1);
  }
  lua_pop(lua_state_, 1);
  return result;
}

TableRef& TableRef::operator=(TableRef other) {
  this->swap(other);
  return *this;
}

TableRef TableRef::Create(lua_State* L) {
  CHECK(L != nullptr) << "Creating a table with a null State.";
  lua_createtable(L, 0, 0);
  return TableRef(L, luaL_ref(L, LUA_REGISTRYINDEX));
}

void TableRef::PushTable() const {
  CHECK(!is_unbound()) << "Unbound TableRef";
  lua_rawgeti(lua_state_, LUA_REGISTRYINDEX, table_reference_);
}

void Push(lua_State* L, const TableRef& table) { table.PushTable(); }

ReadResult Read(lua_State* L, int idx, TableRef* table) {
  switch (lua_type(L, idx)) {
    case LUA_TTABLE:
    case LUA_TUSERDATA:
      lua_pushvalue(L, idx);
      *table = TableRef(L, luaL_ref(L, LUA_REGISTRYINDEX));
      return ReadFound();
    case LUA_TNIL:
    case LUA_TNONE:
      return ReadNotFound();
    default:
      return ReadTypeMismatch();
  }
}

}  // namespace lua
}  // namespace lab
}  // namespace deepmind
