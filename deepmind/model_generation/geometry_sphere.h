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

#ifndef DML_DEEPMIND_MODEL_GENERATION_GEOMETRY_SPHERE_H_
#define DML_DEEPMIND_MODEL_GENERATION_GEOMETRY_SPHERE_H_

#include <cstddef>
#include <string>

#include "Eigen/Geometry"
#include "deepmind/model_generation/model.h"

namespace deepmind {
namespace lab {
namespace geometry {

// Parameters used to construct surfaces in the shape of a ellipsoid (defaulting
// to a sphere with diameter 1).
struct Sphere {
  float width_radius = 0.5f;
  float depth_radius = 0.5f;
  float height_radius = 0.5f;
  // How many segments along the azimuth range of an octant of the sphere.
  std::size_t num_phi_segments = 4;
  // How many segments along the elevation range of an octant of the sphere.
  std::size_t num_theta_segments = 4;
  // Shader used for the cylinder's surfaces.
  std::string shader_name;
};

// Create a surface in the shape of a sphere with the parameters provided.
Model::Surface CreateSurface(const Sphere& sphere_params);

// Create the locator set for a sphere model with the parameters provided. All
// locators will be translated by the offset position.
Model::LocatorMap CreateLocators(
    const Sphere& sphere_params,
    const Eigen::Vector3f& offset = Eigen::Vector3f::Zero());

}  // namespace geometry
}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_MODEL_GENERATION_GEOMETRY_SPHERE_H_
