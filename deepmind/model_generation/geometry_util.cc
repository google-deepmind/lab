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

#include "deepmind/model_generation/geometry_util.h"

#include <array>
#include <functional>
#include <string>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/log/check.h"
#include "absl/strings/str_cat.h"
#include "deepmind/model_generation/transform.h"

namespace deepmind {
namespace lab {
namespace geometry {

Eigen::Affine3f CreateZAlignedFrame(  //
    const Eigen::Vector3f& trans,     //
    const Eigen::Vector3f& z_dir,     //
    const Eigen::Vector3f& y_vector) {
  Eigen::Vector3f z_axis = z_dir.normalized();
  Eigen::Vector3f x_axis = y_vector.cross(z_axis);
  float cross_norm = x_axis.norm();
  CHECK_GT(cross_norm, kEpsilon)
      << "z_dir " << z_dir << " is nearly parallel to y_vector " << y_vector;
  x_axis /= cross_norm;
  Eigen::Vector3f y_axis = z_axis.cross(x_axis);
  Eigen::Matrix4f mat;
  mat << x_axis, y_axis, z_axis, trans, 0.0f, 0.0f, 0.0f, 1.0f;
  return Eigen::Affine3f(mat);
}

void ComputeDiskMeshSize(       //
    std::size_t num_sectors,    //
    std::size_t num_tracks,     //
    std::size_t* num_vertices,  //
    std::size_t* num_triangles) {
  *num_vertices = (num_tracks + 1) * (num_sectors + 1) - 1;
  *num_triangles = (num_tracks * 2 - 1) * num_sectors;
}

void BuildDiskMesh(                                                 //
    std::size_t num_sectors,                                        //
    std::size_t num_tracks,                                         //
    std::size_t offset,                                             //
    const std::function<std::array<float, 8>(float, float)>& eval,  //
    std::vector<float>* vertices,                                   //
    std::vector<int>* indices) {
  std::array<float, 8> vertex;
  for (std::size_t i = 1; i <= num_tracks; ++i) {
    float v = i / static_cast<float>(num_tracks);
    vertex = eval(0.0f, v);
    vertices->insert(vertices->end(), vertex.begin(), vertex.end());
    ++offset;
  }
  for (std::size_t j = 1; j <= num_sectors; ++j) {
    // Add apex vertex.
    vertex = eval((j - 0.5f) / num_sectors, 0.0f);
    vertices->insert(vertices->end(), vertex.begin(), vertex.end());
    ++offset;
    // Add apical face.
    indices->push_back(offset);
    indices->push_back(offset - 1);
    indices->push_back(offset - num_tracks - 1);
    // Trace the face strip from the apex.
    float u = j / static_cast<float>(num_sectors);
    for (std::size_t i = 1; i < num_tracks; ++i) {
      float v = i / static_cast<float>(num_tracks);
      vertex = eval(u, v);
      vertices->insert(vertices->end(), vertex.begin(), vertex.end());
      ++offset;
      // Triangulate strip faces;
      indices->push_back(offset);
      indices->push_back(offset - 1);
      indices->push_back(offset - num_tracks - 2);
      indices->push_back(offset - num_tracks - 1);
      indices->push_back(offset);
      indices->push_back(offset - num_tracks - 2);
    }
    // Add rim vertex.
    vertex = eval(u, 1.0f);
    vertices->insert(vertices->end(), vertex.begin(), vertex.end());
    ++offset;
  }
}

void ComputeRectMeshSize(       //
    std::size_t num_rows,       //
    std::size_t num_cols,       //
    std::size_t* num_vertices,  //
    std::size_t* num_triangles) {
  *num_vertices = (num_rows + 1) * (num_cols + 1);
  *num_triangles = num_rows * num_cols * 2;
}

void BuildRectMesh(                                                 //
    std::size_t num_rows,                                           //
    std::size_t num_cols,                                           //
    std::size_t offset,                                             //
    const std::function<std::array<float, 8>(float, float)>& eval,  //
    std::vector<float>* vertices,                                   //
    std::vector<int>* indices) {
  std::array<float, 8> vertex;
  for (std::size_t i = 0; i <= num_rows; ++i) {
    float v = i / static_cast<float>(num_rows);
    vertex = eval(0.0f, v);
    vertices->insert(vertices->end(), vertex.begin(), vertex.end());
    ++offset;
  }
  for (std::size_t j = 1; j <= num_cols; ++j) {
    float u = j / static_cast<float>(num_cols);
    for (std::size_t i = 0; i < num_rows; ++i) {
      float v = i / static_cast<float>(num_rows);
      vertex = eval(u, v);
      vertices->insert(vertices->end(), vertex.begin(), vertex.end());
      ++offset;
      // Triangulate strip faces;
      indices->push_back(offset - num_rows - 2);
      indices->push_back(offset - 1);
      indices->push_back(offset);
      indices->push_back(offset - num_rows - 2);
      indices->push_back(offset);
      indices->push_back(offset - num_rows - 1);
    }
    vertex = eval(u, 1.0f);
    vertices->insert(vertices->end(), vertex.begin(), vertex.end());
    ++offset;
  }
}

void BuildDefaultLocators(                                             //
    const std::function<Transform(float, float, float)>& eval_socket,  //
    const std::function<Transform(float, float, float)>& eval_plug,    //
    absl::flat_hash_map<std::string, Transform>* locators) {
  static const char* const kHeightPrefix[] = {"bottom_", "centre_", "top_"};
  for (int k = 0; k < 3; ++k) {
    const float w = k - 1.0f;
    static const char* const kDepthPrefix[] = {"back_", "centre_", "front_"};
    for (int j = 0; j < 3; ++j) {
      const float v = j - 1.0f;
      static const char* const kWidthPrefix[] = {"left_", "centre_", "right_"};
      for (int i = 0; i < 3; ++i) {
        const float u = i - 1.0f;
        const auto kPrefix =
            absl::StrCat(kDepthPrefix[j], kHeightPrefix[k], kWidthPrefix[i]);
        const auto kSocketName = absl::StrCat(kPrefix, "s");
        (*locators)[kSocketName] = eval_socket(u, v, w);
        const auto kPlugName = absl::StrCat(kPrefix, "p");
        (*locators)[kPlugName] = eval_plug(u, v, w);
      }
    }
  }
}

Eigen::Vector3f ComputeDefaultZDir(  //
    float u, float v, float w,       //
    float rx, float ry, float rz) {
  // Compute z_dir as the normal vector of the ellipsoid inscribed in the
  // primitive's bounding box, uniformly scaled so that it is tangent to the
  // locator's position.
  Eigen::Vector3f z_dir(u * rx, v * ry, w * rz);
  float norm = z_dir.norm();
  if (norm > kEpsilon) {
    return z_dir / norm;
  } else {
    return Eigen::Vector3f::UnitZ();
  }
}

Eigen::Vector3f ComputeDefaultYVector(  //
    float u, float v, float w) {
  // Deterministic choice of y_vector which ensures no-parallelism with the
  // z_dir vector computed by the function above.
  if (u * u + v * v <= kEpsilon) {
    if (w >= 0.0f) {
      return Eigen::Vector3f::UnitY();
    } else {
      return -Eigen::Vector3f::UnitY();
    }
  } else {
    return -Eigen::Vector3f::UnitZ();
  }
}

}  // namespace geometry
}  // namespace lab
}  // namespace deepmind
