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

#ifndef DML_DEEPMIND_MODEL_GENERATION_GEOMETRY_UTIL_H_
#define DML_DEEPMIND_MODEL_GENERATION_GEOMETRY_UTIL_H_

#include <array>
#include <cmath>
#include <cstddef>
#include <functional>
#include <string>
#include <vector>

#include "Eigen/Geometry"
#include "absl/container/flat_hash_map.h"
#include "deepmind/model_generation/transform.h"

namespace deepmind {
namespace lab {
namespace geometry {

// Shared constants.
constexpr float kEpsilon = 1.0e-6f;
constexpr float kPi = 3.14159265358979323846f;

// Build a reference frame centered on position `trans` where the z axis is a
// unit vector in the direction of `z_dir`, the x axis is computed as the
// normalised cross product of `y_vector` and the z axis, and the y axis is
// computed as the cross product of the z axis and the x axis.
Eigen::Affine3f CreateZAlignedFrame(  //
    const Eigen::Vector3f& trans,     //
    const Eigen::Vector3f& z_dir,     //
    const Eigen::Vector3f& y_vector);

// Computes the number of vertices and triangles required to discretise a disk
// with the given amount of sectors (angular segments) and tracks (radial
// segments). See https://en.wikipedia.org/wiki/Disk_sector for an illustration.
// Both `num_sectors` and `num_tracks` must be greater than 0.
void ComputeDiskMeshSize(       //
    std::size_t num_sectors,    //
    std::size_t num_tracks,     //
    std::size_t* num_vertices,  //
    std::size_t* num_triangles);

// Samples the given evaluator (eval) at the vertices of a discretised disk and
// builds a polygonal mesh with the results. The sampled vertices and face
// indices are appended to the vector parameters of the same name. The `offset`
// value is added to the face indices. Both `num_sectors` and `num_tracks` must
// be greater than 0.
void BuildDiskMesh(                                                 //
    std::size_t num_sectors,                                        //
    std::size_t num_tracks,                                         //
    std::size_t offset,                                             //
    const std::function<std::array<float, 8>(float, float)>& eval,  //
    std::vector<float>* vertices,                                   //
    std::vector<int>* indices);

// Computes the number of vertices and triangles required to discretise a
// rectangular grid with the given amount of polygonal rows and columns. Both
// `num_rows` and `num_cols` must be greater than 0.
void ComputeRectMeshSize(       //
    std::size_t num_rows,       //
    std::size_t num_cols,       //
    std::size_t* num_vertices,  //
    std::size_t* num_triangles);

// Samples the given evaluator (eval) at the vertices of a discretised rectangle
// and builds a polygonal mesh with the results. The sampled vertices and
// face indices are appended to the vector parameters of the same name. The
// `offset` value is added to the face indices. Both `num_rows` and `num_cols`
// must be greater than 0.
void BuildRectMesh(                                                 //
    std::size_t num_rows,                                           //
    std::size_t num_cols,                                           //
    std::size_t offset,                                             //
    const std::function<std::array<float, 8>(float, float)>& eval,  //
    std::vector<float>* vertices,                                   //
    std::vector<int>* indices);

// Build the default locator set. Default locators are evaluated using functions
// `eval_socket` and `eval_plug`. These functions are called at 27 positions
// evenly spaced within a normalised bounding box of the primitive, and the
// resulting locators are named according to the following regex:
//
//   [back|centre|front]_[bottom|centre|top]_[left|centre|right]_[s|p]
//
// where the s and p suffixes stand for socket and plug, respectively.
// Socket and plug locators are meant to be placed the same position but have
// opposing orientations (sockets are inbound, plugs are outbound).
void BuildDefaultLocators(                                             //
    const std::function<Transform(float, float, float)>& eval_socket,  //
    const std::function<Transform(float, float, float)>& eval_plug,    //
    absl::flat_hash_map<std::string, Transform>* locators);

// Computes the z_dir vector used to construct the default locator set for most
// primitives. Coefficients [u, v, w] are the normalised coordinates (range [-1,
// 1]) of the locator's position within an axis-aligned bounding box
// encompassing the primitive. Coefficients [rx, ry, rz] are the reciprocals of
// the extents of such bounding box along each axis.
Eigen::Vector3f ComputeDefaultZDir(  //
    float u, float v, float w,       //
    float rx, float ry, float rz);

// Computes the y_vector used to construct the default locator set for most
// primitives. Coefficients [u, v, w] are the normalised coordinates (range [-1,
// 1]) of the locator's position within an axis-aligned bounding box
// encompassing the primitive.
Eigen::Vector3f ComputeDefaultYVector(  //
    float u, float v, float w);

}  // namespace geometry
}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_MODEL_GENERATION_GEOMETRY_UTIL_H_
