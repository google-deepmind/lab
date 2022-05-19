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

#include "deepmind/model_generation/model_getters.h"

#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/log/check.h"
#include "deepmind/include/deepmind_model_getters.h"
#include "deepmind/model_generation/model.h"
#include "deepmind/model_generation/transform.h"

namespace deepmind {
namespace lab {
namespace {

const Model* CastModel(const void* model_data) {
  CHECK(model_data != nullptr);
  return static_cast<const Model*>(model_data);
}

void GetName(const void* model_data, std::size_t max_length, char* name) {
  name[CastModel(model_data)->name.copy(name, max_length - 1)] = '\0';
}

std::size_t GetSurfaceCount(const void* model_data) {
  return CastModel(model_data)->surfaces.size();
}

void GetSurfaceName(const void* model_data, std::size_t surf_idx,
                    std::size_t max_length, char* name) {
  const Model& model = *CastModel(model_data);
  CHECK_LT(surf_idx, model.surfaces.size()) << "Incorrect surface index.";
  CHECK_GT(max_length, 0) << "name buffer must have positive length.";
  name[model.surfaces[surf_idx].name.copy(name, max_length - 1)] = '\0';
}

std::size_t GetSurfaceVertexCount(const void* model_data,
                                  std::size_t surf_idx) {
  const Model& model = *CastModel(model_data);
  CHECK_LT(surf_idx, model.surfaces.size()) << "Incorrect surface index.";
  return model.surfaces[surf_idx].vertices.size() / 8;
}

void GetSurfaceVertexLocation(const void* model_data, std::size_t surf_idx,
                              std::size_t vert_idx, float location[3]) {
  const Model& model = *CastModel(model_data);
  CHECK_LT(surf_idx, model.surfaces.size()) << "Incorrect surface index.";
  const Model::Surface& surf = model.surfaces[surf_idx];
  CHECK_LT(vert_idx, surf.vertices.size() / 8) << "Incorrect vertex index.";
  const float* vertex = &surf.vertices[8 * vert_idx];
  for (int i = 0; i < 3; ++i) {
    location[i] = vertex[i];
  }
}

void GetSurfaceVertexNormal(const void* model_data, std::size_t surf_idx,
                            std::size_t vert_idx, float normal[3]) {
  const Model& model = *CastModel(model_data);
  CHECK_LT(surf_idx, model.surfaces.size()) << "Incorrect surface index.";
  const Model::Surface& surf = model.surfaces[surf_idx];
  CHECK_LT(vert_idx, surf.vertices.size() / 8) << "Incorrect vertex index.";
  const float* vertex = &surf.vertices[8 * vert_idx];
  for (int i = 0; i < 3; ++i) {
    normal[i] = vertex[3 + i];
  }
}

void GetSurfaceVertexST(const void* model_data, std::size_t surf_idx,
                        std::size_t vert_idx, float st[2]) {
  const Model& model = *CastModel(model_data);
  CHECK_LT(surf_idx, model.surfaces.size()) << "Incorrect surface index.";
  const Model::Surface& surf = model.surfaces[surf_idx];
  CHECK_LT(vert_idx, surf.vertices.size() / 8) << "Incorrect vertex index.";
  const float* vertex = &surf.vertices[8 * vert_idx];
  for (int i = 0; i < 2; ++i) {
    st[i] = vertex[6 + i];
  }
}

std::size_t GetSurfaceFaceCount(const void* model_data, std::size_t surf_idx) {
  const Model& model = *CastModel(model_data);
  CHECK_LT(surf_idx, model.surfaces.size()) << "Incorrect surface index.";
  return model.surfaces[surf_idx].indices.size() / 3;
}

void GetSurfaceFace(const void* model_data, std::size_t surf_idx,
                    std::size_t face_idx, int indices[3]) {
  const Model& model = *CastModel(model_data);
  CHECK_LT(surf_idx, model.surfaces.size()) << "Incorrect surface index.";
  const Model::Surface& surf = model.surfaces[surf_idx];
  CHECK_LT(face_idx, surf.indices.size() / 3) << "Incorrect face index.";
  const int* face = &surf.indices[3 * face_idx];
  for (int i = 0; i < 3; ++i) {
    indices[i] = face[i];
  }
}

std::size_t GetSurfaceShaderCount(const void* model_data,
                                  std::size_t surf_idx) {
  return 1;
}

void GetSurfaceShader(const void* model_data, std::size_t surf_idx,
                      std::size_t shad_idx, std::size_t max_length,
                      char* name) {
  const Model& model = *CastModel(model_data);
  CHECK_LT(surf_idx, model.surfaces.size()) << "Incorrect surface index.";
  CHECK_GT(max_length, 0) << "name buffer must have positive length.";
  name[model.surfaces[surf_idx].shader_name.copy(name, max_length - 1)] = '\0';
}

std::size_t GetTagCount(const void* model_data) {
  return CastModel(model_data)->locators.size();
}

void GetTagName(const void* model_data, std::size_t tag_idx,
                std::size_t max_length, char* name) {
  const Model& model = *CastModel(model_data);
  CHECK_LT(tag_idx, model.locators.size()) << "Incorrect tag index.";
  Model::LocatorMap::const_iterator it = model.locators.cbegin();
  for (std::size_t i = 0; i < tag_idx; ++i) {
    ++it;
  }
  CHECK_GT(max_length, 0) << "name buffer must have positive length.";
  name[it->first.copy(name, max_length - 1)] = '\0';
}

void GetTagAxis(const void* model_data, std::size_t tag_idx,
                std::size_t axis_idx, float axis[3]) {
  const Model& model = *CastModel(model_data);
  CHECK_LT(tag_idx, model.locators.size()) << "Incorrect tag index.";
  Model::LocatorMap::const_iterator it = model.locators.cbegin();
  for (std::size_t i = 0; i < tag_idx; ++i) {
    ++it;
  }
  const Transform& xfrm = it->second;
  for (int i = 0; i < 3; ++i) {
    axis[i] = xfrm(i, axis_idx);
  }
}

void GetTagOrigin(const void* model_data, std::size_t tag_idx,
                  float origin[3]) {
  const Model& model = *CastModel(model_data);
  CHECK_LT(tag_idx, model.locators.size()) << "Incorrect tag index.";
  Model::LocatorMap::const_iterator it = model.locators.cbegin();
  for (std::size_t i = 0; i < tag_idx; ++i) {
    ++it;
  }
  const Transform& xfrm = it->second;
  for (int i = 0; i < 3; ++i) {
    origin[i] = xfrm(i, 3);
  }
}

}  // namespace

DeepmindModelGetters ModelGetters() {
  return {
      GetName,                   //
      GetSurfaceCount,           //
      GetSurfaceName,            //
      GetSurfaceVertexCount,     //
      GetSurfaceVertexLocation,  //
      GetSurfaceVertexNormal,    //
      GetSurfaceVertexST,        //
      GetSurfaceFaceCount,       //
      GetSurfaceFace,            //
      GetSurfaceShaderCount,     //
      GetSurfaceShader,          //
      GetTagCount,               //
      GetTagName,                //
      GetTagAxis,                //
      GetTagOrigin               //
  };
}

}  // namespace lab
}  // namespace deepmind
