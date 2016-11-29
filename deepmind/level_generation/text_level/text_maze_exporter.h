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

#ifndef DML_DEEPMIND_LEVEL_GENERATION_TEXT_LEVEL_TEXT_MAZE_EXPORTER_H_
#define DML_DEEPMIND_LEVEL_GENERATION_TEXT_LEVEL_TEXT_MAZE_EXPORTER_H_

#include <cstddef>
#include <map>
#include <random>
#include <vector>

#include "Eigen/Core"
#include "deepmind/level_generation/map_builder/brush.h"
#include "deepmind/level_generation/map_builder/builder.h"
#include "deepmind/level_generation/map_builder/entity.h"

namespace deepmind {
namespace lab {

// Creates a Quake3 .map file from a given maze. Uses map_builder::Builder to
// build up and generate the actual file.
class TextMazeExporter {
 public:
  using CellGroup = int;

  enum class Theme {
    kDefault,
    TRON,
    MINESWEEPER,
    TETRIS,
    GO,
    PACMAN,
    MISHMASH,
  };

  struct Settings {
    Settings()
        : theme(Theme::kDefault),
          wall_decal_frequency(0.1),
          floor_object_frequency(0.05),
          cell_size(100.0 / map_builder::kWorldToGameUnits),
          ceiling_scale(1.0),
          light_intensity(1.0) {}

    Theme theme;
    std::string skybox_texture_name;
    double wall_decal_frequency;
    double floor_object_frequency;
    double cell_size;
    double ceiling_scale;
    double light_intensity;
  };

  explicit TextMazeExporter(
      std::mt19937_64* rng,
      const Settings& settings = Settings());

  const Settings& GetSettings() const { return settings_; }

  // Call Finalize() before calling ToString().
  void Finalize();

  // Returns the .map format output string.
  std::string ToString() const { return builder_.ToString(); }

  void SetBoundingBox(Eigen::Vector3d size);

  void AddFloor(
      Eigen::Vector3d a,
      Eigen::Vector3d b,
      CellGroup group_number);

  void AddWall(
      Eigen::Vector3d a,
      Eigen::Vector3d b,
      Eigen::Vector3d interior_direction,
      CellGroup group_number);

  void AddWall(
      Eigen::Vector3d a,
      Eigen::Vector3d b,
      Eigen::Vector3d interior_direction,
      const map_builder::Texture& texture);

  void Add(map_builder::Entity entity) {
    builder_.AddEntity(std::move(entity));
  }

  // Creates a light source.
  map_builder::Entity MakeLight(
      Eigen::Vector3d position,
      double intensity);

  // Creates a generic entity, scaling the position by the cell size.
  map_builder::Entity MakeEntity(
      Eigen::Vector3d position,
      std::string class_name,
      std::vector<std::pair<std::string, std::string>> attributes);

  // Creates a generic entity, scaling the position by the cell size
  // and adding an offset.
  map_builder::Entity MakeEntityWithRealOffset(
      Eigen::Vector3d position,
      Eigen::Vector3d offset,
      std::string class_name,
      std::vector<std::pair<std::string, std::string>> attributes);

  // Creates a brush entity with a given texture. To make the texture fit the
  // brush, set texture_scale_{width, height} to be the size of the brush
  // divided by the size of the texture. If they are set to zero, the default
  // scale is used.
  map_builder::Entity MakeBrushEntity(
      Eigen::Vector3d min,
      Eigen::Vector3d max,
      std::string class_name,
      std::string texture_name,
      double texture_scale_width,
      double texture_scale_height,
      std::vector<std::pair<std::string, std::string>> attributes);

 private:
  struct ArtLocation {
    Eigen::Vector3d a;
    Eigen::Vector3d b;
    Eigen::Vector3d interior_direction;
  };

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
    std::vector<MazeModel> floor_models;
    std::vector<MazeModel> wall_models;
  };

  map_builder::Patch GenerateWallDecal(
      const std::string& texture,
      double size,
      Eigen::Vector3d a,
      Eigen::Vector3d b,
      Eigen::Vector3d interior_direction);

  map_builder::Patch GenerateWallDecal(
      std::size_t texture_index,
      Eigen::Vector3d a,
      Eigen::Vector3d b,
      Eigen::Vector3d interior_direction);

  map_builder::Entity GenerateWallModel(
      std::size_t model_index,
      Eigen::Vector3d a,
      Eigen::Vector3d b,
      Eigen::Vector3d interior_direction);

  std::size_t Unbiased(std::size_t n) {
    return size_dist(
        *rng_,
        std::uniform_int_distribution<std::size_t>::param_type(0, n - 1));
  }

  std::mt19937_64* rng_;
  std::uniform_int_distribution<std::size_t> size_dist;
  std::uniform_real_distribution<double> real_dist;

  const Settings settings_;

  std::map<CellGroup, map_builder::Texture> floor_textures_;
  std::map<CellGroup, map_builder::Texture> ceiling_textures_;
  std::map<CellGroup, map_builder::Texture> wall_textures_;

  map_builder::Builder builder_;
  TextureSet texture_set_;
  std::vector<ArtLocation> art_locations_;
  Eigen::Vector3d bounding_box_size_;
};

}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_LEVEL_GENERATION_TEXT_LEVEL_TEXT_MAZE_EXPORTER_H_
