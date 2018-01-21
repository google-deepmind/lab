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

#ifndef DML_DEEPMIND_MODEL_GENERATION_MODEL_UTIL_H_
#define DML_DEEPMIND_MODEL_GENERATION_MODEL_UTIL_H_

#include "Eigen/Geometry"
#include "deepmind/model_generation/model.h"

namespace deepmind {
namespace lab {

// Transforms the geometry of the given surface in-place, using matrix 'xfrm'
// for vertex positions and the inverse of its linear part 'xfrm_i' for vertex
// normals. The bounding box 'bbox' is updated to encompass the transformed
// vertices.
void TransformSurface(              //
    const Eigen::Affine3f& xfrm,    //
    const Eigen::Matrix3f& xfrm_i,  //
    Model::Surface* surface,        //
    Eigen::AlignedBox3f* bbox);

}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_MODEL_GENERATION_MODEL_UTIL_H_
