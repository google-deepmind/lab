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

#include "deepmind/level_generation/text_level/text_level_exporter.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace deepmind {
namespace lab {
namespace {

using ::testing::Eq;

class TextLevelExporterTest : public ::testing::Test {
 protected:
  TextLevelExporterTest() {}
  TextLevelSettings settings_;
};

TEST_F(TextLevelExporterTest, AddWall) {
  TextLevelExporter exporter(&settings_);
  exporter.AddWall({0.105, 0.2, 0.3}, {0.4, 0.5, 60000}, {0, 0, 0}, {1, 1}, 0);

  const char kExpectedSubstr[] = R"({
  "classname" "worldspawn"
  "light" "100"
  "worldtype" "2"
  {
    ( 10.5 0 0 ) ( 10.5 32 0 ) ( 10.5 0 32 ) map/lab_games/lg_style_01_wall_green 0 0 0 0.0976562 0.0976562 0 0 0
    ( 40 0 0 ) ( 40 0 32 ) ( 40 32 0 ) map/lab_games/lg_style_01_wall_green 0 0 0 0.0976562 0.0976562 0 0 0
    ( 0 20 0 ) ( 0 20 32 ) ( 32 20 0 ) map/lab_games/lg_style_01_wall_green 0 0 0 0.0976562 0.0976562 0 0 0
    ( 0 50 0 ) ( 32 50 0 ) ( 0 50 32 ) map/lab_games/lg_style_01_wall_green 0 0 0 0.0976562 0.0976562 0 0 0
    ( 0 0 30 ) ( 32 0 30 ) ( 0 32 30 ) map/lab_games/lg_style_01_wall_green 0 0 0 0.0976562 0.0976562 0 0 0
    ( 0 0 6e+06 ) ( 0 32 6e+06 ) ( 32 0 6e+06 ) map/lab_games/lg_style_01_wall_green 0 0 0 0.0976562 0.0976562 0 0 0
  }
}
)";

  EXPECT_THAT(exporter.ToString(), Eq(kExpectedSubstr));
}

TEST_F(TextLevelExporterTest, AddLight) {
  TextLevelExporter exporter(&settings_);
  exporter.Add(exporter.MakeLight({1, 2, 3}, 50));

  const char kExpectedSubstr[] = R"(
{
  "classname" "light"
  "light" "250"
  "origin" "100 200 300"
  "spawnflags" "0"
  "style" "0"
})";

  ASSERT_THAT(exporter.ToString(), testing::HasSubstr(kExpectedSubstr));
}

TEST_F(TextLevelExporterTest, AddPlatform) {
  TextLevelExporter exporter(&settings_);
  exporter.AddPlatform(3, 4, 7);

  const char kExpectedSubstr[] = R"({
  "classname" "worldspawn"
  "light" "100"
  "worldtype" "2"
  {
    ( 300 0 0 ) ( 300 32 0 ) ( 300 0 32 ) map/lab_games/lg_style_02_wall_blue 0 0 0 0.0976562 0.0976562 0 0 0
    ( 400 0 0 ) ( 400 0 32 ) ( 400 32 0 ) map/lab_games/lg_style_02_wall_blue 0 0 0 0.0976562 0.0976562 0 0 0
    ( 0 400 0 ) ( 0 400 32 ) ( 32 400 0 ) map/lab_games/lg_style_02_wall_blue 0 0 0 0.0976562 0.0976562 0 0 0
    ( 0 500 0 ) ( 32 500 0 ) ( 0 500 32 ) map/lab_games/lg_style_02_wall_blue 0 0 0 0.0976562 0.0976562 0 0 0
    ( 0 0 96.875 ) ( 32 0 96.875 ) ( 0 32 96.875 ) map/lab_games/lg_style_02_wall_blue 0 0 0 0.0976562 0.0976562 0 0 0
    ( 0 0 129.375 ) ( 0 32 129.375 ) ( 32 0 129.375 ) map/lab_games/lg_style_02_wall_blue 0 0 0 0.0976562 0.0976562 0 0 0
  }
  {
    ( 300 0 0 ) ( 300 32 0 ) ( 300 0 32 ) map/black_d 0 0 0 1.5625 1.5625 0 0 0
    ( 400 0 0 ) ( 400 0 32 ) ( 400 32 0 ) map/black_d 0 0 0 1.5625 1.5625 0 0 0
    ( 0 400 0 ) ( 0 400 32 ) ( 32 400 0 ) map/black_d 0 0 0 1.5625 1.5625 0 0 0
    ( 0 500 0 ) ( 32 500 0 ) ( 0 500 32 ) map/black_d 0 0 0 1.5625 1.5625 0 0 0
    ( 0 0 129.375 ) ( 32 0 129.375 ) ( 0 32 129.375 ) map/black_d 0 0 0 1.5625 1.5625 0 0 0
    ( 0 0 136.875 ) ( 0 32 136.875 ) ( 32 0 136.875 ) map/black_d 0 0 0 1.5625 1.5625 0 0 0
  }
  {
    ( 300 0 0 ) ( 300 32 0 ) ( 300 0 32 ) map/lab_games/lg_style_01_floor_orange 0 0 0 0.0976562 0.0976562 0 0 0
    ( 400 0 0 ) ( 400 0 32 ) ( 400 32 0 ) map/lab_games/lg_style_01_floor_orange 0 0 0 0.0976562 0.0976562 0 0 0
    ( 0 400 0 ) ( 0 400 32 ) ( 32 400 0 ) map/lab_games/lg_style_01_floor_orange 0 0 0 0.0976562 0.0976562 0 0 0
    ( 0 500 0 ) ( 32 500 0 ) ( 0 500 32 ) map/lab_games/lg_style_01_floor_orange 0 0 0 0.0976562 0.0976562 0 0 0
    ( 0 0 136.875 ) ( 32 0 136.875 ) ( 0 32 136.875 ) map/lab_games/lg_style_01_floor_orange 0 0 0 0.0976562 0.0976562 0 0 0
    ( 0 0 140 ) ( 0 32 140 ) ( 32 0 140 ) map/lab_games/lg_style_01_floor_orange 0 0 0 0.0976562 0.0976562 0 0 0
  }
}
)";

  EXPECT_THAT(exporter.ToString(), Eq(kExpectedSubstr));
}

