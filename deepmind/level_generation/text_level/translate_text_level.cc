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

#include "deepmind/level_generation/text_level/translate_text_level.h"

#include <algorithm>
#include <iterator>
#include <utility>
#include <vector>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "deepmind/level_generation/map_builder/entity.h"
#include "deepmind/level_generation/text_level/grid_maze.h"
#include "deepmind/level_generation/text_level/parse_text_level.h"
#include "deepmind/level_generation/text_level/text_level_exporter.h"

namespace deepmind {
namespace lab {
namespace {

// Whether all bits of rhs are set in lhs.
inline bool DirectionHas(GridMaze::Direction lhs, GridMaze::Direction rhs) {
  return (static_cast<unsigned char>(lhs) & static_cast<unsigned char>(rhs))
      == static_cast<unsigned char>(rhs);
}

// Spawn point elevation.
constexpr double kStartPointZOffset = 0.3;

// Door dimensions.
constexpr double kDoorThickness = 0.01;
constexpr double kFenceDoorThickness = 0.04;
constexpr double kFenceDoorPosts = 4;
constexpr double kDoorSurround = 0.01;

// Main translation function; creates the core level geometry.
void TranslateCoreLevel(const GridMaze& maze, TextLevelExporter* exporter,
                        std::mt19937_64* rng,
                        TextLevelSettings* level_settings) {
  using Dir = GridMaze::Direction;

  // Light sources are added periodically with random strength.
  constexpr std::size_t kLightSpacing = 2;
  constexpr double kLightZOffset = 0.7;

  // Note: This is what the original code did, not necessarily what we intended.
  std::uniform_int_distribution<int> light_dist(10, 19);

  exporter->SetBoundingBox({maze.width() * 1.0, maze.height() * 1.0,
                            level_settings->ceiling_height});

  if (level_settings->draw_default_layout) {
    maze.Visit([&](std::size_t i, std::size_t j, const GridMaze::Cell& cell) {
      if (cell.value == cell.kInaccessible) return;

      // The .map format uses a bottom-left based coordinate system.
      //
      // We need to convert from our own top-left based (i, j) coordinates to
      // bottom-left based (x, y) coordinates:
      //     x <=> j
      //     y <=> height - i - 1
      //
      //  (i, j) = (I + 0, J + 0)        (i, j) = (I + 0, J + 1)
      //  (x, y) = (J + 0, h - I)        (x, y) = (J + 1, h - I)
      //         +------------------------------+
      //         |                              |
      //         |          cell (I, J)         |
      //         |                              |
      //         +------------------------------+
      //  (x, y) = (J + 0, h - I - 1)    (x, y) = (J + 1, h - I - 1)
      //  (i, j) = (I + 1, J + 0)        (i, j) = (I + 1, J + 1)
      //
      // For every cardinal direction in which there is no opening, we create a
      // wall across the corresponding cube face.

      double x = j;
      double y = maze.height() - i - 1;
      Eigen::Vector2i cell_location(i + 1, j + 1);

      if (!DirectionHas(cell.opening, Dir::kNorth)) {
        exporter->AddWall({x + 0.0, y + 1.0, 0.0}, {x + 1.0, y + 1.0, 1.0},
                          {0.0, -1.0, 0.0}, cell_location, cell.variation);
      }

      if (!DirectionHas(cell.opening, Dir::kEast)) {
        exporter->AddWall({x + 1.0, y + 0.0, 0.0}, {x + 1.0, y + 1.0, 1.0},
                          {-1.0, 0.0, 0.0}, cell_location, cell.variation);
      }

      if (!DirectionHas(cell.opening, Dir::kSouth)) {
        exporter->AddWall({x + 0.0, y + 0.0, 0.0}, {x + 1.0, y + 0.0, 1.0},
                          {0.0, 1.0, 0.0}, cell_location, cell.variation);
      }

      if (!DirectionHas(cell.opening, Dir::kWest)) {
        exporter->AddWall({x + 0.0, y + 0.0, 0.0}, {x + 0.0, y + 1.0, 1.0},
                          {1.0, 0.0, 0.0}, cell_location, cell.variation);
      }

      exporter->AddFloor({x, y, 0.0}, {x + 1.0, y + 1.0, 0.0}, cell_location,
                         cell.variation);

      if ((maze.height() - i - 1 + j) % kLightSpacing == 0) {
        exporter->Add(exporter->MakeLight({(x + 0.5), (y + 0.5), kLightZOffset},
                                          light_dist(*rng)));
      }
    });
  }
}

// The following two methods create fence door blocks. Fence doors consist of
// top and bottom blocks connected by at least three posts:
// Example door shape:
// -----  - Top block
// | | |  - Posts
// -----  - Bottom block

// Returns blocks representing a "horizontal" (oriented along x axis) fence
// door.
std::vector<std::pair<Eigen::Vector3d, Eigen::Vector3d>>
CreateHorizontalFenceDoorBlocks(double x, double y) {
  std::vector<std::pair<Eigen::Vector3d, Eigen::Vector3d>> blocks;
  // Top block
  blocks.push_back(std::pair<Eigen::Vector3d, Eigen::Vector3d>(
      {x + kDoorSurround, y + 0.5 - kFenceDoorThickness,
       1.0 - 2.0 * kFenceDoorThickness},
      {x + 1 + kDoorSurround, y + 0.5 + kFenceDoorThickness, 1.0}));
  // Bottom block
  blocks.push_back(std::pair<Eigen::Vector3d, Eigen::Vector3d>(
      {x + kDoorSurround, y + 0.5 - kFenceDoorThickness, 0},
      {x + 1 + kDoorSurround, y + 0.5 + kFenceDoorThickness,
       2.0 * kFenceDoorThickness}));
  // Post blocks
  const float post_step =
      (1.0 - 2.0 * kFenceDoorThickness - 2.0 * kDoorSurround) /
      (kFenceDoorPosts - 1);
  for (float px = x + kFenceDoorThickness + kDoorSurround; px < x + 1;
       px += post_step) {
    blocks.push_back(std::pair<Eigen::Vector3d, Eigen::Vector3d>(
        {px - kFenceDoorThickness, y + 0.5 - kFenceDoorThickness,
         2.0 * kFenceDoorThickness},
        {px + kFenceDoorThickness, y + 0.5 + kFenceDoorThickness,
         1.0 - 2.0 * kFenceDoorThickness}));
  }
  return blocks;
}

// Returns blocks representing a "vertical" (oriented along y axis) fence
// door.
std::vector<std::pair<Eigen::Vector3d, Eigen::Vector3d>>
CreateVerticalFenceDoorBlocks(double x, double y) {
  std::vector<std::pair<Eigen::Vector3d, Eigen::Vector3d>> blocks;
  // Top block
  blocks.push_back(std::pair<Eigen::Vector3d, Eigen::Vector3d>(
      {x + 0.5 - kFenceDoorThickness, y + kDoorSurround,
       1.0 - 2.0 * kFenceDoorThickness},
      {x + 0.5 + kFenceDoorThickness, y + 1 + kDoorSurround, 1.0}));
  // Bottom block
  blocks.push_back(std::pair<Eigen::Vector3d, Eigen::Vector3d>(
      {x + 0.5 - kFenceDoorThickness, y + kDoorSurround, 0},
      {x + 0.5 + kFenceDoorThickness, y + 1 + kDoorSurround,
       2.0 * kFenceDoorThickness}));
  // Post blocks
  const float post_step =
      (1.0 - 2.0 * kFenceDoorThickness - 2.0 * kDoorSurround) /
      (kFenceDoorPosts - 1);
  for (float py = y + kFenceDoorThickness + kDoorSurround; py < y + 1;
       py += post_step) {
    blocks.push_back(std::pair<Eigen::Vector3d, Eigen::Vector3d>(
        {x + 0.5 - kFenceDoorThickness, py - kFenceDoorThickness,
         2.0 * kFenceDoorThickness},
        {x + 0.5 + kFenceDoorThickness, py + kFenceDoorThickness,
         1.0 - 2.0 * kFenceDoorThickness}));
  }
  return blocks;
}

std::vector<map_builder::Entity> MakeDoor(
    double x, double y, char dir, TextLevelExporter* exporter) {
  std::vector<map_builder::Entity> door;
  std::string target = absl::StrCat("door_", x, "_", y);

  if (dir == 'H') {
    door.push_back(exporter->MakeFittedBrushEntity(
        {x + 0.0 + kDoorSurround, y + 0.5 - kDoorThickness, 0.0},
        {x + 1.0 - kDoorSurround, y + 0.5 + kDoorThickness, 1.0},
        "func_door", "map/fut_door_d", 1024, 1024,
        {{"angle", "0"}, {"targetname", target}}));

    door.push_back(exporter->MakeBrushEntity(
        {x + 0.0,                 y + 0.0,                  0.0},
        {x + 1.0,                 y + 0.5 - kDoorThickness, 1.0},
        "trigger_multiple", "", 0.0, 0.0,
        {{"wait", "1"}, {"target", target}}));

    door.push_back(exporter->MakeBrushEntity(
        {x + 0.0,                 y + 0.5 + kDoorThickness, 0.0},
        {x + 1.0,                 y + 1.0,                  1.0},
        "trigger_multiple", "", 0.0, 0.0,
        {{"wait", "1"}, {"target", target}}));
  } else if (dir == 'I') {
    door.push_back(exporter->MakeFittedBrushEntity(
        {x + 0.5 - kDoorThickness, y + 0.0 + kDoorSurround, 0.0},
        {x + 0.5 + kDoorThickness, y + 1.0 - kDoorSurround, 1.0},
        "func_door", "map/fut_door_d", 1024, 1024,
        {{"angle", "90"}, {"targetname", target}}));

    door.push_back(exporter->MakeBrushEntity(
        {x + 0.0,                  y + 0.0,                 0.0},
        {x + 0.5 - kDoorThickness, y + 1.0,                 1.0},
        "trigger_multiple", "", 0.0, 0.0,
        {{"wait", "1"}, {"target", target}}));

    door.push_back(exporter->MakeBrushEntity(
        {x + 0.5 + kDoorThickness, y + 0.0 ,                0.0},
        {x + 1.0,                  y + 1.0,                 1.0},
        "trigger_multiple", "", 0.0, 0.0,
        {{"wait", "1"}, {"target", target}}));
  }

  return door;
}

std::vector<map_builder::Entity> MakeFenceDoor(
    double x, double y, char dir, TextLevelExporter* exporter) {
  std::vector<map_builder::Entity> door;
  std::string target = absl::StrCat("door_", x, "_", y);

  const double texture_scale = (1.0 + 2.0 * kDoorSurround) / 1024.0;
  if (dir == 'H') {
    door.push_back(exporter->MakeBrushEntity(
        CreateHorizontalFenceDoorBlocks(x, y), "func_door",
        "door_placeholder:" + target, texture_scale, texture_scale,
        {{"angle", "0"}, {"targetname", target}}));

    door.push_back(exporter->MakeBrushEntity(
        {x + 0.0,                 y + 0.0,                  0.0},
        {x + 1.0,                 y + 0.5 - kDoorThickness, 1.0},
        "trigger_multiple", "", 0.0, 0.0,
        {{"wait", "1"}, {"target", target}}));

    door.push_back(exporter->MakeBrushEntity(
        {x + 0.0,                 y + 0.5 + kDoorThickness, 0.0},
        {x + 1.0,                 y + 1.0,                  1.0},
        "trigger_multiple", "", 0.0, 0.0,
        {{"wait", "1"}, {"target", target}}));
  } else if (dir == 'I') {
    door.push_back(exporter->MakeBrushEntity(
        CreateVerticalFenceDoorBlocks(x, y), "func_door",
        "door_placeholder:" + target, texture_scale, texture_scale,
        {{"angle", "90"}, {"targetname", target}}));

    door.push_back(exporter->MakeBrushEntity(
        {x + 0.0,                  y + 0.0,                 0.0},
        {x + 0.5 - kDoorThickness, y + 1.0,                 1.0},
        "trigger_multiple", "", 0.0, 0.0,
        {{"wait", "1"}, {"target", target}}));

    door.push_back(exporter->MakeBrushEntity(
        {x + 0.5 + kDoorThickness, y + 0.0 ,                0.0},
        {x + 1.0,                  y + 1.0,                 1.0},
        "trigger_multiple", "", 0.0, 0.0,
        {{"wait", "1"}, {"target", target}}));
  }

  return door;
}

// Ready-made callback for adding a number of generally useful entities.
bool DefaultHandler(
    double i, double j, char val,
    const MapSnippetEmitter& emitter, std::vector<std::string>* out) {
  switch (val) {
    default:
      return false;

    case 'P':
      out->push_back(emitter.AddSpawn(i, j, 0, 0));
      return true;

    case 'I':
      out->push_back(emitter.AddDoor(i, j, 'I'));
      return true;
    case 'H':
      out->push_back(emitter.AddDoor(i, j, 'H'));
      return true;
  }
}

class MapSnippetEmitterImpl : public MapSnippetEmitter {
 public:
  MapSnippetEmitterImpl(const GridMaze* maze, TextLevelExporter* exporter)
      : maze_(*maze), exporter_(exporter) {}

