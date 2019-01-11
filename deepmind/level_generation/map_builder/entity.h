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
// A set of classes to represent the data described by the .map format.

#ifndef DML_DEEPMIND_LEVEL_GENERATION_MAP_BUILDER_ENTITY_H_
#define DML_DEEPMIND_LEVEL_GENERATION_MAP_BUILDER_ENTITY_H_

#include <iterator>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "absl/strings/str_cat.h"
#include "Eigen/Core"
#include "deepmind/level_generation/map_builder/brush.h"

namespace deepmind {
namespace lab {
namespace map_builder {

constexpr double kPi = 3.14159265358979323846;

// Enum to denote teams used in certain entity types.
enum class Team { kBlue, kRed };

// Concrete class used to represent an angle.
class Angle {
 public:
  Angle() : radians_(0) {}
  static Angle Degrees(double degrees) { return Angle((kPi / 180) * degrees); }
  static Angle Radians(double radians) { return Angle(radians); }

  double degrees() const { return (180 / kPi) * radians(); }
  double radians() const { return radians_; }

 private:
  explicit Angle(double radians) : radians_(radians) {}

  double radians_;
};

// Struct used to store the pitch, yaw & roll used in specifying the rotation of
// an entity.
struct PitchYawRoll {
  Angle pitch;
  Angle yaw;
  Angle roll;
};

// Entity, building block for everything inside a map file. Every entity
// contains a classname (which depicts its type), in addition to zero or more
// attributes. It can also optionally contain zero or more brushes. See
// http://www.gamers.org/dEngine/quake/QDP/qmapspec.html#2.2.1 for details.
class Entity {
 public:
  explicit Entity(std::string class_name)
      : class_name_(std::move(class_name)) {}

  // Lots of entity types have a position attribute, so we provide this
  // overloaded constructor as a helper.
  explicit Entity(std::string class_name, Eigen::Vector3d position);

  // Factory functions for creating various types of entities. Only generic
  // entity types should be added here, anything specific to a level should be
  // added to the particular level generator.

  // Creates a point light at a particular world position with a given
  // intensity. The default intensity in DeepMind Lab is 200.
  static Entity CreatePointLight(Eigen::Vector3d position, double intensity);

  // Creates a spawn point at the provided position. Angle provides the
  // horizontal direction in degrees.
  static Entity CreateSpawn(Eigen::Vector3d position, Angle angle);

  // Creates a team spawn point at the provided position. Angle provides the
  // horizontal direction in degrees. Returns two entities, one for the initial
  // spawn and another for subsequent spawns, both at the same location.
  static std::pair<Entity, Entity> CreateTeamSpawn(Eigen::Vector3d position,
                                                   Angle angle, Team team);

  // Creates a team flag entity at the provided position. Used in CTF game
  // modes.
  static Entity CreateFlag(Eigen::Vector3d position, Team team);

  // Creates a default world entity. This is the entity world geometry should be
  // added to.
  static Entity CreateWorld();

  // Creates a model entity with position, rotation and scale attributes. Scale
  // is a multiple of the model's size.
  static Entity CreateModel(
      const std::string& model_filename,
      Eigen::Vector3d position,
      PitchYawRoll rotation,
      Eigen::Vector3d scale);

  const std::string& class_name() const { return class_name_; }

  void set_attribute(const std::string& key, std::string value) {
    attributes_[key] = std::move(value);
  }

  void set_attribute(const std::string& key, Eigen::Vector3d value) {
    attributes_[key] = absl::StrCat(value.x(), " ", value.y(), " ", value.z());
  }

  template <typename T>
  void set_attributes(const T& attributes) {
    for (const auto& attribute : attributes) {
      attributes_[attribute.first] = attribute.second;
    }
  }

  template <typename T>
  void add_brushes(const T& brushes) {
    brushes_.insert(brushes_.end(), std::begin(brushes), std::end(brushes));
  }

  void add_brush(Brush b) { brushes_.push_back(std::move(b)); }
  void add_patch(Patch p) { patches_.push_back(std::move(p)); }

  std::string ToString() const;

 private:
  // The entity's "class name" and only required attribute. This specifies what
  // the entity should be parsed as when loading into game, and what attributes
  // should be read (if any).
  const std::string class_name_;

  // Map of attributes specific to the particular entity. This can be anything
  // from positions to angles, names to links to other entities. For an
  // extensive (but not exhaustive) list of available attributes, see
  // http://www.gamers.org/dEngine/quake/QDP/qmapspec.html
  std::map<std::string, std::string> attributes_;

  // Geometry for the given entity.
  std::vector<Brush> brushes_;
  std::vector<Patch> patches_;
};

}  // namespace map_builder
}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_LEVEL_GENERATION_MAP_BUILDER_ENTITY_H_
