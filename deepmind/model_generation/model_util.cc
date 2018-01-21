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

#include "deepmind/model_generation/model_util.h"

namespace deepmind {
namespace lab {

void TransformSurface(              //
    const Eigen::Affine3f& xfrm,    //
    const Eigen::Matrix3f& xfrm_i,  //
    Model::Surface* surface,        //
    Eigen::AlignedBox3f* bbox) {
  auto& vertices = surface->vertices;
  for (std::size_t i = 0; i < vertices.size(); i += 8) {
    Eigen::Vector4f pos(vertices[i], vertices[i + 1], vertices[i + 2], 1.0f);
    Eigen::RowVector3f nrm(vertices[i + 3], vertices[i + 4], vertices[i + 5]);
    pos = xfrm * pos;
    nrm = nrm * xfrm_i;
    nrm.normalize();
    for (std::size_t j = 0; j < 3; ++j) {
      vertices[i + j] = pos[j];
      vertices[i + 3 + j] = nrm[j];
    }
    bbox->extend(pos.head<3>());
  }
}

}  // namespace lab
}  // namespace deepmind
