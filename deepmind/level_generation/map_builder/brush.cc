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

#include "deepmind/support/stringprintf.h"
#include "deepmind/support/str_cat.h"
#include "deepmind/support/str_split.h"
#include "deepmind/support/string_view.h"
#include "deepmind/support/string_view_utils.h"
#include "Eigen/Geometry"

namespace deepmind {
namespace lab {
namespace map_builder {

namespace {

// Helper function to parse a single brush.
bool ParseSingleBrush(StringPiece brush_str, Brush* brush) {
  for (StringPiece line : strings::Split(brush_str, '\n')) {
    strings::RemoveLeadingWhitespace(&line);
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
  return StringPrintf("%.256s %d %d %g %g %g %d 0 0", path.c_str(), offset.x(),
                      offset.y(), rot_angle, scale.x(), scale.y(),
                      static_cast<int>(flags));
}

std::string Plane::ToString() const {
  // Convert world units to game units.
  return StringPrintf("( %g %g %g ) ( %g %g %g ) ( %g %g %g ) %s",
                      a.x() * kWorldToGameUnits, a.y() * kWorldToGameUnits,
                      a.z() * kWorldToGameUnits, b.x() * kWorldToGameUnits,
                      b.y() * kWorldToGameUnits, b.z() * kWorldToGameUnits,
                      c.x() * kWorldToGameUnits, c.y() * kWorldToGameUnits,
                      c.z() * kWorldToGameUnits, texture.ToString().c_str());
}

std::string Brush::ToString() const {
  string brush = "{\n";
  for (const auto& plane : planes) {
    StrAppend(&brush, "    ", plane.ToString(), "\n");
  }
  StrAppend(&brush, "  }");
  return brush;
}

std::string PatchPoint::ToString() const {
  return StringPrintf("( %g %g %g %g %g )", pos.x() * kWorldToGameUnits,
                      pos.y() * kWorldToGameUnits, pos.z() * kWorldToGameUnits,
                      uv.x(), uv.y());
}

std::string Patch::ToString() const {
  string patch =
      StrCat("{\n    patchDef2\n    {\n      ", texture_.path, "\n      ( ",
             grid_size_.x(), " ", grid_size_.y(), " 0 0 0 )\n");
  StrAppend(&patch, "      (\n");
  for (int x = 0; x < grid_size_.x(); x++) {
    StrAppend(&patch, "        (");
    for (int y = 0; y < grid_size_.y(); y++) {
      StrAppend(&patch, " ", point({x, y}).ToString());
    }
    StrAppend(&patch, " )\n");
  }
  StrAppend(&patch, "      )\n    }\n  }");
  return patch;
}

namespace brush_util {

std::vector<Brush> ParseBrushes(StringPiece brushes_str) {
  std::vector<Brush> brushes;

  int brush_start = -1;
  for (auto cur = brushes_str.find('{'); cur != brushes_str.npos;
       cur = brushes_str.find_first_of("{}", cur)) {
    if (brushes_str[cur] == '{') {
      brush_start = cur;
    } else if (brush_start != -1) {
      // End of current brush, parse its assets.
      Brush brush;
      if (ParseSingleBrush(
              brushes_str.substr(brush_start, cur - brush_start + 1), &brush)) {
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
  Eigen::Vector3d min_pos = a.cwiseMin(b);
  Eigen::Vector3d max_pos = a.cwiseMax(b);

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

  // Top.
  brushes.emplace_back(CreateBoxBrush(
      {a.x() - thickness, a.y() - thickness, b.z()}, b + t, texture));

  // Bottom.
  brushes.emplace_back(CreateBoxBrush(
      a - t, {b.x() + thickness, b.y() + thickness, a.z()}, texture));

  // Left.
  brushes.emplace_back(CreateBoxBrush(
      a - t, {a.x(), b.y() + thickness, b.z() + thickness}, texture));

  // Right.
  brushes.emplace_back(CreateBoxBrush(
      {b.x(), a.y() - thickness, a.z() - thickness}, b + t, texture));

  // Front.
  brushes.emplace_back(CreateBoxBrush(
      {a.x() - thickness, b.y(), a.z() - thickness}, b + t, texture));

  // Back.
  brushes.emplace_back(CreateBoxBrush(
      a - t, {b.x() + thickness, a.y(), b.z() + thickness}, texture));

  return brushes;
}

std::vector<Brush> CreateSkybox(const Eigen::Vector3d& position,
                                const Eigen::Vector3d& size, double thickness,
                                const std::string& texture_name,
                                const Eigen::Vector2i& texture_size) {
  const auto half_size = size * 0.5;
  auto a = position - half_size;
  auto b = position + half_size;
  const Eigen::Vector3d t = {thickness, thickness, thickness};

  std::vector<Brush> brushes;

  // Up.
  brushes.emplace_back(brush_util::CreateFittedBoxBrush(
      {a.x() - thickness, a.y() - thickness, b.z()}, b + t,
      StrCat(texture_name, "_up"), texture_size));

  // Down.
  brushes.emplace_back(brush_util::CreateFittedBoxBrush(
      a - t, {b.x() + thickness, b.y() + thickness, a.z()},
      StrCat(texture_name, "_dn"), texture_size));

  // Left.
  brushes.emplace_back(brush_util::CreateFittedBoxBrush(
      {a.x(), a.y() - thickness, a.z()},
      {a.x() - thickness, b.y() + thickness, b.z()},
      StrCat(texture_name, "_lf"), texture_size));

  // Right.
  brushes.emplace_back(brush_util::CreateFittedBoxBrush(
      {b.x(), a.y() - thickness, a.z()},
      {b.x() + thickness, b.y() + thickness, b.z()},
      StrCat(texture_name, "_rt"), texture_size));

  // Front.
  brushes.emplace_back(brush_util::CreateFittedBoxBrush(
      a, {b.x(), a.y() - thickness, b.z()}, StrCat(texture_name, "_ft"),
      texture_size));

  // Back.
  brushes.emplace_back(brush_util::CreateFittedBoxBrush(
      {a.x(), b.y() + thickness, a.z()}, b, StrCat(texture_name, "_bk"),
      texture_size));

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
