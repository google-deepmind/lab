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

#include "deepmind/level_generation/map_builder/brush.h"

#include <cstdio>

#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"
#include "Eigen/Geometry"

namespace deepmind {
namespace lab {
namespace map_builder {
namespace {

// Helper function to parse a single brush.
bool ParseSingleBrush(absl::string_view brush_str, Brush* brush) {
  for (absl::string_view line : absl::StrSplit(brush_str, '\n')) {
    line = absl::StripLeadingAsciiWhitespace(line);
    Plane p;

    char path[256];
    int flags;
    if (std::sscanf(
            line.data(),
            "( %lf %lf %lf ) ( %lf %lf %lf ) ( %lf %lf %lf ) %255s %d %d "
            "%lf %lf %lf %d 0 0",
            &p.a[0], &p.a[1], &p.a[2], &p.b[0], &p.b[1], &p.b[2], &p.c[0],
            &p.c[1], &p.c[2], path, &p.texture.offset[0], &p.texture.offset[1],
            &p.texture.rot_angle, &p.texture.scale[0], &p.texture.scale[1],
            &flags) == 16) {
      p.texture.path = path;
      p.texture.flags = static_cast<Texture::Flags>(flags);
      p.a /= kWorldToGameUnits;
      p.b /= kWorldToGameUnits;
      p.c /= kWorldToGameUnits;
      brush->planes.push_back(std::move(p));
    }
  }

  return !brush->planes.empty();
}

}  // namespace

std::string Texture::ToString() const {
  return absl::StrFormat("%.256s %d %d %g %g %g %d 0 0", path, offset.x(),
                         offset.y(), rot_angle, scale.x(), scale.y(),
                         static_cast<int>(flags));
}

std::string Plane::ToString() const {
  // Convert world units to game units.
  return absl::StrFormat("( %g %g %g ) ( %g %g %g ) ( %g %g %g ) %s",
                         a.x() * kWorldToGameUnits, a.y() * kWorldToGameUnits,
                         a.z() * kWorldToGameUnits, b.x() * kWorldToGameUnits,
                         b.y() * kWorldToGameUnits, b.z() * kWorldToGameUnits,
                         c.x() * kWorldToGameUnits, c.y() * kWorldToGameUnits,
                         c.z() * kWorldToGameUnits, texture.ToString());
}

std::string Brush::ToString() const {
  std::string brush = "{\n";
  for (const auto& plane : planes) {
    absl::StrAppend(&brush, "    ", plane.ToString(), "\n");
  }
  absl::StrAppend(&brush, "  }");
  return brush;
}

std::string PatchPoint::ToString() const {
  return absl::StrFormat("( %g %g %g %g %g )", pos.x() * kWorldToGameUnits,
                         pos.y() * kWorldToGameUnits,
                         pos.z() * kWorldToGameUnits, uv.x(), uv.y());
}

std::string Patch::ToString() const {
  std::string patch = absl::StrCat("{\n    patchDef2\n    {\n      ",
                                   texture_.path, "\n      ( ", grid_size_.x(),
                                   " ", grid_size_.y(), " 0 0 0 )\n");
  absl::StrAppend(&patch, "      (\n");
  for (int x = 0; x < grid_size_.x(); x++) {
    absl::StrAppend(&patch, "        (");
    for (int y = 0; y < grid_size_.y(); y++) {
      absl::StrAppend(&patch, " ", point({x, y}).ToString());
    }
    absl::StrAppend(&patch, " )\n");
  }
  absl::StrAppend(&patch, "      )\n    }\n  }");
  return patch;
}

namespace brush_util {

std::vector<Brush> ParseBrushes(absl::string_view brushes_str) {
  std::vector<Brush> brushes;

  int brush_start = -1;
  for (auto cur = brushes_str.find('{'); cur != brushes_str.npos;
       cur = brushes_str.find_first_of("{}", cur)) {
    if (brushes_str[cur] == '{') {
      brush_start = cur;
    } else if (brush_start != -1) {
      // End of current brush, parse its assets.
      Brush brush;
      if (ParseSingleBrush(absl::ClippedSubstr(brushes_str, brush_start,
                                               cur - brush_start + 1),
                           &brush)) {
        brushes.push_back(std::move(brush));
      }
      brush_start = -1;
    }
    ++cur;
  }
  return brushes;
}

Brush CreateBoxBrush(const Eigen::Vector3d& a, const Eigen::Vector3d& b,
                     const Texture& texture) {
  auto min_pos = a.cwiseMin(b);
  auto max_pos = a.cwiseMax(b);
  Brush brush;
  brush.planes.emplace_back(Plane{
      {min_pos.x(), 0, 0}, {min_pos.x(), 1, 0}, {min_pos.x(), 0, 1}, texture});
  brush.planes.emplace_back(Plane{
      {max_pos.x(), 0, 0}, {max_pos.x(), 0, 1}, {max_pos.x(), 1, 0}, texture});
  brush.planes.emplace_back(Plane{
      {0, min_pos.y(), 0}, {0, min_pos.y(), 1}, {1, min_pos.y(), 0}, texture});
  brush.planes.emplace_back(Plane{
      {0, max_pos.y(), 0}, {1, max_pos.y(), 0}, {0, max_pos.y(), 1}, texture});
  brush.planes.emplace_back(Plane{
      {0, 0, min_pos.z()}, {1, 0, min_pos.z()}, {0, 1, min_pos.z()}, texture});
  brush.planes.emplace_back(Plane{
      {0, 0, max_pos.z()}, {0, 1, max_pos.z()}, {1, 0, max_pos.z()}, texture});

  return brush;
}

Brush CreateFittedBoxBrush(const Eigen::Vector3d& a, const Eigen::Vector3d& b,
                           const std::string& texture_name,
                           Eigen::Vector2i texture_size) {
  auto min_pos = a.cwiseMin(b);
  auto max_pos = a.cwiseMax(b);

  Brush brush;

  Eigen::Vector3d box_size = (max_pos - min_pos);
  Eigen::Vector2d scale(map_builder::kWorldToGameUnits / texture_size.x(),
                        map_builder::kWorldToGameUnits / texture_size.y());

  // Helper function for calculating texture offset from the origin.
  auto offset_func = [](double v) { return v - std::floor(v); };

  Eigen::Vector2i offset_xy =
      Eigen::Vector2d(
          offset_func(min_pos.x() / box_size.x()) * texture_size.x(),
          offset_func(min_pos.y() / box_size.y()) * texture_size.y())
          .cast<int>();

  Eigen::Vector2i offset_xz =
      Eigen::Vector2d(
          offset_func(min_pos.x() / box_size.x()) * texture_size.x(),
          offset_func(min_pos.z() / box_size.z()) * texture_size.y())
          .cast<int>();

  Eigen::Vector2i offset_yz =
      Eigen::Vector2d(
          offset_func(min_pos.y() / box_size.y()) * texture_size.x(),
          offset_func(min_pos.z() / box_size.z()) * texture_size.y())
          .cast<int>();

  // X Plane
  Eigen::Vector2d scale_yz = {-scale.x() * box_size.y(),
                              scale.y() * box_size.z()};
  brush.planes.emplace_back(Plane{{min_pos.x(), 0, 0},
                                  {min_pos.x(), 1, 0},
                                  {min_pos.x(), 0, 1},
                                  {texture_name, offset_yz, 0, scale_yz}});
  brush.planes.emplace_back(Plane{{max_pos.x(), 0, 0},
                                  {max_pos.x(), 0, 1},
                                  {max_pos.x(), 1, 0},
                                  {texture_name, offset_yz, 0, scale_yz}});

  // Y Plane
  Eigen::Vector2d scale_xz = {-scale.x() * box_size.x(),
                              scale.y() * box_size.z()};
  brush.planes.emplace_back(Plane{{0, min_pos.y(), 0},
                                  {0, min_pos.y(), 1},
                                  {1, min_pos.y(), 0},
                                  {texture_name, offset_xz, 0, scale_xz}});
  brush.planes.emplace_back(Plane{{0, max_pos.y(), 0},
                                  {1, max_pos.y(), 0},
                                  {0, max_pos.y(), 1},
                                  {texture_name, offset_xz, 0, scale_xz}});

  // Z Plane
  Eigen::Vector2d scale_xy = {-scale.x() * box_size.x(),
                              scale.y() * box_size.y()};
  brush.planes.emplace_back(Plane{{0, 0, min_pos.z()},
                                  {1, 0, min_pos.z()},
                                  {0, 1, min_pos.z()},
                                  {texture_name, offset_xy, 0, scale_xy}});
  brush.planes.emplace_back(Plane{{0, 0, max_pos.z()},
                                  {0, 1, max_pos.z()},
                                  {1, 0, max_pos.z()},
                                  {texture_name, offset_xy, 0, scale_xy}});

  return brush;
}

std::vector<Brush> CreateHollowBox(const Eigen::Vector3d& a,
                                   const Eigen::Vector3d& b, double thickness,
                                   const Texture& texture) {
  std::vector<Brush> brushes;
  Eigen::Vector3d t(thickness, thickness, thickness);

  auto min_pos = a.cwiseMin(b);
  auto max_pos = a.cwiseMax(b);

  // Top.
  brushes.emplace_back(CreateBoxBrush(
      {min_pos.x() - thickness, min_pos.y() - thickness, max_pos.z()},
      max_pos + t, texture));

  // Bottom.
  brushes.emplace_back(CreateBoxBrush(
      min_pos - t,
      {max_pos.x() + thickness, max_pos.y() + thickness, min_pos.z()},
      texture));

  // Left.
  brushes.emplace_back(CreateBoxBrush(
      min_pos - t,
      {min_pos.x(), max_pos.y() + thickness, max_pos.z() + thickness},
      texture));

  // Right.
  brushes.emplace_back(CreateBoxBrush(
      {max_pos.x(), min_pos.y() - thickness, min_pos.z() - thickness},
      max_pos + t, texture));

  // Front.
  brushes.emplace_back(CreateBoxBrush(
      {min_pos.x() - thickness, max_pos.y(), min_pos.z() - thickness},
      max_pos + t, texture));

  // Back.
  brushes.emplace_back(CreateBoxBrush(
      min_pos - t,
      {max_pos.x() + thickness, min_pos.y(), max_pos.z() + thickness},
      texture));

  return brushes;
}

std::vector<Brush> CreateSkybox(const Eigen::Vector3d& position,
                                const Eigen::Vector3d& size, double thickness,
                                const std::string& texture_name,
                                const Eigen::Vector2i& texture_size) {
  const auto half_size = size * 0.5;
  auto min_pos = position - half_size;
  auto max_pos = position + half_size;

  std::vector<Brush> brushes;

  brushes.emplace_back(CreateFittedBoxBrush(
      max_pos, {min_pos.x(), min_pos.y(), max_pos.z() + thickness},
      absl::StrCat(texture_name, "_up"), texture_size));

  brushes.emplace_back(CreateFittedBoxBrush(
      min_pos, {max_pos.x(), max_pos.y(), min_pos.z() - thickness},
      absl::StrCat(texture_name, "_dn"), texture_size));

  // Left.
  brushes.emplace_back(CreateFittedBoxBrush(
      min_pos, {min_pos.x() - thickness, max_pos.y(), max_pos.z()},
      absl::StrCat(texture_name, "_lf"), texture_size));

  // Right.
  brushes.emplace_back(CreateFittedBoxBrush(
      max_pos, {max_pos.x() + thickness, min_pos.y(), min_pos.z()},
      absl::StrCat(texture_name, "_rt"), texture_size));

  // Front
  brushes.emplace_back(CreateFittedBoxBrush(
      min_pos, {max_pos.x(), min_pos.y() - thickness, max_pos.z()},
      absl::StrCat(texture_name, "_ft"), texture_size));

  // Back
  brushes.emplace_back(CreateFittedBoxBrush(
      max_pos, {min_pos.x(), max_pos.y() + thickness, min_pos.z()},
      absl::StrCat(texture_name, "_bk"), texture_size));

  return brushes;
}

Patch CreateGridPatch(Eigen::Vector3d center, Eigen::Vector3d normal,
                      Eigen::Vector3d up, Eigen::Vector2d size,
                      Eigen::Vector2i grid_size, const Texture& texture) {
  normal.normalize();
  up.normalize();

  Eigen::Vector3d right = normal.cross(up).normalized();

  Eigen::Vector3d start_pos =
      center - (right * size.x() * 0.5) - (up * size.y() * 0.5);
  Eigen::Vector2d step = {size.x() / (grid_size.x() - 1),
                          size.y() / (grid_size.y() - 1)};

  auto patch = Patch(grid_size, texture);
  for (int x = 0; x < grid_size.x(); ++x) {
    for (int y = 0; y < grid_size.y(); ++y) {
      auto pos = start_pos + (right * step.x() * x) + (up * step.y() * y);
      double u = static_cast<double>(x) / (grid_size.x() - 1);
      double v = static_cast<double>(y) / (grid_size.y() - 1);
      patch.set_point({x, y}, {pos, {u, 1 - v}});
    }
  }
  return patch;
}

}  // namespace brush_util

}  // namespace map_builder
}  // namespace lab
}  // namespace deepmind
