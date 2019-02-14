// Copyright (C) 2016 Google Inc.
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

#include "deepmind/level_generation/map_builder/builder.h"

#include <string>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"

namespace deepmind {
namespace lab {
namespace map_builder {

// Size of the skybox, placed outside the encompassing world.
constexpr double kSkyboxSize = 12.0;

// Default thickness for skybox brushes.
constexpr double kThickness = 1.0;

Builder::Builder() { entities_.emplace_back(Entity::CreateWorld()); }

std::string Builder::ToString() const {
  return absl::StrCat(
      absl::StrJoin(entities_, "\n\n",
                    [](std::string* out, const Entity& entity) {
                      absl::StrAppend(out, entity.ToString());
                    }),
      "\n");
}

void Builder::AddSkybox(
    const Eigen::Vector3d& position,
    const Eigen::Vector3d& world_size,
    std::string texture_name,
    const Eigen::Vector2i& texture_size) {
  // First create hollow box around the entire world.
  auto half_world_size = world_size * 0.5;
  mutable_world_entity()->add_brushes(brush_util::CreateHollowBox(
      position + half_world_size, position - half_world_size, kThickness,
      {texture_name}));

  // Next, create the skybox entity and surround with a skybox brush.
  const Eigen::Vector3d sky_box_size = {kSkyboxSize, kSkyboxSize, kSkyboxSize};
  auto sky_box_pos =
      Eigen::Vector3d{position.x() - half_world_size.x() - sky_box_size.x(),
                      position.y() - half_world_size.y() - sky_box_size.y(), 0};

  AddEntity(Entity("_skybox", sky_box_pos));

  mutable_world_entity()->add_brushes(brush_util::CreateSkybox(
      sky_box_pos, sky_box_size, kThickness, texture_name, texture_size));
}

}  // namespace map_builder
}  // namespace lab
}  // namespace deepmind
