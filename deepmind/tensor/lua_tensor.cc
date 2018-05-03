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

#include "deepmind/tensor/lua_tensor.h"

#include <iostream>

#include "deepmind/lua/bind.h"
#include "deepmind/lua/table_ref.h"
#include "public/file_reader_types.h"

namespace deepmind {
namespace lab {
namespace tensor {

int LuaTensorConstructors(lua_State* L) {
  lua::TableRef table = lua::TableRef::Create(L);
  void* fs = nullptr;
  lua::Read(L, lua_upvalueindex(1) , &fs);
  auto table_insert = [L, &table, fs](const char* name, lua_CFunction fn) {
    lua_pushlightuserdata(L, fs);
    lua_pushcclosure(L, fn, 1);
    table.InsertFromStackTop(name);
  };

  table_insert("ByteTensor", &lua::Bind<LuaTensor<std::uint8_t>::Create>);
  table_insert("CharTensor", &lua::Bind<LuaTensor<std::int8_t>::Create>);
  table_insert("Int16Tensor", &lua::Bind<LuaTensor<std::int16_t>::Create>);
  table_insert("Int32Tensor", &lua::Bind<LuaTensor<std::int32_t>::Create>);
  table_insert("Int64Tensor", &lua::Bind<LuaTensor<std::int64_t>::Create>);
  table_insert("FloatTensor", &lua::Bind<LuaTensor<float>::Create>);
  table_insert("DoubleTensor", &lua::Bind<LuaTensor<double>::Create>);
  table_insert("Tensor", &lua::Bind<LuaTensor<double>::Create>);
  lua::Push(L, table);
  return 1;
}

void LuaTensorRegister(lua_State* L) {
  LuaTensor<std::uint8_t>::Register(L);
  LuaTensor<std::int8_t>::Register(L);
  LuaTensor<std::int16_t>::Register(L);
  LuaTensor<std::int32_t>::Register(L);
  LuaTensor<std::int64_t>::Register(L);
  LuaTensor<float>::Register(L);
  LuaTensor<double>::Register(L);
}

}  // namespace tensor
}  // namespace lab
}  // namespace deepmind
