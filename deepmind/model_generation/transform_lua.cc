// Copyright (C) 2017 Google Inc.
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

#include "deepmind/model_generation/transform_lua.h"

#include "deepmind/support/logging.h"
#include "deepmind/tensor/lua_tensor.h"
#include "deepmind/tensor/tensor_view.h"

namespace deepmind {
namespace lab {

void Push(lua_State* L, const Transform& xfrm) {
  auto storage = std::make_shared<tensor::StorageVector<Transform>>(1, xfrm);
  tensor::LuaTensor<float>::CreateObject(
      L,
      tensor::TensorView<float>(tensor::Layout({4, 4}),
                                storage->mutable_data()->front().data()),
      storage);
}

bool Read(lua_State* L, int idx, Transform* xfrm) {
  auto* tensor = tensor::LuaTensor<float>::ReadObject(L, idx);
  if (tensor == nullptr) {
    return false;
  }
  const auto& view = tensor->tensor_view();
  const auto& shape = view.shape();
  if (shape.size() != 2 || shape[0] != 4 || shape[1] != 4 ||
      !view.IsContiguous()) {
    LOG(ERROR) << "Incorrect dimensions for arg 'xfrm'";
    return false;
  }
  const float* storage = view.storage();
  view.ForEachOffset(
      [=](std::size_t offset) { xfrm->data()[offset] = storage[offset]; });
  return true;
}

}  // namespace lab
}  // namespace deepmind
