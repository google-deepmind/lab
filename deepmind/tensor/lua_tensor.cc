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

namespace deepmind {
namespace lab {
namespace tensor {

int LuaTensorConstructors(lua_State* L) {
  lua::TableRef table = lua::TableRef::Create(L);
  table.Insert("ByteTensor", &lua::Bind<LuaTensor<std::uint8_t>::Create>);
  table.Insert("CharTensor", &lua::Bind<LuaTensor<std::int8_t>::Create>);
  table.Insert("Int16Tensor", &lua::Bind<LuaTensor<std::int16_t>::Create>);
  table.Insert("Int32Tensor", &lua::Bind<LuaTensor<std::int32_t>::Create>);
  table.Insert("Int64Tensor", &lua::Bind<LuaTensor<std::int64_t>::Create>);
  table.Insert("FloatTensor", &lua::Bind<LuaTensor<float>::Create>);
  table.Insert("DoubleTensor", &lua::Bind<LuaTensor<double>::Create>);
  table.Insert("Tensor", &lua::Bind<LuaTensor<double>::Create>);
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
