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

#include "deepmind/model_generation/geometry_cone.h"

#include <array>
#include <cmath>
#include <string>
#include <vector>

#include "Eigen/Geometry"
#include "absl/log/check.h"
#include "deepmind/model_generation/geometry_util.h"
#include "deepmind/model_generation/model.h"
#include "deepmind/model_generation/transform.h"

namespace deepmind {
namespace lab {
namespace geometry {

Model::Surface CreateSurface(const Cone& cone) {
  Model::Surface surf;
  // Run argument checks.
  CHECK_GT(cone.width_radius, kEpsilon);
  CHECK_GT(cone.depth_radius, kEpsilon);
  CHECK_GT(cone.height, kEpsilon);
  CHECK_GT(cone.num_phi_segments, 0);
  CHECK_GT(cone.num_radius_segments, 0);
  CHECK_GT(cone.num_height_segments, 0);
  // Reserve storage for conical sheet and caps.
  const std::size_t sections = cone.num_phi_segments * 4;
  std::size_t sheet_num_vertices, sheet_num_triangles;
  ComputeDiskMeshSize(sections, cone.num_height_segments, &sheet_num_vertices,
                      &sheet_num_triangles);
  std::size_t cap_num_vertices, cap_num_triangles;
  ComputeDiskMeshSize(sections, cone.num_radius_segments, &cap_num_vertices,
                      &cap_num_triangles);
  surf.vertices.reserve((sheet_num_vertices + cap_num_vertices) * 8);
  surf.indices.reserve((sheet_num_triangles + cap_num_triangles) * 3);
  // Build cone sheet.
  const float rx = 1.0f / cone.width_radius;
  const float ry = 1.0f / cone.depth_radius;
  const float rz = 1.0f / cone.height;
  const float nz = 0.70710678118f;   // 0.5f * kPi - std::atan(1.0f)
  const float nxy = 0.70710678118f;  // sqrt(1.0f - nz * nz)
  BuildDiskMesh(sections, cone.num_height_segments, surf.vertices.size() / 8,
                [rx, ry, rz, nz, nxy, &cone](float u,
                                             float v) -> std::array<float, 8> {
                  const float phi = 2.0f * kPi * u;
                  const float px = std::cos(phi) * v * cone.width_radius;
                  const float py = -std::sin(phi) * v * cone.depth_radius;
                  const float pz = (0.5f - v) * cone.height;
                  Eigen::Vector3f nrm = {std::cos(phi) * nxy * rx,
                                         -std::sin(phi) * nxy * ry, nz * rz};
                  nrm.normalize();
                  return {{px, py, pz, nrm[0], nrm[1], nrm[2], u, v}};
                },
                &surf.vertices, &surf.indices);
  // Build cone cap.
  BuildDiskMesh(sections, cone.num_radius_segments, surf.vertices.size() / 8,
                [&cone](float u, float v) -> std::array<float, 8> {
                  const float phi = 2.0f * kPi * u;
                  const float x = std::cos(phi) * v * cone.width_radius;
                  const float y = std::sin(phi) * v * cone.depth_radius;
                  const float z = -0.5f * cone.height;
                  return {{x, y, z, 0.0f, 0.0f, -1.0f, u, v}};
                },
                &surf.vertices, &surf.indices);
  surf.name = "cone_surface";
  surf.shader_name = cone.shader_name;
  return surf;
}

Model::LocatorMap CreateLocators(const Cone& cone,
                                 const Eigen::Vector3f& offset) {
  Model::LocatorMap locators;
  const float rx = 1.0f / cone.width_radius;
  const float ry = 1.0f / cone.depth_radius;
  const float rz = 1.0f / cone.height;
  BuildDefaultLocators(
      [rx, ry, rz, &cone, &offset](float u, float v, float w) -> Transform {
        auto z_dir = ComputeDefaultZDir(u, v, w, rx, ry, rz);
        auto y_vector = ComputeDefaultYVector(u, v, w);
        Eigen::Vector3f pos(u, v, w);
        pos.head<2>().normalize();
        pos[0] *= 0.5f * (1.0f - w) * cone.width_radius;
        pos[1] *= 0.5f * (1.0f - w) * cone.depth_radius;
        pos[2] *= 0.5f * cone.height;
        return CreateZAlignedFrame(pos + offset, -z_dir, -y_vector);
      },
      [rx, ry, rz, &cone, &offset](float u, float v, float w) -> Transform {
        auto z_dir = ComputeDefaultZDir(u, v, w, rx, ry, rz);
        auto y_vector = ComputeDefaultYVector(u, v, w);
        Eigen::Vector3f pos(u, v, w);
        pos.head<2>().normalize();
        pos[0] *= 0.5f * (1.0f - w) * cone.width_radius;
        pos[1] *= 0.5f * (1.0f - w) * cone.depth_radius;
        pos[2] *= 0.5f * cone.height;
        return CreateZAlignedFrame(pos + offset, z_dir, y_vector);
      },
      &locators);
  return locators;
}

}  // namespace geometry
}  // namespace lab
}  // namespace deepmind
