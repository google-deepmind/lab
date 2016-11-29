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

#include "deepmind/support/str_cat.h"
#include "deepmind/support/str_join.h"
#include "deepmind/level_generation/map_builder/entity.h"
#include "deepmind/level_generation/text_level/grid_maze.h"
#include "deepmind/level_generation/text_level/parse_text_level.h"
#include "deepmind/level_generation/text_level/text_maze_exporter.h"

namespace deepmind {
namespace lab {
namespace {

// Whether all bits of rhs are set in lhs.
inline bool DirectionHas(
    GridMaze::Direction lhs,
    GridMaze::Direction rhs) {
  return (static_cast<unsigned char>(lhs) & static_cast<unsigned char>(rhs))
      == static_cast<unsigned char>(rhs);
}

// Spawn point elevation.
constexpr double kStartPointZOffset = 0.3;

// Door dimensions
constexpr double kDoorThickness = 0.01;
constexpr double kDoorSurround = 0.01;

// Main translation function; creates the core level geometry.
void TranslateCoreLevel(const GridMaze& maze, std::vector<std::string>* out,
                        TextMazeExporter* exporter, std::mt19937_64* rng) {
  using Dir = GridMaze::Direction;

  // Light sources are added periodically with random strength.
  constexpr std::size_t kLightSpacing = 2;
  constexpr double kLightZOffset = 0.7;

  // Note: This is what the original code did, not necessarily what we intended.
  std::uniform_int_distribution<int> light_dist(10, 19);

  exporter->SetBoundingBox({maze.width() * 1.0, maze.height() * 1.0, 1.0});
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

      if (!DirectionHas(cell.opening, Dir::kNorth)) {
        exporter->AddWall({x + 0.0, y + 1.0, 0.0},
                          {x + 1.0, y + 1.0, 1.0},
                          {    0.0,    -1.0, 0.0},
                          cell.variation);
      }

      if (!DirectionHas(cell.opening, Dir::kEast)) {
        exporter->AddWall({x + 1.0, y + 0.0, 0.0},
                          {x + 1.0, y + 1.0, 1.0},
                          {   -1.0,     0.0, 0.0},
                          cell.variation);
      }

      if (!DirectionHas(cell.opening, Dir::kSouth)) {
        exporter->AddWall({x + 0.0, y + 0.0, 0.0},
                          {x + 1.0, y + 0.0, 1.0},
                          {    0.0,     1.0, 0.0},
                          cell.variation);
      }

      if (!DirectionHas(cell.opening, Dir::kWest)) {
        exporter->AddWall({x + 0.0, y + 0.0, 0.0},
                          {x + 0.0, y + 1.0, 1.0},
                          {    1.0,     0.0, 0.0},
                          cell.variation);
      }

      exporter->AddFloor({x, y, 0.0}, {x + 1.0, y + 1.0, 0.0}, cell.variation);

      if ((maze.height() - i - 1 + j) % kLightSpacing == 0) {
        exporter->Add(exporter->MakeLight({(x + 0.5), (y + 0.5), kLightZOffset},
                                          light_dist(*rng)));
      }
    });
}

std::vector<map_builder::Entity> MakeDoor(
    double x, double y, char dir, TextMazeExporter* exporter) {
  std::vector<map_builder::Entity> door;
  std::string target = StrCat("door_", x, "_", y);

  const double texture_scale = (1.0 + 2.0 * kDoorSurround) / 1024.0;
  if (dir == 'H') {
    door.push_back(exporter->MakeBrushEntity(
        {x + 0.0 + kDoorSurround, y + 0.5 - kDoorThickness, 0.0},
        {x + 1.0 - kDoorSurround, y + 0.5 + kDoorThickness, 1.0},
        "func_door", "map/fut_door_d", texture_scale, texture_scale,
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
        {x + 0.5 - kDoorThickness, y + 0.0 + kDoorSurround, 0.0},
        {x + 0.5 + kDoorThickness, y + 1.0 - kDoorSurround, 1.0},
        "func_door", "map/fut_door_d", texture_scale, texture_scale,
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
    std::size_t i, std::size_t j, char val,
    const MapSnippetEmitter& emitter, std::vector<std::string>* out) {
  switch (val) {
    default:
      return false;

    case 'P':
      out->push_back(emitter.AddSpawn(i, j, 0));
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
  MapSnippetEmitterImpl(const GridMaze* maze, TextMazeExporter* exporter)
      : maze_(*maze), exporter_(exporter) {}

  std::string AddEntity(
      std::size_t i, std::size_t j, std::string class_name,
      const std::vector<std::pair<std::string, std::string>>&
      attributes) const {
    return exporter_
        ->MakeEntity(Eigen::Vector3d({j + 0.5, (maze_.height() - i - 1) + 0.5,
                                      kStartPointZOffset}),
                     std::move(class_name), attributes)
        .ToString();
  }

  std::string AddSpawn(std::size_t i, std::size_t j, double angle_rad) const {
    return map_builder::Entity::CreateSpawn(
        Eigen::Vector3d(
            {j + 0.5, (maze_.height() - i - 1) + 0.5, kStartPointZOffset})
        * exporter_->GetSettings().cell_size,
        map_builder::Angle::Radians(angle_rad)).ToString();
  }

  std::string AddDoor(std::size_t i, std::size_t j, char direction) const {
    return strings::Join(
        MakeDoor(j, maze_.height() - i - 1, direction, exporter_), "\n\n",
        [](string* out, const map_builder::Entity& ent) {
          StrAppend(out, ent.ToString());
        });
  }

 private:
  friend class MapSnippetEmitter;

  const GridMaze& maze_;
  TextMazeExporter* const exporter_;
};

}  // namespace

std::string MapSnippetEmitter::AddEntity(
    std::size_t i,
    std::size_t j,
    std::string class_name,
    const std::vector<std::pair<std::string, std::string>>& attributes) const {
  return static_cast<const MapSnippetEmitterImpl*>(this)->AddEntity(
      i, j, std::move(class_name), attributes);
}

std::string MapSnippetEmitter::AddSpawn(
    std::size_t i, std::size_t j, double angle_rad) const {
  return static_cast<const MapSnippetEmitterImpl*>(this)->AddSpawn(
      i, j, angle_rad);
}

std::string MapSnippetEmitter::AddDoor(
    std::size_t i, std::size_t j, char direction) const {
  return static_cast<const MapSnippetEmitterImpl*>(this)->AddDoor(
      i, j, direction);
}

std::string TranslateTextLevel(
    std::string level_text, std::string variations_text,
    std::mt19937_64* rng,
    const TranslateTextLevelCallback& callback) {
  GridMaze maze =
      ParseTextLevel(std::move(level_text), std::move(variations_text));

  TextMazeExporter exporter(rng);

  MapSnippetEmitterImpl emitter(&maze, &exporter);

  std::vector<std::string> lines, extra_lines;
  TranslateCoreLevel(maze, &lines, &exporter, rng);

  exporter.Finalize();

  maze.Visit([&](std::size_t i, std::size_t j, const GridMaze::Cell& cell) {
      if (cell.value == cell.kInaccessible) return;

      std::vector<std::string> v;

      if (callback(i, j, cell.value, emitter, &v) ||
          DefaultHandler(i, j, cell.value, emitter, &v)) {
        std::move(v.begin(), v.end(), std::back_inserter(extra_lines));
      }
  });

  lines.push_back(exporter.ToString());
  std::move(extra_lines.begin(), extra_lines.end(), std::back_inserter(lines));

  return std::accumulate(
      lines.begin(), lines.end(), std::string(),
      [](std::string& acc, const std::string& val) -> std::string {
        acc.append(val);
        acc.append(2, '\n');
        return std::move(acc);
      });
}

}  // namespace lab
}  // namespace deepmind
