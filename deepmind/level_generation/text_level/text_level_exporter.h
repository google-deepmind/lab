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
// TextMazeExporter: An internal data structure to manage the translation of an
// absract "maze" entity into a .map format representation.

#ifndef DML_DEEPMIND_LEVEL_GENERATION_TEXT_LEVEL_TEXT_LEVEL_EXPORTER_H_
#define DML_DEEPMIND_LEVEL_GENERATION_TEXT_LEVEL_TEXT_LEVEL_EXPORTER_H_

#include <array>
#include <cstddef>
#include <map>
#include <random>
#include <utility>
#include <vector>

#include "Eigen/Core"
#include "deepmind/level_generation/map_builder/brush.h"
#include "deepmind/level_generation/map_builder/builder.h"
#include "deepmind/level_generation/map_builder/entity.h"
#include "deepmind/level_generation/text_level/text_level_settings.h"

namespace deepmind {
namespace lab {
constexpr char kGlassTexture[] = "map/poltergeist";

// Creates a Quake3 .map file from a given maze. Uses map_builder::Builder to
// build up and generate the actual file.
class TextLevelExporter {
 public:
  using CellGroup = int;

  // Scale the textures to match the 100 units cell size.
  static constexpr double kTexelSize = 100.0;

  // A scaling factor for the platforms height.
  static constexpr double kHeightScale = 0.2;

  explicit TextLevelExporter(TextLevelSettings* settings);

  const TextLevelSettings& GetSettings() const { return *settings_; }

  // Call Finalize() before calling ToString().
  void Finalize();

  // Returns the .map format output string.
  std::string ToString() const { return builder_.ToString(); }

  void SetBoundingBox(Eigen::Vector3d size);

  void AddFloor(
      const Eigen::Vector3d& a,
      const Eigen::Vector3d& b,
      const Eigen::Vector2i& cell,
      CellGroup group_number);

  void AddWall(
      const Eigen::Vector3d& a,
      const Eigen::Vector3d& b,
      const Eigen::Vector3d& interior_direction,
      const Eigen::Vector2i& cell,
      CellGroup group_number);


  void AddPlatform(double x, double y, int height);

  void AddGlassColumn(double x, double y, int height);

  void Add(map_builder::Entity entity) {
    builder_.AddEntity(std::move(entity));
  }

  // Creates a light source.
  map_builder::Entity MakeLight(
      const Eigen::Vector3d& position,
      double intensity);

  // Creates a generic entity, scaling the position by the cell size.
  map_builder::Entity MakeEntity(
      const Eigen::Vector3d& position,
      std::string class_name,
      const std::vector<std::pair<std::string, std::string>>& attributes);

  // Creates a generic entity, scaling the position by the cell size
  // and adding an offset.
  map_builder::Entity MakeEntityWithRealOffset(
      const Eigen::Vector3d& position,
      const Eigen::Vector3d& offset,
      std::string class_name,
      const std::vector<std::pair<std::string, std::string>>& attributes);

  // Creates a brush entity with a given texture. To make the texture fit the
  // brush, set texture_scale_{width, height} to be the size of the brush
  // divided by the size of the texture. If they are set to zero, the default
  // scale is used.
  map_builder::Entity MakeBrushEntity(
      const Eigen::Vector3d& min,
      const Eigen::Vector3d& max,
      std::string class_name,
      std::string texture_name,
      double texture_scale_width,
      double texture_scale_height,
      const std::vector<std::pair<std::string, std::string>>& attributes);

  // Creates a brush entity with a given texture. The texture is fitted to the
  // brush according to its size.
  map_builder::Entity MakeFittedBrushEntity(
      const Eigen::Vector3d& min,
      const Eigen::Vector3d& max,
      std::string class_name,
      const std::string& texture_name,
      double texture_width,
      double texture_height,
      const std::vector<std::pair<std::string, std::string>>& attributes);

  // Creates a brush entity with a series of blocks and a given texture. To make
  // the texture fit the brush, set texture_scale_{width, height} to be the size
  // of the brush divided by the size of the texture. If they are set to zero,
  // the default scale is used.
  map_builder::Entity MakeBrushEntity(
      const std::vector<std::pair<Eigen::Vector3d, Eigen::Vector3d>>& blocks,
      std::string class_name,
      std::string texture_name,
      double texture_scale_width,
      double texture_scale_height,
      const std::vector<std::pair<std::string, std::string>>& attributes);

 private:
  using  WallArtLocation = Theme::WallArtLocation;
  using  FloorArtLocation = Theme::FloorArtLocation;

  // Set of textures for a theme.
  struct TextureSet {
    struct MazeModel {
      std::string name;
      double scale;
    };

    std::vector<map_builder::Texture> floor;
    std::vector<map_builder::Texture> ceiling;
    std::vector<map_builder::Texture> wall;
    std::vector<map_builder::Texture> wall_decals;
    std::vector<map_builder::Texture> brick;
    std::vector<MazeModel> floor_models;
  };

  map_builder::Patch GenerateWallDecal(
      const Theme::Texture& texture,
      const Eigen::Vector3d& a,
      const Eigen::Vector3d& b,
      const Eigen::Vector3d& interior_direction);

  TextLevelSettings* settings_;

  std::map<CellGroup, map_builder::Texture> floor_textures_;
  std::map<CellGroup, map_builder::Texture> ceiling_textures_;
  map_builder::Texture riser_texture_;
  map_builder::Texture tread_texture_;
  std::array<std::map<CellGroup, map_builder::Texture>, 4> wall_textures_;

  map_builder::Builder builder_;
  TextureSet texture_set_;
  std::vector<WallArtLocation> wall_art_locations_;

  std::vector<FloorArtLocation> floor_art_locations_;
  Eigen::Vector3d bounding_box_size_;

  map_builder::Texture glass_texture_{kGlassTexture, {0, 0}, 0, {0, 0}};
};

}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_LEVEL_GENERATION_TEXT_LEVEL_TEXT_LEVEL_EXPORTER_H_