  std::string AddEntity(double i, double j, double height,
                        std::string class_name,
                        const std::vector<std::pair<std::string, std::string>>&
                            attributes) const {
    return exporter_
        ->MakeEntity(Eigen::Vector3d({j + 0.5, (maze_.height() - i - 1) + 0.5,
                                      height * TextLevelExporter::kHeightScale +
                                          kStartPointZOffset}),
                     std::move(class_name), attributes)
        .ToString();
  }

  std::string AddSpawn(double i, double j, double height,
                       double angle_rad) const {
    return map_builder::Entity::CreateSpawn(
               Eigen::Vector3d({j + 0.5, (maze_.height() - i - 1) + 0.5,
                                height * TextLevelExporter::kHeightScale +
                                    kStartPointZOffset}) *
                   exporter_->GetSettings().cell_size,
               map_builder::Angle::Radians(angle_rad))
        .ToString();
  }

  std::string AddDoor(double i, double j, char direction) const {
    return absl::StrJoin(
        MakeDoor(j, maze_.height() - i - 1, direction, exporter_), "\n\n",
        [](std::string* out, const map_builder::Entity& ent) {
          absl::StrAppend(out, ent.ToString());
        });
  }

  std::string AddFenceDoor(double i, double j, char direction) const {
    return absl::StrJoin(
        MakeFenceDoor(j, maze_.height() - i - 1, direction, exporter_), "\n\n",
        [](std::string* out, const map_builder::Entity& ent) {
          absl::StrAppend(out, ent.ToString());
        });
  }

