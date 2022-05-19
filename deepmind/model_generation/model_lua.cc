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

#include "deepmind/model_generation/model_lua.h"

#include <algorithm>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include "absl/log/log.h"
#include "deepmind/lua/push.h"
#include "deepmind/lua/read.h"
#include "deepmind/lua/table_ref.h"
#include "deepmind/model_generation/model.h"
#include "deepmind/model_generation/transform.h"
#include "deepmind/model_generation/transform_lua.h"
#include "deepmind/tensor/lua_tensor.h"
#include "deepmind/tensor/tensor_view.h"

namespace deepmind {
namespace lab {

void Push(lua_State* L, const Model& model) {
  lua::TableRef res = lua::TableRef::Create(L);
  res.Insert("name", model.name);
  lua::TableRef surfaces = res.CreateSubTable("surfaces");
  for (const Model::Surface& surf : model.surfaces) {
    // Save each surface.
    lua::TableRef surface = surfaces.CreateSubTable(surf.name);
    surface.Insert("name", surf.name);
    tensor::ShapeVector vertices_shape = {surf.vertices.size() / 8, 8};
    tensor::LuaTensor<float>::CreateObject(L, std::move(vertices_shape),
                                           surf.vertices);
    surface.InsertFromStackTop("vertices");
    tensor::ShapeVector indices_shape = {surf.indices.size() / 3, 3};
    auto* indices = tensor::LuaTensor<int>::CreateObject(
        L, std::move(indices_shape), surf.indices);
    // Translate C++ indices (base 0) to Lua (base 1).
    indices->mutable_tensor_view()->ForEachMutable([](int* val) { ++*val; });
    surface.InsertFromStackTop("indices");
    surface.Insert("shaderName", surf.shader_name);
  }
  res.Insert("locators", model.locators);
  lua::Push(L, res);
}

lua::ReadResult Read(lua_State* L, int idx, Model* model) {
  lua::TableRef table;
  lua::ReadResult read_result = lua::Read(L, idx, &table);
  if (!IsFound(read_result)) {
    LOG(ERROR) << "Failed to read model table";
    return read_result;
  }
  if (IsTypeMismatch(table.LookUp("name", &model->name))) {
    LOG(ERROR) << "Failed to read model name";
    return lua::ReadTypeMismatch();
  }
  lua::TableRef surfaces;
  if (!IsFound(table.LookUp("surfaces", &surfaces))) {
    LOG(ERROR) << "'surfaces' must be a table.";
    return lua::ReadTypeMismatch();
  }
  for (const auto& surface_name : surfaces.Keys<std::string>()) {
    // Read each surface.
    lua::TableRef surface;
    if (!IsFound(surfaces.LookUp(surface_name, &surface))) {
      LOG(ERROR) << "Failed to read surface table";
      return lua::ReadTypeMismatch();
    }
    Model::Surface model_surface;
    model_surface.name = surface_name;
    if (surface.LookUp("shaderName", &model_surface.shader_name) !=
        lua::ReadFound()) {
      LOG(ERROR) << "Bad or missing arg 'surfaces.shader_name'";
      return lua::ReadTypeMismatch();
    }
    // Read vertices.
    surface.LookUpToStack("vertices");
    auto* vertices = tensor::LuaTensor<float>::ReadObject(L, -1);
    if (vertices == nullptr) {
      LOG(ERROR) << "Bad or missing arg 'surfaces.vertices'";
      lua_pop(L, 1);
      return lua::ReadTypeMismatch();
    }
    if (vertices->tensor_view().shape().size() != 2 ||
        vertices->tensor_view().shape()[1] != 8) {
      LOG(ERROR) << "Incorrect dimensions for arg 'surfaces.vertices'";
      lua_pop(L, 1);
      return lua::ReadTypeMismatch();
    }
    model_surface.vertices.reserve(vertices->tensor_view().shape()[0] * 8);
    auto& surface_vertices = model_surface.vertices;
    vertices->tensor_view().ForEach(
        [&surface_vertices](float val) { surface_vertices.push_back(val); });
    lua_pop(L, 1);
    // Read indices.
    surface.LookUpToStack("indices");
    auto* indices = tensor::LuaTensor<std::int32_t>::ReadObject(L, -1);
    if (indices == nullptr) {
      LOG(ERROR) << "Bad or missing arg 'surfaces.indices'";
      lua_pop(L, 1);
      return lua::ReadTypeMismatch();
    }
    if (indices->tensor_view().shape().size() != 2 ||
        indices->tensor_view().shape()[1] != 3) {
      LOG(ERROR) << "Incorrect dimensions for arg 'surfaces.indices'";
      lua_pop(L, 1);
      return lua::ReadTypeMismatch();
    }
    model_surface.indices.reserve(indices->tensor_view().shape()[0] * 3);
    auto& surface_indices = model_surface.indices;
    int min_idx = std::numeric_limits<int>::max();
    int max_idx = std::numeric_limits<int>::min();
    indices->tensor_view().ForEach(
        [&surface_indices, &min_idx, &max_idx](int val) {
          min_idx = std::min(min_idx, val);
          max_idx = std::max(max_idx, val);
          // Translate Lua indices (base 1) to C++ indices (base 0).
          surface_indices.push_back(val - 1);
        });
    if (min_idx < 1 || max_idx > model_surface.vertices.size() / 8) {
      LOG(ERROR) << "Found vertex index in 'surfaces.indices' outside the "
                    "expected range [1, "
                 << model_surface.vertices.size() << "]";
      lua_pop(L, 1);
      return lua::ReadTypeMismatch();
    }
    model->surfaces.emplace_back(std::move(model_surface));
    lua_pop(L, 1);
  }

  if (IsTypeMismatch(table.LookUp("locators", &model->locators))) {
    LOG(ERROR) << "Failed to read locators";
    return lua::ReadTypeMismatch();
  }
  return lua::ReadFound();
}

}  // namespace lab
}  // namespace deepmind
