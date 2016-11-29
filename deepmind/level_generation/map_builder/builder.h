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
//
// Builder: a utility to serialize .map data to text.

#ifndef DML_DEEPMIND_LEVEL_GENERATION_MAP_BUILDER_BUILDER_H_
#define DML_DEEPMIND_LEVEL_GENERATION_MAP_BUILDER_BUILDER_H_

#include <string>
#include <vector>

#include "Eigen/Core"
#include "deepmind/level_generation/map_builder/entity.h"

namespace deepmind {
namespace lab {
namespace map_builder {

// Helper class for creating an Open Arena map file, which can then be used to
// generate a BSP. See http://www.gamers.org/dEngine/quake/QDP/qmapspec.html for
// details. This class is not thread-safe.
class Builder {
 public:
  Builder();

  // Disable copy and assign.
  Builder(const Builder&) = delete;
  void operator=(const Builder&) = delete;

  // Returns the worldspawn entity. Can use this to add brushes or attributes to
  // the worldspawn entity after it has constructed.
  Entity* mutable_world_entity() { return &entities_[0]; }
  const Entity& world_entity() const { return entities_[0]; }

  void AddEntity(Entity entity) { entities_.push_back(std::move(entity)); }

  // Adds a skybox and creates the appropriate brushes, first for the skybox
  // itself, then creating a water-tight box with the provided world_size. The
  // texture name and size are required to ensure the textures are correctly
  // fitted.
  void AddSkybox(
      const Eigen::Vector3d& center,
      const Eigen::Vector3d& world_size,
      std::string texture_name,
      const Eigen::Vector2i& texture_size);

  // Serializes the current map file to a string, which can then be used by the
  // BSP compiler.
  std::string ToString() const;

 private:
  // Ordered array of entities in the map. The first entity is always the world
  // entity which contains the world geometry.
  std::vector<Entity> entities_;
};

}  // namespace map_builder
}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_LEVEL_GENERATION_MAP_BUILDER_BUILDER_H_
