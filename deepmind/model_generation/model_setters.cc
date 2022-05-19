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

#include "deepmind/model_generation/model_setters.h"

#include <string>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/log/check.h"
#include "absl/log/log.h"
#include "deepmind/include/deepmind_model_setters.h"
#include "deepmind/model_generation/model.h"
#include "deepmind/model_generation/transform.h"

namespace deepmind {
namespace lab {
namespace {

ModelSettersData* CastModelData(void* model_data) {
  CHECK(model_data != nullptr);
  return static_cast<ModelSettersData*>(model_data);
}

void SetName(void* model_data, const char* name) {
  CastModelData(model_data)->model.name = name;
}

void SetSurfaceCount(void* model_data, std::size_t num_surfaces) {
  CastModelData(model_data)->model.surfaces.resize(num_surfaces);
}

void SetSurfaceName(void* model_data, std::size_t surf_idx, const char* name) {
  Model& model = CastModelData(model_data)->model;
  CHECK_LT(surf_idx, model.surfaces.size()) << "Incorrect surface index.";
  model.surfaces[surf_idx].name = name;
}

void SetSurfaceVertexCount(void* model_data, std::size_t surf_idx,
                           std::size_t num_verts) {
  Model& model = CastModelData(model_data)->model;
  CHECK_LT(surf_idx, model.surfaces.size()) << "Incorrect surface index.";
  model.surfaces[surf_idx].vertices.resize(num_verts * 8);
}

void SetSurfaceVertexLocation(void* model_data, std::size_t surf_idx,
                              std::size_t vert_idx, float location[3]) {
  Model& model = CastModelData(model_data)->model;
  CHECK_LT(surf_idx, model.surfaces.size()) << "Incorrect surface index.";
  Model::Surface& surf = model.surfaces[surf_idx];
  CHECK_LT(vert_idx, surf.vertices.size() / 8) << "Incorrect vertex index.";
  float* vertex = &surf.vertices[vert_idx * 8];
  for (int i = 0; i < 3; ++i) {
    vertex[i] = location[i];
  }
}

void SetSurfaceVertexNormal(void* model_data, std::size_t surf_idx,
                            std::size_t vert_idx, float normal[3]) {
  Model& model = CastModelData(model_data)->model;
  CHECK_LT(surf_idx, model.surfaces.size()) << "Incorrect surface index.";
  Model::Surface& surf = model.surfaces[surf_idx];
  CHECK_LT(vert_idx, surf.vertices.size() / 8) << "Incorrect vertex index.";
  float* vertex = &surf.vertices[vert_idx * 8];
  for (int i = 0; i < 3; ++i) {
    vertex[3 + i] = normal[i];
  }
}

void SetSurfaceVertexST(void* model_data, std::size_t surf_idx,
                        std::size_t vert_idx, float st[2]) {
  Model& model = CastModelData(model_data)->model;
  CHECK_LT(surf_idx, model.surfaces.size()) << "Incorrect surface index.";
  Model::Surface& surf = model.surfaces[surf_idx];
  CHECK_LT(vert_idx, surf.vertices.size() / 8) << "Incorrect vertex index.";
  float* vertex = &surf.vertices[vert_idx * 8];
  for (int i = 0; i < 2; ++i) {
    vertex[6 + i] = st[i];
  }
}

void SetSurfaceFaceCount(void* model_data, std::size_t surf_idx,
                         std::size_t num_faces) {
  Model& model = CastModelData(model_data)->model;
  CHECK_LT(surf_idx, model.surfaces.size()) << "Incorrect surface index.";
  model.surfaces[surf_idx].indices.resize(num_faces * 3);
}

void SetSurfaceFace(void* model_data, std::size_t surf_idx,
                    std::size_t face_idx, int indices[3]) {
  Model& model = CastModelData(model_data)->model;
  CHECK_LT(surf_idx, model.surfaces.size()) << "Incorrect surface index.";
  Model::Surface& surf = model.surfaces[surf_idx];
  CHECK_LT(face_idx, surf.indices.size() / 3) << "Incorrect face index.";
  int* face = &surf.indices[face_idx * 3];
  for (int i = 0; i < 3; ++i) {
    face[i] = indices[i];
  }
}

void SetSurfaceShaderCount(void* model_data, std::size_t surf_idx,
                           std::size_t num_shaders) {
  Model& model = CastModelData(model_data)->model;
  CHECK_LT(surf_idx, model.surfaces.size()) << "Incorrect surface index.";
  if (num_shaders > 0) {
    LOG_IF(WARNING, num_shaders > 1)
        << "Surface specifies " << num_shaders
        << "shaders, only the 1st one will be used.";
  } else {
    model.surfaces[surf_idx].shader_name = "default";
  }
}

void SetSurfaceShader(void* model_data, std::size_t surf_idx,
                      std::size_t shad_idx, const char* name) {
  Model& model = CastModelData(model_data)->model;
  CHECK_LT(surf_idx, model.surfaces.size()) << "Incorrect surface index.";
  if (shad_idx == 0) {
    model.surfaces[surf_idx].shader_name = name;
  }
}

void SetTagCount(void* model_data, std::size_t num_tags) {
  CastModelData(model_data)->locatorNames.reserve(num_tags);
}

void SetTagName(void* model_data, std::size_t tag_idx, const char* name) {
  ModelSettersData& data = *CastModelData(model_data);
  CHECK_EQ(tag_idx, data.locatorNames.size()) << "Incorrect tag index.";
  data.locatorNames.push_back(name);
}

void SetTagAxis(void* model_data, std::size_t tag_idx, std::size_t axis_idx,
                float axis[3]) {
  ModelSettersData& data = *CastModelData(model_data);
  CHECK_LT(tag_idx, data.locatorNames.size()) << "Incorrect tag index.";
  Transform& xfrm = data.model.locators[data.locatorNames[tag_idx]];
  for (int i = 0; i < 3; ++i) {
    xfrm(i, axis_idx) = axis[i];
  }
  xfrm(3, axis_idx) = 0.0f;
}

void SetTagOrigin(void* model_data, std::size_t tag_idx, float origin[3]) {
  ModelSettersData& data = *CastModelData(model_data);
  CHECK_LT(tag_idx, data.locatorNames.size()) << "Incorrect tag index.";
  Transform& xfrm = data.model.locators[data.locatorNames[tag_idx]];
  for (int i = 0; i < 3; ++i) {
    xfrm(i, 3) = origin[i];
  }
  xfrm(3, 3) = 1.0f;
}

}  // namespace

DeepmindModelSetters ModelSetters() {
  return {
      SetName,                   //
      SetSurfaceCount,           //
      SetSurfaceName,            //
      SetSurfaceVertexCount,     //
      SetSurfaceVertexLocation,  //
      SetSurfaceVertexNormal,    //
      SetSurfaceVertexST,        //
      SetSurfaceFaceCount,       //
      SetSurfaceFace,            //
      SetSurfaceShaderCount,     //
      SetSurfaceShader,          //
      SetTagCount,               //
      SetTagName,                //
      SetTagAxis,                //
      SetTagOrigin               //
  };
}

}  // namespace lab
}  // namespace deepmind
