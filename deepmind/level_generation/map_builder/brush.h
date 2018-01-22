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
// Low-level data structures that make up .map data.

#ifndef DML_DEEPMIND_LEVEL_GENERATION_MAP_BUILDER_BRUSH_H_
#define DML_DEEPMIND_LEVEL_GENERATION_MAP_BUILDER_BRUSH_H_

#include <string>
#include <vector>

#include "absl/strings/string_view.h"
#include "Eigen/Core"

namespace deepmind {
namespace lab {
namespace map_builder {

// Constant used to convert world units to game units. In DeepMind Lab there are
// 32 units to the meter, so we multiply any plane or position by this number.
constexpr double kWorldToGameUnits = 32.0;

// Textures are used in brushes and patches to define surface settings.
struct Texture {
  enum class Flags : int {
    kNone = 0x0,

    // Detail flag used to help improve BSP generation by disregarding this
    // brush for determining the PVS tree, instead adding them to the leaf of
    // any other non-detail (structural) brushes. This is mostly used in heavy
    // or large brush geometry. See q3map2/q3map2/brush.c for details.
    kDetail = 0x8000000,
  };

  Texture(
      std::string p = "",
      Eigen::Vector2i o = Eigen::Vector2i::Zero(),
      double r = 0.0,
      Eigen::Vector2d s = Eigen::Vector2d::Zero(),
      Flags f = Flags::kNone)
      : path(std::move(p)), offset(o), rot_angle(r), scale(s), flags(f) {}

  // Converts Texture to map file formatted string.
  std::string ToString() const;

  // Path to the texture relative from path.
  std::string path;

  // Texture offset (must be multiple of 16).
  Eigen::Vector2i offset;

  // Texture rotation in degrees.
  double rot_angle;

  // Scales texture (negative value to flip).
  Eigen::Vector2d scale;

  // Bitmask of flags used in the BSP generation.
  Flags flags;
};

// Plane, used in brushes to define a given region. Defined by three
// non-collinear points in a clockwise orientation. Positions should be in world
// units as they are converted during serialization to game units using the
// kWorldToGameUnits ratio.
struct Plane {
  // Converts Plane to map file formatted string.
  std::string ToString() const;

  // 3 points that defines the plane in world space.
  Eigen::Vector3d a;
  Eigen::Vector3d b;
  Eigen::Vector3d c;

  // Texture to use for this particular surface.
  Texture texture;
};

// Brush, used to define a solid region made up of 5 or more planes.
struct Brush {
  std::string ToString() const;

  // Array of planes that make up the Brush.
  std::vector<Plane> planes;
};

// PatchPoint, stores the world position and uv texture co-ordinates.
struct PatchPoint {
  // Converts PatchPoint to map file formatted string.
  std::string ToString() const;

  bool operator==(const PatchPoint& b) const {
    return pos == b.pos && uv == b.uv;
  }

  Eigen::Vector3d pos;
  Eigen::Vector2d uv;
};

// Patches are another type of brush that can be used for a given entity,
// where an array of n x m points are used to create a "patch" that can be used
// for things like placing textures upon other pieces of geometry. For each
// point, you can set the world position and the uv value for the associated
// shader.
class Patch {
 public:
  explicit Patch(Eigen::Vector2i grid_size, Texture texture)
      : grid_size_(grid_size),
        points_(grid_size_.x() * grid_size_.y(),
                {Eigen::Vector3d::Zero(), Eigen::Vector2d::Zero()}),
        texture_(std::move(texture)) {}

  void set_point(const Eigen::Vector2i& grid_pos, PatchPoint point) {
    points_[grid_pos.y() * grid_size_.x() + grid_pos.x()] = point;
  }

  PatchPoint point(const Eigen::Vector2i& grid_pos) const {
    return points_[grid_pos.y() * grid_size_.x() + grid_pos.x()];
  }

  int num_points() const { return points_.size(); }

  std::string ToString() const;

 private:
  Eigen::Vector2i grid_size_;
  std::vector<PatchPoint> points_;

  // Texture to use for this particular patch.
  Texture texture_;
};

namespace brush_util {

// Parses a string of brushes into a vector of brushes. Groups all planes
// contained within parentheses, including child brushes. For example:
// "{
//   ( 0 0 0 ) ( 0 32 0 ) ( 0 0 32 ) test_texture 0 0 0 0 0 0 0 0
//   ( 64 0 0 ) ( 64 0 32 ) ( 64 32 0 ) test_texture 0 0 0 0 0 0 0 0
//   ...
// }
// {
//   ( 0 0 0 ) ( 0 32 0 ) ( 0 0 32 ) test_texture 0 0 0 0 0 0 0 0
//   ( 64 0 0 ) ( 64 0 32 ) ( 64 32 0 ) test_texture 0 0 0 0 0 0 0 0
//   ...
// }"
//
// would create a vector of two brushes.
std::vector<Brush> ParseBrushes(absl::string_view str);

// Helper function for creating a box brush from two extents.
// All planes use the provided texture.
Brush CreateBoxBrush(
    const Eigen::Vector3d& a,
    const Eigen::Vector3d& b,
    const Texture& texture);

// Creates a box brush with texture that fits the brush size exactly once. To do
// this, each plane needs to have the scale and offset calculated based on the
// plane's size and distance from the origin. The texture_size is the size in
// pixels for the provided texture name.
//   *---*
//  /   /|
// *---* |
// |   | *
// |   |/
// *---*
Brush CreateFittedBoxBrush(
    const Eigen::Vector3d& a,
    const Eigen::Vector3d& b,
    const std::string& texture_name,
    Eigen::Vector2i texture_size);

// Helper function for creating a hollowed out box with a specified wall
// thickness. This is used for creating a bounding box for levels with open
// world sections, without which the map will fail to compile. Walls are
// overlapped in the same fashion as Radiant, to guarantee they're airtight.
// Below is a top-down ASCII drawing of the shape that is created:
// +--+------+--+
// |  |      |  |
// +--+------+--+
// |  |      |  |
// |  |      |  |
// +--+------+--+
// |  |      |  |
// +--+------+--+
std::vector<Brush> CreateHollowBox(
    const Eigen::Vector3d& a,
    const Eigen::Vector3d& b,
    double thickness,
    const Texture& texture);

// Helper function to create the level's skybox brushes. To ensure the textures
// align correctly, we create the brushes so that they don't intersect with each
// other, whilst still guaranteeing the box is airtight. The texture name is
// appended with a suffix for each side (so _ft for front, etc).
// Here is a top-down representation:
//    +------+
//    |      |
// +--+------+--+
// |  |      |  |
// |  |      |  |
// +--+------+--+
//    |      |
//    +------+
std::vector<Brush> CreateSkybox(
    const Eigen::Vector3d& position,
    const Eigen::Vector3d& size,
    double thickness,
    const std::string& texture_name,
    const Eigen::Vector2i& texture_size);

// Helper function for creating a grid patch from a world position, normal, up
// and world size. The provided grid_size is the number of vertices to create in
// each direction. The UV's for each point will be interpolated between 0-1.
// Below is an ASCII representation of a 3x3 grid patch:
// +----+----+
// |    |    |
// |    |    |
// +----+----+
// |    |    |
// |    |    |
// +----+----+
Patch CreateGridPatch(
    Eigen::Vector3d center,
    Eigen::Vector3d normal,
    Eigen::Vector3d up,
    Eigen::Vector2d size,
    Eigen::Vector2i grid_size,
    const Texture& texture);

}  // namespace brush_util

}  // namespace map_builder
}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_LEVEL_GENERATION_MAP_BUILDER_BRUSH_H_
