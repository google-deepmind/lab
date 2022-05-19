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

#include "deepmind/model_generation/geometry_sphere.h"

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

Model::Surface CreateSurface(const Sphere& sphere) {
  Model::Surface surf;
  // Run argument checks.
  CHECK_GT(sphere.width_radius, kEpsilon);
  CHECK_GT(sphere.depth_radius, kEpsilon);
  CHECK_GT(sphere.height_radius, kEpsilon);
  CHECK_GT(sphere.num_phi_segments, 0);
  CHECK_GT(sphere.num_theta_segments, 0);
  // Reserve storage for two hemispheres.
  const std::size_t sections = sphere.num_phi_segments * 4;
  std::size_t hemi_num_vertices, hemi_num_triangles;
  ComputeDiskMeshSize(sections, sphere.num_theta_segments, &hemi_num_vertices,
                      &hemi_num_triangles);
  surf.vertices.reserve(hemi_num_vertices * 2 * 8);
  surf.indices.reserve(hemi_num_triangles * 2 * 3);
  // Build two hemispheres.
  const float rx = 1.0f / sphere.width_radius;
  const float ry = 1.0f / sphere.depth_radius;
  const float rz = 1.0f / sphere.height_radius;
  BuildDiskMesh(
      sections, sphere.num_theta_segments, surf.vertices.size() / 8,
      [rx, ry, rz, &sphere](float u, float v) -> std::array<float, 8> {
        const float phi = 2.0f * kPi * u;
        const float theta = 0.5f * kPi * v;
        const float x = std::cos(phi) * std::sin(theta);
        const float y = -std::sin(phi) * std::sin(theta);
        const float z = std::cos(theta);
        Eigen::Vector3f nrm = {x * rx, y * ry, z * rz};
        nrm.normalize();
        return {{x * sphere.width_radius, y * sphere.depth_radius,
                 z * sphere.height_radius, nrm[0], nrm[1], nrm[2], u, v}};
      },
      &surf.vertices, &surf.indices);
  BuildDiskMesh(
      sections, sphere.num_theta_segments, surf.vertices.size() / 8,
      [rx, ry, rz, &sphere](float u, float v) -> std::array<float, 8> {
        const float phi = 2.0f * kPi * u;
        const float theta = 0.5f * kPi * v;
        const float x = std::cos(phi) * std::sin(theta);
        const float y = std::sin(phi) * std::sin(theta);
        const float z = -std::cos(theta);
        Eigen::Vector3f nrm = {x * rx, y * ry, z * rz};
        nrm.normalize();
        return {{x * sphere.width_radius, y * sphere.depth_radius,
                 z * sphere.height_radius, nrm[0], nrm[1], nrm[2], u, v}};
      },
      &surf.vertices, &surf.indices);
  surf.name = "sphere_surface";
  surf.shader_name = sphere.shader_name;
  return surf;
}

Model::LocatorMap CreateLocators(const Sphere& sphere,
                                 const Eigen::Vector3f& offset) {
  Model::LocatorMap locators;
  const float rx = 1.0f / sphere.width_radius;
  const float ry = 1.0f / sphere.depth_radius;
  const float rz = 1.0f / sphere.height_radius;
  BuildDefaultLocators(
      [rx, ry, rz, &sphere, &offset](float u, float v, float w) -> Transform {
        auto z_dir = ComputeDefaultZDir(u, v, w, rx, ry, rz);
        auto y_vector = ComputeDefaultYVector(u, v, w);
        Eigen::Vector3f pos(u, v, w);
        pos.normalize();
        pos[0] *= sphere.width_radius;
        pos[1] *= sphere.depth_radius;
        pos[2] *= sphere.height_radius;
        return CreateZAlignedFrame(pos + offset, -z_dir, -y_vector);
      },
      [rx, ry, rz, &sphere, &offset](float u, float v, float w) -> Transform {
        auto z_dir = ComputeDefaultZDir(u, v, w, rx, ry, rz);
        auto y_vector = ComputeDefaultYVector(u, v, w);
        Eigen::Vector3f pos(u, v, w);
        pos.normalize();
        pos[0] *= sphere.width_radius;
        pos[1] *= sphere.depth_radius;
        pos[2] *= sphere.height_radius;
        return CreateZAlignedFrame(pos + offset, z_dir, y_vector);
      },
      &locators);
  return locators;
}

}  // namespace geometry
}  // namespace lab
}  // namespace deepmind
