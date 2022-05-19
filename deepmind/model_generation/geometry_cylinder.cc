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

#include "deepmind/model_generation/geometry_cylinder.h"

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

Model::Surface CreateSurface(const Cylinder& cylinder) {
  Model::Surface surf;
  // Run argument checks.
  CHECK_GT(cylinder.width_radius, kEpsilon);
  CHECK_GT(cylinder.depth_radius, kEpsilon);
  CHECK_GT(cylinder.height, kEpsilon);
  CHECK_GT(cylinder.num_phi_segments, 0);
  CHECK_GT(cylinder.num_radius_segments, 0);
  CHECK_GT(cylinder.num_height_segments, 0);
  // Reserve storage for cylinder sheet and caps.
  const std::size_t sections = cylinder.num_phi_segments * 4;
  std::size_t sheet_num_vertices, sheet_num_triangles;
  ComputeRectMeshSize(cylinder.num_height_segments, sections,
                      &sheet_num_vertices, &sheet_num_triangles);
  std::size_t cap_num_vertices, cap_num_triangles;
  ComputeDiskMeshSize(sections, cylinder.num_radius_segments, &cap_num_vertices,
                      &cap_num_triangles);
  surf.vertices.reserve((sheet_num_vertices + cap_num_vertices * 2) * 8);
  surf.indices.reserve((sheet_num_triangles + cap_num_triangles * 2) * 3);
  // Build cylinder sheet.
  const float rx = 1.0f / cylinder.width_radius;
  const float ry = 1.0f / cylinder.depth_radius;
  BuildRectMesh(cylinder.num_height_segments, sections,
                surf.vertices.size() / 8,
                [rx, ry, &cylinder](float u, float v) -> std::array<float, 8> {
                  const float phi = 2.0f * kPi * u;
                  const float x = std::cos(phi);
                  const float y = -std::sin(phi);
                  const float z = v - 0.5f;
                  Eigen::Vector3f nrm = {rx * x, ry * y, 0.0f};
                  nrm.normalize();
                  return {{x * cylinder.width_radius, y * cylinder.depth_radius,
                           z * cylinder.height, nrm[0], nrm[1], nrm[2], u, v}};
                },
                &surf.vertices, &surf.indices);
  // Build top cap.
  BuildDiskMesh(sections, cylinder.num_radius_segments,
                surf.vertices.size() / 8,
                [&cylinder](float u, float v) -> std::array<float, 8> {
                  const float phi = 2.0f * kPi * u;
                  const float px = std::cos(phi) * v * cylinder.width_radius;
                  const float py = -std::sin(phi) * v * cylinder.depth_radius;
                  const float pz = 0.5f * cylinder.height;
                  return {{px, py, pz, 0.0f, 0.0f, 1.0f, u, v}};
                },
                &surf.vertices, &surf.indices);
  // Build bottom cap.
  BuildDiskMesh(sections, cylinder.num_radius_segments,
                surf.vertices.size() / 8,
                [&cylinder](float u, float v) -> std::array<float, 8> {
                  const float phi = 2.0f * kPi * u;
                  const float px = std::cos(phi) * v * cylinder.width_radius;
                  const float py = std::sin(phi) * v * cylinder.depth_radius;
                  const float pz = -0.5f * cylinder.height;
                  return {{px, py, pz, 0.0f, 0.0f, -1.0f, u, v}};
                },
                &surf.vertices, &surf.indices);
  surf.name = "cylinder_surface";
  surf.shader_name = cylinder.shader_name;
  return surf;
}

Model::LocatorMap CreateLocators(const Cylinder& cylinder,
                                 const Eigen::Vector3f& offset) {
  Model::LocatorMap locators;
  const float rx = 1.0f / cylinder.width_radius;
  const float ry = 1.0f / cylinder.depth_radius;
  const float rz = 2.0f / cylinder.height;
  BuildDefaultLocators(
      [rx, ry, rz, &cylinder, &offset](float u, float v, float w) -> Transform {
        auto z_dir = ComputeDefaultZDir(u, v, w, rx, ry, rz);
        auto y_vector = ComputeDefaultYVector(u, v, w);
        Eigen::Vector3f pos(u, v, w);
        pos.head<2>().normalize();
        pos[0] *= cylinder.width_radius;
        pos[1] *= cylinder.depth_radius;
        pos[2] *= 0.5 * cylinder.height;
        return CreateZAlignedFrame(pos + offset, -z_dir, -y_vector);
      },
      [rx, ry, rz, &cylinder, &offset](float u, float v, float w) -> Transform {
        auto z_dir = ComputeDefaultZDir(u, v, w, rx, ry, rz);
        auto y_vector = ComputeDefaultYVector(u, v, w);
        Eigen::Vector3f pos(u, v, w);
        pos.head<2>().normalize();
        pos[0] *= cylinder.width_radius;
        pos[1] *= cylinder.depth_radius;
        pos[2] *= 0.5 * cylinder.height;
        return CreateZAlignedFrame(pos + offset, z_dir, y_vector);
      },
      &locators);
  return locators;
}

}  // namespace geometry
}  // namespace lab
}  // namespace deepmind
