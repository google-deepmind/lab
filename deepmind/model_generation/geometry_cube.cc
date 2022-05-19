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

#include "deepmind/model_generation/geometry_cube.h"

#include <array>
#include <string>
#include <vector>

#include "absl/log/check.h"
#include "deepmind/model_generation/geometry_util.h"
#include "deepmind/model_generation/model.h"
#include "deepmind/model_generation/transform.h"

namespace deepmind {
namespace lab {
namespace geometry {

Model::Surface CreateSurface(const Cube& cube) {
  Model::Surface surf;
  // Run argument checks.
  CHECK_GT(cube.width, kEpsilon);
  CHECK_GT(cube.depth, kEpsilon);
  CHECK_GT(cube.height, kEpsilon);
  CHECK_GT(cube.num_width_segments, 0);
  CHECK_GT(cube.num_depth_segments, 0);
  CHECK_GT(cube.num_height_segments, 0);
  // Reserve storage for 3 pairs of face sheets.
  std::size_t zy_num_vertices, zy_num_triangles;
  ComputeRectMeshSize(cube.num_depth_segments, cube.num_height_segments,
                      &zy_num_vertices, &zy_num_triangles);
  std::size_t xz_num_vertices, xz_num_triangles;
  ComputeRectMeshSize(cube.num_height_segments, cube.num_width_segments,
                      &xz_num_vertices, &xz_num_triangles);
  std::size_t xy_num_vertices, xy_num_triangles;
  ComputeRectMeshSize(cube.num_depth_segments, cube.num_width_segments,
                      &xy_num_vertices, &xy_num_triangles);
  surf.vertices.reserve((zy_num_vertices + xz_num_vertices + xy_num_vertices) *
                        2 * 8);
  surf.indices.reserve(
      (zy_num_triangles + xz_num_triangles + xy_num_triangles) * 2 * 3);
  // Build zy faces.
  BuildRectMesh(cube.num_depth_segments, cube.num_height_segments,
                surf.vertices.size() / 8,
                [&cube](float u, float v) -> std::array<float, 8> {
                  const float x = 0.5f * cube.width;
                  const float y = (0.5f - u) * cube.depth;
                  const float z = (v - 0.5f) * cube.height;
                  return {{x, y, z, 1.0f, 0.0f, 0.0f, u, v}};
                },
                &surf.vertices, &surf.indices);
  BuildRectMesh(cube.num_depth_segments, cube.num_height_segments,
                surf.vertices.size() / 8,
                [&cube](float u, float v) -> std::array<float, 8> {
                  const float x = -0.5f * cube.width;
                  const float y = (u - 0.5f) * cube.depth;
                  const float z = (v - 0.5f) * cube.height;
                  return {{x, y, z, -1.0f, 0.0f, 0.0f, u, v}};
                },
                &surf.vertices, &surf.indices);
  // Build xz faces.
  BuildRectMesh(cube.num_height_segments, cube.num_width_segments,
                surf.vertices.size() / 8,
                [&cube](float u, float v) -> std::array<float, 8> {
                  const float x = (u - 0.5f) * cube.width;
                  const float y = 0.5f * cube.depth;
                  const float z = (v - 0.5f) * cube.height;
                  return {{x, y, z, 0.0f, 1.0f, 0.0f, u, v}};
                },
                &surf.vertices, &surf.indices);
  BuildRectMesh(cube.num_height_segments, cube.num_width_segments,
                surf.vertices.size() / 8,
                [&cube](float u, float v) -> std::array<float, 8> {
                  const float x = (0.5f - u) * cube.width;
                  const float y = -0.5f * cube.depth;
                  const float z = (v - 0.5f) * cube.height;
                  return {{x, y, z, 0.0f, -1.0f, 0.0f, u, v}};
                },
                &surf.vertices, &surf.indices);
  // Build xy faces.
  BuildRectMesh(cube.num_depth_segments, cube.num_width_segments,
                surf.vertices.size() / 8,
                [&cube](float u, float v) -> std::array<float, 8> {
                  const float x = (u - 0.5f) * cube.width;
                  const float y = (0.5f - v) * cube.depth;
                  const float z = 0.5f * cube.height;
                  return {{x, y, z, 0.0f, 0.0f, 1.0f, u, v}};
                },
                &surf.vertices, &surf.indices);
  BuildRectMesh(cube.num_depth_segments, cube.num_width_segments,
                surf.vertices.size() / 8,
                [&cube](float u, float v) -> std::array<float, 8> {
                  const float x = (u - 0.5f) * cube.width;
                  const float y = (v - 0.5f) * cube.depth;
                  const float z = -0.5f * cube.height;
                  return {{x, y, z, 0.0f, 0.0f, -1.0f, u, v}};
                },
                &surf.vertices, &surf.indices);
  surf.name = "cube_surface";
  surf.shader_name = cube.shader_name;
  return surf;
}

Model::LocatorMap CreateLocators(const Cube& cube,
                                 const Eigen::Vector3f& offset) {
  Model::LocatorMap locators;
  const float rx = 1.0f / cube.width;
  const float ry = 1.0f / cube.depth;
  const float rz = 1.0f / cube.height;
  BuildDefaultLocators(
      [rx, ry, rz, &cube, &offset](float u, float v, float w) -> Transform {
        auto z_dir = ComputeDefaultZDir(u, v, w, rx, ry, rz);
        auto y_vector = ComputeDefaultYVector(u, v, w);
        const Eigen::Vector3f pos(u * 0.5f * cube.width,  //
                                  v * 0.5f * cube.depth,  //
                                  w * 0.5f * cube.height);
        return CreateZAlignedFrame(pos + offset, -z_dir, -y_vector);
      },
      [rx, ry, rz, &cube, &offset](float u, float v, float w) -> Transform {
        auto z_dir = ComputeDefaultZDir(u, v, w, rx, ry, rz);
        auto y_vector = ComputeDefaultYVector(u, v, w);
        const Eigen::Vector3f pos(u * 0.5f * cube.width,  //
                                  v * 0.5f * cube.depth,  //
                                  w * 0.5f * cube.height);
        return CreateZAlignedFrame(pos + offset, z_dir, y_vector);
      },
      &locators);
  return locators;
}

}  // namespace geometry
}  // namespace lab
}  // namespace deepmind
