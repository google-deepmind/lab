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

#include "deepmind/level_generation/map_builder/entity.h"

#include <cmath>
#include <string>

#include "deepmind/support/str_cat.h"
#include "deepmind/support/str_join.h"

namespace deepmind {
namespace lab {
namespace map_builder {

Entity::Entity(std::string class_name, Eigen::Vector3d origin)
    : Entity(std::move(class_name)) {
  set_attribute("origin", origin * kWorldToGameUnits);
}

Entity Entity::CreatePointLight(Eigen::Vector3d position, double intensity) {
  Entity point_light("light", position);
  point_light.set_attribute("light", StringPrintf("%g", intensity));
  point_light.set_attribute("style", "0");
  point_light.set_attribute("spawnflags", "0");
  return point_light;
}

Entity Entity::CreateSpawn(Eigen::Vector3d position, Angle angle) {
  Entity spawn_point("info_player_start", position);
  if (angle.radians()) {
    spawn_point.set_attribute("angle", StringPrintf("%g", angle.degrees()));
  }
  return spawn_point;
}

std::pair<Entity, Entity> Entity::CreateTeamSpawn(Eigen::Vector3d position,
                                                  Angle angle, Team team) {
  const auto& team_string = team == Team::kRed ? "red" : "blue";
  Entity team_player(StrCat("team_CTF_", team_string, "player"), position);
  Entity team_spawn(StrCat("team_CTF_", team_string, "spawn"), position);
  if (angle.radians()) {
    auto angle_string = StringPrintf("%g", angle.degrees());
    team_spawn.set_attribute("angle", angle_string);
    team_player.set_attribute("angle", angle_string);
  }
  return {team_player, team_spawn};
}

Entity Entity::CreateFlag(Eigen::Vector3d position, Team team) {
  auto entity_name =
      team == Team::kRed ? "team_CTF_redflag" : "team_CTF_blueflag";
  return Entity(entity_name, position);
}

Entity Entity::CreateWorld() { return Entity("worldspawn"); }

Entity Entity::CreateModel(const std::string& model_filename,
                           Eigen::Vector3d position, PitchYawRoll rotation,
                           Eigen::Vector3d scale) {
  Entity model_entity("misc_model", position);
  model_entity.set_attribute("model", model_filename);

  // Only add rotation if we have any set.
  if (rotation.pitch.radians() || rotation.yaw.radians() ||
      rotation.roll.radians()) {
    model_entity.set_attribute(
        "angles",
        StringPrintf("%g %g %g", rotation.pitch.degrees(),
                     rotation.yaw.degrees(), rotation.roll.degrees()));
  }

  // Same for scale, only add if it's not 1.
  if (scale != Eigen::Vector3d(1, 1, 1)) {
    model_entity.set_attribute("modelscale_vec", scale);
  }
  return model_entity;
}

std::string Entity::ToString() const {
  string result = StrCat("{\n  \"classname\" \"", class_name(), "\"");
  // Add attributes (if we have any).
  if (!attributes_.empty()) {
    const auto quote_formatter = [](string* out, const string& in) {
      StrAppend(out, "\"", in, "\"");
    };

    StrAppend(&result, "\n  ",
              strings::Join(attributes_, "\n  ",
                            strings::PairFormatter(quote_formatter, " ",
                                                   quote_formatter)));
  }
  // Add brushes (if we have any).
  if (!brushes_.empty()) {
    StrAppend(
        &result, "\n  ",
        strings::Join(brushes_, "\n  ", [](string* out, const Brush& brush) {
          StrAppend(out, brush.ToString());
        }));
  }
  // Add patches (if we have any).
  if (!patches_.empty()) {
    StrAppend(
        &result, "\n  ",
        strings::Join(patches_, "\n  ", [](string* out, const Patch& patch) {
          StrAppend(out, patch.ToString());
        }));
  }
  StrAppend(&result, "\n}");
  return result;
}

}  // namespace map_builder
}  // namespace lab
}  // namespace deepmind