  void AddPlatform(double i, double j, double height) const {
    exporter_->AddPlatform(j, maze_.height() - i - 1, height);
  }

  void AddGlassColumn(double i, double j, double height) const {
    exporter_->AddGlassColumn(j, maze_.height() - i - 1, height);
  }

 private:
  friend class MapSnippetEmitter;

  const GridMaze& maze_;
  TextLevelExporter* const exporter_;
};

}  // namespace

std::string MapSnippetEmitter::AddEntity(
    double i, double j, double height, std::string class_name,
    const std::vector<std::pair<std::string, std::string>>& attributes) const {
  return static_cast<const MapSnippetEmitterImpl*>(this)->AddEntity(
      i, j, height, std::move(class_name), attributes);
}

std::string MapSnippetEmitter::AddSpawn(double i, double j,
                                        double height, double angle_rad) const {
  return static_cast<const MapSnippetEmitterImpl*>(this)->AddSpawn(i, j, height,
                                                                   angle_rad);
}

std::string MapSnippetEmitter::AddDoor(
    double i, double j, char direction) const {
  return static_cast<const MapSnippetEmitterImpl*>(this)->AddDoor(
      i, j, direction);
}

std::string MapSnippetEmitter::AddFenceDoor(
    double i, double j, char direction) const {
  return static_cast<const MapSnippetEmitterImpl*>(this)->AddFenceDoor(
      i, j, direction);
}

std::string MapSnippetEmitter::AddPlatform(double i, double j,
                                           double height) const {
  static_cast<const MapSnippetEmitterImpl*>(this)->AddPlatform(i, j, height);
  return "";
}

std::string MapSnippetEmitter::AddGlassColumn(double i, double j,
                                              double height) const {
  static_cast<const MapSnippetEmitterImpl*>(this)->AddGlassColumn(i, j, height);
  return "";
}

std::string TranslateTextLevel(std::string level_text,
                               std::string variations_text,
                               std::mt19937_64* rng,
                               const TranslateTextLevelCallback& callback,
                               TextLevelSettings* level_settings) {
  GridMaze maze =
      ParseTextLevel(std::move(level_text), std::move(variations_text));

  TextLevelExporter exporter(level_settings);

  MapSnippetEmitterImpl emitter(&maze, &exporter);

  TranslateCoreLevel(maze, &exporter, rng, level_settings);

  exporter.Finalize();

  std::vector<std::string> lines;
  maze.Visit([&](std::size_t i, std::size_t j, const GridMaze::Cell& cell) {
      if (cell.value == cell.kInaccessible) return;

      std::vector<std::string> v;

      if (callback(i, j, cell.value, emitter, &v) ||
          DefaultHandler(i, j, cell.value, emitter, &v)) {
        std::move(v.begin(), v.end(), std::back_inserter(lines));
      }
  });

  // The exporter string is already newline-terminated, so only add 1 newline
  return absl::StrCat(exporter.ToString(), "\n", absl::StrJoin(lines, "\n\n"),
                      "\n\n");
}

}  // namespace lab
}  // namespace deepmind