TEST_F(TextLevelExporterTest, AddGlassColumn) {
  TextLevelExporter exporter(&settings_);
  exporter.AddGlassColumn(2, 5, 8);

  const char kExpectedSubstr[] = R"({
  "classname" "worldspawn"
  "light" "100"
  "worldtype" "2"
  {
    ( 200 0 0 ) ( 200 32 0 ) ( 200 0 32 ) map/poltergeist 0 0 0 0 0 0 0 0
    ( 300 0 0 ) ( 300 0 32 ) ( 300 32 0 ) map/poltergeist 0 0 0 0 0 0 0 0
    ( 0 500 0 ) ( 0 500 32 ) ( 32 500 0 ) map/poltergeist 0 0 0 0 0 0 0 0
    ( 0 600 0 ) ( 32 600 0 ) ( 0 600 32 ) map/poltergeist 0 0 0 0 0 0 0 0
    ( 0 0 0 ) ( 32 0 0 ) ( 0 32 0 ) map/poltergeist 0 0 0 0 0 0 0 0
    ( 0 0 160 ) ( 0 32 160 ) ( 32 0 160 ) map/poltergeist 0 0 0 0 0 0 0 0
  }
}
)";

  EXPECT_THAT(exporter.ToString(), Eq(kExpectedSubstr));
}

TEST_F(TextLevelExporterTest, AddBrushEntity) {
  TextLevelExporter exporter(&settings_);
  exporter.Add(exporter.MakeBrushEntity(
      {150, 200, 10}, {170, 230, 50}, "func_door",
      "maps/brush_entity_placeholder", 0, 0,
      {{"speed", "5"}, {"wait", "10"}}));

  const char kExpectedSubstr[] = R"(
{
  "classname" "func_door"
  "speed" "5"
  "wait" "10"
  {
    ( 15000 0 0 ) ( 15000 32 0 ) ( 15000 0 32 ) maps/brush_entity_placeholder 0 0 0 0 0 0 0 0
    ( 17000 0 0 ) ( 17000 0 32 ) ( 17000 32 0 ) maps/brush_entity_placeholder 0 0 0 0 0 0 0 0
    ( 0 20000 0 ) ( 0 20000 32 ) ( 32 20000 0 ) maps/brush_entity_placeholder 0 0 0 0 0 0 0 0
    ( 0 23000 0 ) ( 32 23000 0 ) ( 0 23000 32 ) maps/brush_entity_placeholder 0 0 0 0 0 0 0 0
    ( 0 0 1000 ) ( 32 0 1000 ) ( 0 32 1000 ) maps/brush_entity_placeholder 0 0 0 0 0 0 0 0
    ( 0 0 5000 ) ( 0 32 5000 ) ( 32 0 5000 ) maps/brush_entity_placeholder 0 0 0 0 0 0 0 0
  }
})";

  ASSERT_THAT(exporter.ToString(), testing::HasSubstr(kExpectedSubstr));
}

TEST_F(TextLevelExporterTest, AddEntity) {
  TextLevelExporter exporter(&settings_);
  exporter.Add(exporter.MakeEntity({1, 2, 3}, "custom_class",
                                   {{"key_00", "value_00"},
                                    {"key_01", "value_01"},
                                    {"key_02", "value_02"}}));

  const char kExpectedSubstr[] = R"(
{
  "classname" "custom_class"
  "key_00" "value_00"
  "key_01" "value_01"
  "key_02" "value_02"
  "origin" "100 200 300"
})";

  ASSERT_THAT(exporter.ToString(), testing::HasSubstr(kExpectedSubstr));
}

}  // namespace
}  // namespace lab
}  // namespace deepmind
