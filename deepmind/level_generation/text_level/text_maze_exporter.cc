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

#include "deepmind/level_generation/text_level/text_maze_exporter.h"

#include "deepmind/support/logging.h"
#include "deepmind/support/stringprintf.h"
#include "deepmind/support/str_cat.h"
#include "Eigen/Geometry"
#include "deepmind/level_generation/map_builder/entity.h"

namespace deepmind {
namespace lab {
namespace {

// Scale the textures to match the 100 units cell size.
constexpr double kTexelSize = 100.0;

// Default size used for skybox.
const Eigen::Vector2i kSkyboxTextureSize = {1024, 1024};

struct TextureFile {
  std::string name;
  int width;
  int height;
};

std::vector<TextureFile> BuildDecalList(
    const std::string& filename,
    int num_decals,
    int width,
    int height) {
  std::vector<TextureFile> files;
  for (int i = 0; i < num_decals; i++) {
    files.push_back(
        {StringPrintf("%s_%03d", filename.c_str(), i + 1), width, height});
  }
  return files;
}

std::vector<TextureFile> JoinDecalLists(
    const std::vector<std::vector<TextureFile>>& lists) {
  std::vector<TextureFile> files;
  for (const auto& list : lists) {
    for (const auto& file : list) {
      files.push_back(file);
    }
  }
  return files;
}

std::vector<map_builder::Texture> BuildTextures(
    const std::string& prefix,
    const std::vector<TextureFile>& files) {
  std::vector<map_builder::Texture> textures;
  for (const auto& file : files) {
    textures.push_back(map_builder::Texture{
        prefix + "/" + file.name,
        {0, 0},
        0,
        {kTexelSize / file.width, kTexelSize / file.height}});
  }
  return textures;
}

}  // namespace

TextMazeExporter::TextMazeExporter(
    std::mt19937_64* rng,
    const Settings& settings)
    : rng_(rng),
      settings_(settings) {
  builder_.mutable_world_entity()->set_attribute("light", StrCat(100));
  builder_.mutable_world_entity()->set_attribute("worldtype", StrCat(2));

  // Shared decal textures among all themes.
  auto decal_textures = BuildTextures(
      "decal/lab_games",
      JoinDecalLists({BuildDecalList("dec_img_style01", 20, 1024, 1024),
                      BuildDecalList("dec_img_style02", 20, 1024, 1024),
                      BuildDecalList("dec_img_style03", 20, 1024, 1024),
                      BuildDecalList("dec_img_style04", 20, 1024, 1024)}));

  switch (settings_.theme) {
    case Theme::kDefault:
    case Theme::MISHMASH:
      texture_set_ = TextureSet{
          BuildTextures("map/lab_games",
                        {{"lg_style_01_floor_orange", 1024, 1024},
                         {"lg_style_01_floor_orange_bright", 1024, 1024},
                         {"lg_style_01_floor_blue", 1024, 1024},
                         {"lg_style_01_floor_blue_bright", 1024, 1024},
                         {"lg_style_02_floor_blue", 1024, 1024},
                         {"lg_style_02_floor_blue_bright", 1024, 1024},
                         {"lg_style_02_floor_green", 1024, 1024},
                         {"lg_style_02_floor_green_bright", 1024, 1024},
                         {"lg_style_03_floor_green", 1024, 1024},
                         {"lg_style_03_floor_green_bright", 1024, 1024},
                         {"lg_style_03_floor_blue", 1024, 1024},
                         {"lg_style_03_floor_blue_bright", 1024, 1024},
                         {"lg_style_04_floor_blue", 1024, 1024},
                         {"lg_style_04_floor_blue_bright", 1024, 1024},
                         {"lg_style_04_floor_orange", 1024, 1024},
                         {"lg_style_04_floor_orange_bright", 1024, 1024},
                         {"lg_style_05_floor_blue", 1024, 1024},
                         {"lg_style_05_floor_blue_bright", 1024, 1024},
                         {"lg_style_05_floor_orange", 1024, 1024},
                         {"lg_style_05_floor_orange_bright", 1024, 1024}}),

          BuildTextures("map/lab_games", {{"fake_sky", 1024, 1024}}),
          BuildTextures("map/lab_games",
                        {{"lg_style_01_wall_green", 1024, 1024},
                         {"lg_style_01_wall_green_bright", 1024, 1024},
                         {"lg_style_01_wall_red", 1024, 1024},
                         {"lg_style_01_wall_red_bright", 1024, 1024},
                         {"lg_style_02_wall_yellow", 1024, 1024},
                         {"lg_style_02_wall_yellow_bright", 1024, 1024},
                         {"lg_style_02_wall_blue", 1024, 1024},
                         {"lg_style_02_wall_blue_bright", 1024, 1024},
                         {"lg_style_03_wall_orange", 1024, 1024},
                         {"lg_style_03_wall_orange_bright", 1024, 1024},
                         {"lg_style_03_wall_gray", 1024, 1024},
                         {"lg_style_03_wall_gray_bright", 1024, 1024},
                         {"lg_style_04_wall_green", 1024, 1024},
                         {"lg_style_04_wall_green_bright", 1024, 1024},
                         {"lg_style_04_wall_red", 1024, 1024},
                         {"lg_style_04_wall_red_bright", 1024, 1024},
                         {"lg_style_05_wall_red", 1024, 1024},
                         {"lg_style_05_wall_red_bright", 1024, 1024},
                         {"lg_style_05_wall_yellow", 1024, 1024},
                         {"lg_style_05_wall_yellow_bright", 1024, 1024}}),
          decal_textures,
      };
      break;
    case Theme::TRON:
      texture_set_ = TextureSet{
          BuildTextures("map/lab_games",
                        {{"lg_style_01_floor_orange", 1024, 1024},
                         {"lg_style_01_floor_orange_bright", 1024, 1024},
                         {"lg_style_01_floor_blue", 1024, 1024},
                         {"lg_style_01_floor_blue_bright", 1024, 1024}}),
          BuildTextures("map/lab_games", {{"fake_sky", 1024, 1024}}),
          BuildTextures("map/lab_games",
                        {{"lg_style_01_wall_green", 1024, 1024},
                         {"lg_style_01_wall_green_bright", 1024, 1024},
                         {"lg_style_01_wall_red", 1024, 1024},
                         {"lg_style_01_wall_red_bright", 1024, 1024}}),
          decal_textures,
      };
      break;
    case Theme::MINESWEEPER:
      texture_set_ = TextureSet{
          BuildTextures("map/lab_games",
                        {{"lg_style_04_floor_blue", 1024, 1024},
                         {"lg_style_04_floor_blue_bright", 1024, 1024},
                         {"lg_style_04_floor_orange", 1024, 1024},
                         {"lg_style_04_floor_orange_bright", 1024, 1024}}),
          BuildTextures("map/lab_games", {{"fake_sky", 1024, 1024}}),
          BuildTextures("map/lab_games",
                        {{"lg_style_04_wall_green", 1024, 1024},
                         {"lg_style_04_wall_green_bright", 1024, 1024},
                         {"lg_style_04_wall_red", 1024, 1024},
                         {"lg_style_04_wall_red_bright", 1024, 1024}}),
          decal_textures,
          {{"fut_obj_barbell_01.md3", 1.0}, {"fut_obj_cylinder_01.md3", 1.0}},
      };
      break;
    case Theme::TETRIS:
      texture_set_ = TextureSet{
          BuildTextures("map/lab_games",
                        {{"lg_style_02_floor_blue", 1024, 1024},
                         {"lg_style_02_floor_blue_bright", 1024, 1024},
                         {"lg_style_02_floor_green", 1024, 1024},
                         {"lg_style_02_floor_green_bright", 1024, 1024}}),
          BuildTextures("map/lab_games", {{"fake_sky", 1024, 1024}}),
          BuildTextures("map/lab_games",
                        {{"lg_style_02_wall_yellow", 1024, 1024},
                         {"lg_style_02_wall_yellow_bright", 1024, 1024},
                         {"lg_style_02_wall_blue", 1024, 1024},
                         {"lg_style_02_wall_blue_bright", 1024, 1024}}),
          decal_textures,
      };
      break;
    case Theme::GO:
      texture_set_ = TextureSet{
          BuildTextures("map/lab_games",
                        {{"lg_style_03_floor_green", 1024, 1024},
                         {"lg_style_03_floor_green_bright", 1024, 1024},
                         {"lg_style_03_floor_blue", 1024, 1024},
                         {"lg_style_03_floor_blue_bright", 1024, 1024}}),
          BuildTextures("map/lab_games", {{"fake_sky", 1024, 1024}}),
          BuildTextures("map/lab_games",
                        {{"lg_style_03_wall_orange", 1024, 1024},
                         {"lg_style_03_wall_orange_bright", 1024, 1024},
                         {"lg_style_03_wall_gray", 1024, 1024},
                         {"lg_style_03_wall_gray_bright", 1024, 1024}}),
          decal_textures,
          {{"fut_obj_barbell_01.md3", 1.0},
           {"fut_obj_coil_01.md3", 1.0},
           {"fut_obj_cone_01.md3", 1.0},
           {"fut_obj_crossbar_01.md3", 1.0},
           {"fut_obj_cube_01.md3", 1.0},
           {"fut_obj_cylinder_01.md3", 1.0},
           {"fut_obj_doubleprism_01.md3", 1.0},
           {"fut_obj_glowball_01.md3", 1.0}},
          {{"fut_obj_toroid_01.md3", 1.0},
           {"fut_obj_toroid_02.md3", 1.0},
           {"fut_obj_toroid_03.md3", 1.0}}};
      break;
    case Theme::PACMAN:
      texture_set_ = TextureSet{
          BuildTextures("map/lab_games",
                        {{"lg_style_05_floor_blue", 1024, 1024},
                         {"lg_style_05_floor_blue_bright", 1024, 1024},
                         {"lg_style_05_floor_orange", 1024, 1024},
                         {"lg_style_05_floor_orange_bright", 1024, 1024}}),
          BuildTextures("map/lab_games", {{"fake_sky", 1024, 1024}}),
          BuildTextures("map/lab_games",
                        {{"lg_style_05_wall_red", 1024, 1024},
                         {"lg_style_05_wall_red_bright", 1024, 1024},
                         {"lg_style_05_wall_yellow", 1024, 1024},
                         {"lg_style_05_wall_yellow_bright", 1024, 1024}}),
          decal_textures,
          {{"fut_obj_toroid_01.md3", 1.0},
           {"fut_obj_cylinder_01.md3", 1.0},
           {"fut_obj_crossbar_01.md3", 1.0},
           {"fut_obj_cube_01.md3", 1.0}},
      };
      break;
  }

  // Always use the first available texture for group 0, the group used for the
  // corridors. This ensures that the overall appearance of the maze can be
  // controlled manually, while still randomly sampling the decoration of
  // smaller elements (e.g. rooms).
  floor_textures_[0] = texture_set_.floor[0];
  ceiling_textures_[0] = texture_set_.ceiling[0];
  wall_textures_[0] = texture_set_.wall[0];
}

void TextMazeExporter::SetBoundingBox(Eigen::Vector3d size) {
  bounding_box_size_ = size;
  bounding_box_size_.z() *= settings_.ceiling_scale;
}

map_builder::Patch TextMazeExporter::GenerateWallDecal(
    const std::string& texture,
    double size,
    Eigen::Vector3d a,
    Eigen::Vector3d b,
    Eigen::Vector3d interior_direction) {
  const Eigen::Vector3d kUp = {0, 0, 1};
  Eigen::Vector3d pos = (a + b) * (0.5 * settings_.cell_size);
  // Push decal slightly away from wall.
  pos += (2.0 / map_builder::kWorldToGameUnits) * interior_direction;
  return map_builder::brush_util::CreateGridPatch(
      pos, -interior_direction, kUp, {size, size}, {3, 3}, {texture});
}

map_builder::Patch TextMazeExporter::GenerateWallDecal(
    std::size_t texture_index,
    Eigen::Vector3d a,
    Eigen::Vector3d b,
    Eigen::Vector3d interior_direction) {
  const auto& texture = texture_set_.wall_decals[texture_index];
  double size = settings_.cell_size * (0.25 + real_dist(*rng_) * 0.5);
  return GenerateWallDecal(texture.path, size, a, b, interior_direction);
}

map_builder::Entity TextMazeExporter::GenerateWallModel(
    std::size_t model_index,
    Eigen::Vector3d a,
    Eigen::Vector3d b,
    Eigen::Vector3d interior_direction) {
  const auto& model = texture_set_.wall_models[model_index];

  using map_builder::Angle;

  auto angle = [](const Eigen::Vector3d& x, const Eigen::Vector3d& y) -> Angle {
    return Angle::Radians(std::atan2(x.cross(y).norm(), x.dot(y)));
  };

  return map_builder::Entity::CreateModel(
      StrCat("models/", model.name),
      ((a + b) / 2) * settings_.cell_size + interior_direction,
      {Angle::Radians(0), Angle::Radians(0), angle(interior_direction, Eigen::Vector3d(0, 1, 0))},
      {model.scale, model.scale, model.scale});
}

void TextMazeExporter::AddWall(
    Eigen::Vector3d a,
    Eigen::Vector3d b,
    Eigen::Vector3d interior_direction,
    const map_builder::Texture& texture) {
  builder_.mutable_world_entity()->add_brush(
      map_builder::brush_util::CreateBoxBrush(
          a * settings_.cell_size + Eigen::Vector3d::Zero().cwiseMin(interior_direction) / map_builder::kWorldToGameUnits,
          b * settings_.cell_size + Eigen::Vector3d::Zero().cwiseMax(interior_direction) / map_builder::kWorldToGameUnits,
          texture));

  art_locations_.push_back(ArtLocation{a, b, interior_direction});
}

void TextMazeExporter::AddWall(
    Eigen::Vector3d a,
    Eigen::Vector3d b,
    Eigen::Vector3d interior_direction,
    CellGroup group_number) {
  auto wall_tex_it = wall_textures_.find(group_number);
  if (wall_tex_it == wall_textures_.end()) {
    wall_tex_it = wall_textures_.emplace(
        group_number,
        texture_set_.wall[Unbiased(texture_set_.wall.size())]).first;
  }
  AddWall(a, b, interior_direction, wall_tex_it->second);
}

void TextMazeExporter::AddFloor(
    Eigen::Vector3d a,
    Eigen::Vector3d b,
    CellGroup group_number) {
  // Walls must have a thickness > 0 to be rendered.
  Eigen::Vector3d thickness_offset(0, 0, (1.0 / map_builder::kWorldToGameUnits));

  if (floor_textures_.find(group_number) == floor_textures_.end()) {
    floor_textures_[group_number] =
        texture_set_.floor[Unbiased(texture_set_.floor.size())];
    ceiling_textures_[group_number] =
        texture_set_.ceiling[Unbiased(texture_set_.ceiling.size())];
  }

  auto* world_entity = builder_.mutable_world_entity();

  world_entity->add_brush(map_builder::brush_util::CreateBoxBrush(
      a * settings_.cell_size, b * settings_.cell_size + thickness_offset,
      floor_textures_[group_number]));

  // Don't add ceiling if we have a skybox set.
  if (settings_.skybox_texture_name.empty()) {
    Eigen::Vector3d ceiling_height(0, 0, bounding_box_size_.z());
    world_entity->add_brush(map_builder::brush_util::CreateBoxBrush(
        (a + ceiling_height) * settings_.cell_size - thickness_offset,
        (b + ceiling_height) * settings_.cell_size,
        ceiling_textures_[group_number]));
  }

  if (!texture_set_.floor_models.empty() &&
      real_dist(*rng_) < settings_.floor_object_frequency) {
    auto model =
        texture_set_.floor_models[Unbiased(texture_set_.floor_models.size())];
    Add(MakeEntityWithRealOffset(
        (a + b) / 2, Eigen::Vector3d(0, 0, 1), "misc_model",
        {{"model", "models/" + model.name},
         {"angle", StrCat(Unbiased(360))},
         {"modelscale", StrCat(model.scale)}}));
  }
}

map_builder::Entity TextMazeExporter::MakeEntity(
    Eigen::Vector3d position,
    std::string class_name,
    std::vector<std::pair<std::string, std::string>> attributes) {
  map_builder::Entity entity(std::move(class_name),
                             position * settings_.cell_size);
  entity.set_attributes(attributes);
  return entity;
}

map_builder::Entity TextMazeExporter::MakeEntityWithRealOffset(
    Eigen::Vector3d position,
    Eigen::Vector3d offset,
    std::string class_name,
    std::vector<std::pair<std::string, std::string>> attributes) {
  map_builder::Entity entity(std::move(class_name),
                             position * settings_.cell_size + offset);
  entity.set_attributes(attributes);
  return entity;
}

map_builder::Entity TextMazeExporter::MakeBrushEntity(
    Eigen::Vector3d min,
    Eigen::Vector3d max,
    std::string class_name,
    std::string texture_name,
    double texture_scale_width,
    double texture_scale_height,
    std::vector<std::pair<std::string, std::string>> attributes) {
  map_builder::Entity entity(std::move(class_name));
  map_builder::Texture texture =
      texture_name.empty()
          ? floor_textures_[0]
          : map_builder::Texture{std::move(texture_name), {0, 0}, 0,
                                 {kTexelSize * texture_scale_width,
                                  kTexelSize * texture_scale_height}};
  entity.add_brush(map_builder::brush_util::CreateBoxBrush(
      settings_.cell_size * min, settings_.cell_size * max, texture));
  entity.set_attributes(attributes);
  return entity;
}

map_builder::Entity TextMazeExporter::MakeLight(
    Eigen::Vector3d position,
    double intensity) {
  return MakeEntity(
      position, "light",
      // Default light brightness is 200.
      {{"light", StrCat(intensity * 5 * settings_.light_intensity)},
        {"style", "0"},
        {"spawnflags", "0"}});
}

void TextMazeExporter::Finalize() {
  int total_pieces =
      texture_set_.wall_decals.size() + texture_set_.wall_models.size();

  std::vector<std::pair<std::size_t, bool>> pieces;
  pieces.reserve(total_pieces);
  for (std::size_t i = 0; i < texture_set_.wall_decals.size(); ++i) {
    pieces.emplace_back(i, true);
  }
  for (std::size_t i = 0; i < texture_set_.wall_models.size(); ++i) {
    pieces.emplace_back(i, false);
  }

  // Uniform selection of pieces.
  std::shuffle(pieces.begin(), pieces.end(), *rng_);

  // Uniform distribution of placement.
  std::shuffle(art_locations_.begin(), art_locations_.end(), *rng_);

  std::size_t requested_pieces_count =
      art_locations_.size() * settings_.wall_decal_frequency + 0.5;

  if (requested_pieces_count > pieces.size()) {
    LOG(WARNING) << "Not enough paintings to match requested density:\n"
                 << settings_.wall_decal_frequency << "("
                 << requested_pieces_count << ") requested, "
                 << static_cast<double>(pieces.size()) / art_locations_.size()
                 << "(" << pieces.size() << ") achieved.";
    requested_pieces_count = pieces.size();
  } else {
    LOG(INFO) << "Requested wall hangings: " << requested_pieces_count;
  }

  for (std::size_t i = 0; i < requested_pieces_count; ++i) {
    const auto& piece = pieces[i];
    const auto& loc = art_locations_[i];
    if (piece.second) {
      builder_.mutable_world_entity()->add_patch(
          GenerateWallDecal(piece.first, loc.a, loc.b, loc.interior_direction));
    } else {
      builder_.AddEntity(
          GenerateWallModel(piece.first, loc.a, loc.b, loc.interior_direction));
    }
  }

  // Add skybox.
  if (!settings_.skybox_texture_name.empty()) {
    Eigen::Vector3d cell_size = Eigen::Vector3d::Constant(settings_.cell_size);
    auto world_size = (bounding_box_size_ * settings_.cell_size) + cell_size;
    auto pos = (world_size - cell_size) * 0.5;
    builder_.AddSkybox(pos, world_size, settings_.skybox_texture_name,
                       kSkyboxTextureSize);
  }
}

}  // namespace lab
}  // namespace deepmind
