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

#ifndef DML_DEEPMIND_MODEL_GENERATION_MODEL_H_
#define DML_DEEPMIND_MODEL_GENERATION_MODEL_H_

#include <string>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "deepmind/model_generation/transform.h"

namespace deepmind {
namespace lab {

// Encapsulates data retrieved from custom models, either defined by the level
// API, or created for the primitives implemented in the model library.
struct Model {
  // Model name.
  std::string name;

  // Custom models are composed of polygonal surfaces, rendered with a given
  // shader.
  struct Surface {
    std::string name;
    // Vertex data, encoded as [pos_x, pos_y, pos_z, nrm_x, nrm_y, nrm_z, tex_s,
    // tex_t]
    std::vector<float> vertices;
    // Triples of vertex indices representing each triangular face.
    std::vector<int> indices;
    std::string shader_name;
  };

  std::vector<Surface> surfaces;

  // Custom models also include named reference frames which serve as locators
  // where to attach child models.
  using LocatorMap = absl::flat_hash_map<std::string, Transform>;

  LocatorMap locators;
};

}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_MODEL_GENERATION_MODEL_H_
