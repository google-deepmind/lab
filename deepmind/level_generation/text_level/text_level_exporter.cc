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

#include "deepmind/level_generation/text_level/text_level_exporter.h"

#include "absl/strings/str_cat.h"
#include "Eigen/Geometry"
#include "deepmind/level_generation/map_builder/entity.h"

namespace deepmind {
namespace lab {
namespace {

// Platform thickness, expressed in height units.
constexpr int kPlatformThickness = 2;
// Thickness of the border on the platforms edge (in maze cell units).
constexpr double kBorderThickness = 0.075;

// Default size used for skybox.
const Eigen::Vector2i kSkyboxTextureSize = {1024, 1024};

struct TextureFile {
  std::string name;
  int width;
  int height;
};

map_builder::Texture MapBuilderTexture(Theme::Texture texture) {
  return {std::move(texture.name),
          {0, 0},
          texture.angle,
          {TextLevelExporter::kTexelSize / texture.width * texture.scale,
           TextLevelExporter::kTexelSize / texture.height * texture.scale}};
}

}  // namespace

TextLevelExporter::TextLevelExporter(TextLevelSettings* settings)
    : settings_(settings) {
  builder_.mutable_world_entity()->set_attribute("light", absl::StrCat(100));
  builder_.mutable_world_entity()->set_attribute("worldtype", absl::StrCat(2));
}

void TextLevelExporter::SetBoundingBox(Eigen::Vector3d size) {
  bounding_box_size_ = std::move(size);
  bounding_box_size_.z() *= settings_->ceiling_scale;
}

map_builder::Patch TextLevelExporter::GenerateWallDecal(
    const Theme::Texture& texture, const Eigen::Vector3d& a,
    const Eigen::Vector3d& b, const Eigen::Vector3d& interior_direction) {
  const Eigen::Vector3d kUp = {0, 0, 1};
  Eigen::Vector3d pos = (a + b) * (0.5 * settings_->cell_size);
  // Push decal slightly away from wall.
  double scaleX = texture.scale;
  double scaleY = texture.scale;
  if (texture.width > texture.height) {
    scaleY *= static_cast<double>(texture.height) /
              static_cast<double>(texture.width);
  } else if (texture.width < texture.height) {
    scaleX *= static_cast<double>(texture.width) /
              static_cast<double>(texture.height);
  }

  pos += (2.0 / map_builder::kWorldToGameUnits) * interior_direction;
  Eigen::Vector3d up = Eigen::AngleAxis<double>(
                           map_builder::Angle::Degrees(texture.angle).radians(),
                           interior_direction) *
                       kUp;
  return map_builder::brush_util::CreateGridPatch(
      pos, -interior_direction, up, {scaleX, scaleY}, {3, 3}, {texture.name});
}

void TextLevelExporter::AddWall(const Eigen::Vector3d& a,
                                const Eigen::Vector3d& b,
                                const Eigen::Vector3d& interior_direction,
                                const Eigen::Vector2i& cell,
                                CellGroup group_number) {
  Theme::Direction direction = Theme::Direction::South;
  if (interior_direction.x() > 0) {
    direction = Theme::Direction::West;
  } else if (interior_direction.x() < 0) {
    direction = Theme::Direction::East;
  } else if (interior_direction.y() < 0) {
    direction = Theme::Direction::North;
  }

  auto& wall_textures = wall_textures_[static_cast<int>(direction)];
  auto wall_tex_it = wall_textures.find(group_number);
  if (wall_tex_it == wall_textures.end()) {
    wall_textures[group_number] =
        MapBuilderTexture(settings_->theme->wall(group_number, direction));
  }

  builder_.mutable_world_entity()->add_brush(
      map_builder::brush_util::CreateBoxBrush(
          a * settings_->cell_size +
              Eigen::Vector3d::Zero().cwiseMin(interior_direction) /
                  map_builder::kWorldToGameUnits,
          b * settings_->cell_size +
              Eigen::Vector3d::Zero().cwiseMax(interior_direction) /
                  map_builder::kWorldToGameUnits,
          wall_textures[group_number]));

  wall_art_locations_.push_back(
      WallArtLocation{a, b, interior_direction, cell, group_number, direction});
}

void TextLevelExporter::AddFloor(const Eigen::Vector3d& a,
                                 const Eigen::Vector3d& b,
                                 const Eigen::Vector2i& cell,
                                 CellGroup group_number) {
  // Walls must have a thickness > 0 to be rendered.
  Eigen::Vector3d thickness_offset(0, 0,
                                   (1.0 / map_builder::kWorldToGameUnits));

  if (floor_textures_.find(group_number) == floor_textures_.end()) {
    floor_textures_[group_number] =
        MapBuilderTexture(settings_->theme->floor(group_number));
    ceiling_textures_[group_number] =
        MapBuilderTexture(settings_->theme->ceiling(group_number));
  }

  auto* world_entity = builder_.mutable_world_entity();

  world_entity->add_brush(map_builder::brush_util::CreateBoxBrush(
      a * settings_->cell_size, b * settings_->cell_size + thickness_offset,
      floor_textures_[group_number]));

  // Don't add ceiling if we have a skybox set.
  if (settings_->skybox_texture_name.empty()) {
    Eigen::Vector3d ceiling_height(0, 0, bounding_box_size_.z());
    world_entity->add_brush(map_builder::brush_util::CreateBoxBrush(
        (a + ceiling_height) * settings_->cell_size - thickness_offset,
        (b + ceiling_height) * settings_->cell_size,
        ceiling_textures_[group_number]));
  }
  floor_art_locations_.push_back({(a + b) / 2, cell, group_number});
}

map_builder::Entity TextLevelExporter::MakeEntity(
    const Eigen::Vector3d& position, std::string class_name,
    const std::vector<std::pair<std::string, std::string>>& attributes) {
  map_builder::Entity entity(std::move(class_name),
                             position * settings_->cell_size);
  entity.set_attributes(attributes);
  return entity;
}

map_builder::Entity TextLevelExporter::MakeEntityWithRealOffset(
    const Eigen::Vector3d& position, const Eigen::Vector3d& offset,
    std::string class_name,
    const std::vector<std::pair<std::string, std::string>>& attributes) {
  map_builder::Entity entity(std::move(class_name),
                             position * settings_->cell_size + offset);
  entity.set_attributes(attributes);
  return entity;
}

map_builder::Entity TextLevelExporter::MakeBrushEntity(
    const Eigen::Vector3d& min, const Eigen::Vector3d& max,
    std::string class_name, std::string texture_name,
    double texture_scale_width, double texture_scale_height,
    const std::vector<std::pair<std::string, std::string>>& attributes) {
  map_builder::Entity entity(std::move(class_name));
  map_builder::Texture texture =
      texture_name.empty()
          ? floor_textures_[0]
          : map_builder::Texture{std::move(texture_name), {0, 0}, 0,
                                 {kTexelSize * texture_scale_width,
                                  kTexelSize * texture_scale_height}};
  entity.add_brush(map_builder::brush_util::CreateBoxBrush(
      settings_->cell_size * min, settings_->cell_size * max, texture));
  entity.set_attributes(attributes);
  return entity;
}

map_builder::Entity TextLevelExporter::MakeFittedBrushEntity(
    const Eigen::Vector3d& min, const Eigen::Vector3d& max,
    std::string class_name, const std::string& texture_name,
    double texture_width, double texture_height,
    const std::vector<std::pair<std::string, std::string>>& attributes) {
  map_builder::Entity entity(std::move(class_name));
  entity.add_brush(map_builder::brush_util::CreateFittedBoxBrush(
      settings_->cell_size * min, settings_->cell_size * max, texture_name,
      {texture_width, texture_height}));
  entity.set_attributes(attributes);
  return entity;
}

map_builder::Entity TextLevelExporter::MakeBrushEntity(
    const std::vector<std::pair<Eigen::Vector3d, Eigen::Vector3d>>& blocks,
    std::string class_name,
    std::string texture_name,
    double texture_scale_width,
    double texture_scale_height,
    const std::vector<std::pair<std::string, std::string>>& attributes) {
  map_builder::Entity entity(std::move(class_name));
  map_builder::Texture texture =
      texture_name.empty()
          ? floor_textures_[0]
          : map_builder::Texture{std::move(texture_name),
                                 {0, 0},
                                 0,
                                 {kTexelSize * texture_scale_width,
                                  kTexelSize * texture_scale_height}};
  for (auto& block : blocks) {
    entity.add_brush(map_builder::brush_util::CreateBoxBrush(
        settings_->cell_size * block.first, settings_->cell_size * block.second,
        texture));
  }
  entity.set_attributes(attributes);
  return entity;
}

void TextLevelExporter::AddPlatform(double x, double y, int height) {
  if (floor_textures_.find(height) == floor_textures_.end()) {
    floor_textures_[height] =
        MapBuilderTexture(settings_->theme->floor(height));
  }
  if (riser_texture_.path.empty()) {
    riser_texture_ = MapBuilderTexture(settings_->theme->platform_riser());
  }

  if (tread_texture_.path.empty()) {
    tread_texture_ = MapBuilderTexture(settings_->theme->platform_tread());
  }
  // Floor must have a thickness > 0 to be rendered.
  double min_thickness = 1.0 / map_builder::kWorldToGameUnits;
  double z = height * kHeightScale - min_thickness;

  auto* world_entity = builder_.mutable_world_entity();

  // Add the platform brick base.
  Eigen::Vector3d brick_min(x, y, z - kPlatformThickness * kHeightScale);
  Eigen::Vector3d brick_max(x + 1.0, y + 1.0, z - kBorderThickness);
  world_entity->add_brush(map_builder::brush_util::CreateBoxBrush(
      brick_min * settings_->cell_size, brick_max * settings_->cell_size,
      riser_texture_));

  // Add a black border on top of the platform base.
  Eigen::Vector3d base_min(x, y, z - kBorderThickness);
  Eigen::Vector3d base_max(x + 1.0, y + 1.0, z);
  world_entity->add_brush(map_builder::brush_util::CreateBoxBrush(
      base_min * settings_->cell_size, base_max * settings_->cell_size,
      tread_texture_));

  // Add the flooring layer on top of the platform base and border.
  Eigen::Vector3d floor_min(x, y, z);
  Eigen::Vector3d floor_max(x + 1.0, y + 1.0, z + min_thickness);
  world_entity->add_brush(map_builder::brush_util::CreateBoxBrush(
      floor_min * settings_->cell_size, floor_max * settings_->cell_size,
      floor_textures_[height]));
}

void TextLevelExporter::AddGlassColumn(double x, double y, int height) {
  Eigen::Vector3d a(x, y, 0);
  Eigen::Vector3d b(x + 1.0, y + 1.0, height * kHeightScale);
  builder_.mutable_world_entity()->add_brush(
      map_builder::brush_util::CreateBoxBrush(
          a * settings_->cell_size, b * settings_->cell_size, glass_texture_));
}

map_builder::Entity TextLevelExporter::MakeLight(
    const Eigen::Vector3d& position, double intensity) {
  return MakeEntity(
      position, "light",
      // Default light brightness is 200.
      {{"light", absl::StrCat(intensity * 5 * settings_->light_intensity)},
       {"style", "0"},
       {"spawnflags", "0"}});
}

void TextLevelExporter::Finalize() {
  auto floor_decorations =
      settings_->theme->FloorDecorations(floor_art_locations_);
  for (const auto& decoration : floor_decorations) {
    if (!decoration.model.name.empty()) {
      Add(MakeEntityWithRealOffset(
          decoration.location.location, Eigen::Vector3d(0, 0, 1), "misc_model",
          {{"model", decoration.model.name},
           {"angle", absl::StrCat(decoration.model.angle)},
           {"modelscale", absl::StrCat(decoration.model.scale)}}));
    }
  }

  auto wall_decorations =
      settings_->theme->WallDecorations(wall_art_locations_);
  for (const auto& decoration : wall_decorations) {
    if (!decoration.texture.name.empty()) {
      builder_.mutable_world_entity()->add_patch(
          GenerateWallDecal(decoration.texture, decoration.location.top_left,
                            decoration.location.bottom_right,
                            decoration.location.interior_direction));
    }
  }

  // Add skybox.
  if (!settings_->skybox_texture_name.empty()) {
    Eigen::Vector3d cell_size = Eigen::Vector3d::Constant(settings_->cell_size);
    auto world_size = (bounding_box_size_ * settings_->cell_size) + cell_size;
    auto pos = (world_size - cell_size) * 0.5;
    builder_.AddSkybox(pos, world_size, settings_->skybox_texture_name,
                       kSkyboxTextureSize);
  }
}

}  // namespace lab
}  // namespace deepmind
