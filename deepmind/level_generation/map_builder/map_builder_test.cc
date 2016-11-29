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

#include "deepmind/support/stringprintf.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "deepmind/level_generation/map_builder/builder.h"

namespace deepmind {
namespace lab {
namespace map_builder {

namespace {

constexpr char kDefaultWorldMap[] = R"({
  "classname" "worldspawn"
})";

TEST(MapBuilderTest, DefaultWorldEntity) {
  Builder builder;
  EXPECT_EQ(kDefaultWorldMap, builder.ToString());
}

constexpr char kCustomWorldEntity[] = R"({
  "classname" "worldspawn"
  "light" "100"
  "worldtype" "2"
})";

TEST(MapBuilderTest, CustomWorldEntity) {
  Builder builder;
  Entity* world_entity = builder.mutable_world_entity();

  world_entity->set_attribute("worldtype", "2");
  world_entity->set_attribute("light", "100");
  EXPECT_EQ(kCustomWorldEntity, builder.ToString());
}

TEST(MapBuilderTest, AddBasicEntity) {
  Entity test_entity("test_entity");

  Builder builder;
  builder.AddEntity(test_entity);

  EXPECT_THAT(builder.ToString(),
              testing::HasSubstr("\"classname\" \"test_entity\""));
}

TEST(MapBuilderTest, CreatePointLight) {
  Eigen::Vector3d pos = {1.0, 1.0, 5.0};
  Entity light_entity = Entity::CreatePointLight(pos, 200.0);

  std::string light_str = light_entity.ToString();
  EXPECT_THAT(light_str, testing::HasSubstr("\"classname\" \"light\""));
  EXPECT_THAT(light_str, testing::HasSubstr("\"light\" \"200\""));
  EXPECT_THAT(light_str, testing::HasSubstr("\"style\" \"0\""));
  EXPECT_THAT(light_str, testing::HasSubstr("\"origin\" \"32 32 160\""));
}

TEST(MapBuilderTest, CreateSpawn) {
  Eigen::Vector3d pos = {2.0, 3.0, 4.0};
  double angle = 45.0;
  Entity spawn_entity = Entity::CreateSpawn(pos, Angle::Degrees(angle));

  std::string spawn_str = spawn_entity.ToString();

  EXPECT_THAT(spawn_str,
              testing::HasSubstr(StringPrintf("\"angle\" \"%g\"", angle)));
  EXPECT_THAT(spawn_str,
              testing::HasSubstr(StringPrintf(
                  "\"origin\" \"%g %g %g\"", pos.x() * kWorldToGameUnits,
                  pos.y() * kWorldToGameUnits, pos.z() * kWorldToGameUnits)));
}

TEST(MapBuilderTest, CreateTeamSpawn) {
  Eigen::Vector3d pos = {2.0, 3.0, 4.0};
  double angle = 45.0;
  auto red_spawn_entities =
      Entity::CreateTeamSpawn(pos, Angle::Degrees(angle), Team::kRed);

  EXPECT_THAT(red_spawn_entities.first.ToString(),
              testing::HasSubstr("\"classname\" \"team_CTF_redplayer\""));
  EXPECT_THAT(red_spawn_entities.second.ToString(),
              testing::HasSubstr("\"classname\" \"team_CTF_redspawn\""));

  auto blue_spawn_entities =
      Entity::CreateTeamSpawn(pos, Angle::Degrees(angle), Team::kBlue);

  EXPECT_THAT(blue_spawn_entities.first.ToString(),
              testing::HasSubstr("\"classname\" \"team_CTF_blueplayer\""));
  EXPECT_THAT(blue_spawn_entities.second.ToString(),
              testing::HasSubstr("\"classname\" \"team_CTF_bluespawn\""));
}

TEST(MapBuilderTest, CreateFlag) {
  Eigen::Vector3d pos = {2.0, 3.0, 4.0};
  auto red_flag_entity = Entity::CreateFlag(pos, Team::kRed);

  EXPECT_THAT(red_flag_entity.ToString(),
              testing::HasSubstr("\"classname\" \"team_CTF_redflag\""));

  auto blue_flag_entity = Entity::CreateFlag(pos, Team::kBlue);

  EXPECT_THAT(blue_flag_entity.ToString(),
              testing::HasSubstr("\"classname\" \"team_CTF_blueflag\""));
}

constexpr char kDefaultModelEntity[] = R"({
  "classname" "misc_model"
  "model" "model/test.md3"
  "origin" "96 128 176"
})";

TEST(MapBuilderTest, CreateModel) {
  Eigen::Vector3d pos = {3.0, 4.0, 5.5};
  PitchYawRoll rotation = {Angle::Degrees(0), Angle::Degrees(90),
                           Angle::Degrees(45)};
  Entity model_entity =
      Entity::CreateModel("model/test.md3", pos, rotation, {1, 1, 2});

  std::string model_str = model_entity.ToString();

  EXPECT_THAT(model_str,
              testing::HasSubstr(StringPrintf(
                  "\"origin\" \"%g %g %g\"", pos.x() * kWorldToGameUnits,
                  pos.y() * kWorldToGameUnits, pos.z() * kWorldToGameUnits)));
  EXPECT_THAT(model_str,
              testing::HasSubstr(StringPrintf(
                  "\"angles\" \"%g %g %g\"", rotation.pitch.degrees(),
                  rotation.yaw.degrees(), rotation.roll.degrees())));
  EXPECT_THAT(model_str,
              testing::HasSubstr("\"modelscale_vec\" \"1 1 2\""));

  // Create model with default scale and rotation.
  Entity default_model =
      Entity::CreateModel("model/test.md3", pos, {}, {1, 1, 1});

  EXPECT_THAT(default_model.ToString(),
              testing::HasSubstr(kDefaultModelEntity));
}

constexpr char kWorldBoxBrush[] = R"(
  {
    ( 32 0 0 ) ( 32 32 0 ) ( 32 0 32 ) test_texture 0 0 0 0 0 0 0 0
    ( 160 0 0 ) ( 160 0 32 ) ( 160 32 0 ) test_texture 0 0 0 0 0 0 0 0
    ( 0 64 0 ) ( 0 64 32 ) ( 32 64 0 ) test_texture 0 0 0 0 0 0 0 0
    ( 0 160 0 ) ( 32 160 0 ) ( 0 160 32 ) test_texture 0 0 0 0 0 0 0 0
    ( 0 0 96 ) ( 32 0 96 ) ( 0 32 96 ) test_texture 0 0 0 0 0 0 0 0
    ( 0 0 160 ) ( 0 32 160 ) ( 32 0 160 ) test_texture 0 0 0 0 0 0 0 0
  })";

TEST(MapBuilderTest, CreateBoxBrush) {
  Eigen::Vector3d a = {1.0, 2.0, 3.0};
  Eigen::Vector3d b = {5.0, 5.0, 5.0};

  Builder builder;
  auto brush = brush_util::CreateBoxBrush(a, b, {"test_texture"});
  builder.mutable_world_entity()->add_brush(brush);

  EXPECT_THAT(builder.ToString(), testing::HasSubstr(kWorldBoxBrush));
}

// Only check one brush for each side.
constexpr char kHollowBoxTop[] = R"(
  {
    ( 0 0 0 ) ( 0 32 0 ) ( 0 0 32 ) test_texture 0 0 0 0 0 0 0 0
    ( 192 0 0 ) ( 192 0 32 ) ( 192 32 0 ) test_texture 0 0 0 0 0 0 0 0
    ( 0 32 0 ) ( 0 32 32 ) ( 32 32 0 ) test_texture 0 0 0 0 0 0 0 0
    ( 0 192 0 ) ( 32 192 0 ) ( 0 192 32 ) test_texture 0 0 0 0 0 0 0 0
    ( 0 0 160 ) ( 32 0 160 ) ( 0 32 160 ) test_texture 0 0 0 0 0 0 0 0
    ( 0 0 192 ) ( 0 32 192 ) ( 32 0 192 ) test_texture 0 0 0 0 0 0 0 0
  })";

constexpr char kHollowBoxLeft[] = R"(
  {
    ( 0 0 0 ) ( 0 32 0 ) ( 0 0 32 ) test_texture 0 0 0 0 0 0 0 0
    ( 32 0 0 ) ( 32 0 32 ) ( 32 32 0 ) test_texture 0 0 0 0 0 0 0 0
    ( 0 32 0 ) ( 0 32 32 ) ( 32 32 0 ) test_texture 0 0 0 0 0 0 0 0
    ( 0 192 0 ) ( 32 192 0 ) ( 0 192 32 ) test_texture 0 0 0 0 0 0 0 0
    ( 0 0 64 ) ( 32 0 64 ) ( 0 32 64 ) test_texture 0 0 0 0 0 0 0 0
    ( 0 0 192 ) ( 0 32 192 ) ( 32 0 192 ) test_texture 0 0 0 0 0 0 0 0
  })";

constexpr char kHollowBoxFront[] = R"(
  {
    ( 0 0 0 ) ( 0 32 0 ) ( 0 0 32 ) test_texture 0 0 0 0 0 0 0 0
    ( 192 0 0 ) ( 192 0 32 ) ( 192 32 0 ) test_texture 0 0 0 0 0 0 0 0
    ( 0 160 0 ) ( 0 160 32 ) ( 32 160 0 ) test_texture 0 0 0 0 0 0 0 0
    ( 0 192 0 ) ( 32 192 0 ) ( 0 192 32 ) test_texture 0 0 0 0 0 0 0 0
    ( 0 0 64 ) ( 32 0 64 ) ( 0 32 64 ) test_texture 0 0 0 0 0 0 0 0
    ( 0 0 192 ) ( 0 32 192 ) ( 32 0 192 ) test_texture 0 0 0 0 0 0 0 0
  })";

TEST(MapBuilderTest, CreateHollowBox) {
  Eigen::Vector3d a = {1.0, 2.0, 3.0};
  Eigen::Vector3d b = {5.0, 5.0, 5.0};

  Builder builder;

  auto brushes = brush_util::CreateHollowBox(a, b, 1.0, {"test_texture"});
  builder.mutable_world_entity()->add_brushes(brushes);

  EXPECT_THAT(builder.ToString(), testing::HasSubstr(kHollowBoxTop));
  EXPECT_THAT(builder.ToString(), testing::HasSubstr(kHollowBoxLeft));
  EXPECT_THAT(builder.ToString(), testing::HasSubstr(kHollowBoxFront));
}

constexpr char kPatchString[] = R"({
    patchDef2
    {
      test
      ( 2 2 0 0 0 )
      (
        ( ( 0 32 32 0 0 ) ( 0 0 0 0 0 ) )
        ( ( 0 0 0 0 0 ) ( 32 64 96 1 1 ) )
      )
    }
  })";

TEST(MapBuilderTest, CreatePatch) {
  Eigen::Vector2i grid = {2, 2};
  PatchPoint point1 = {{0.0, 1.0, 1.0}, {0.0, 0.0}};
  PatchPoint point2 = {{1.0, 2.0, 3.0}, {1.0, 1.0}};

  Patch p(grid, {"test"});

  EXPECT_EQ(grid.x() * grid.y(), p.num_points());

  p.set_point({0, 0}, point1);
  p.set_point({1, 1}, point2);

  EXPECT_EQ(point1, p.point({0, 0}));
  EXPECT_EQ(point2, p.point({1, 1}));
  EXPECT_EQ(kPatchString, p.ToString());

  // Test adding patch to map_builder world entity.
  Builder builder;

  builder.mutable_world_entity()->add_patch(p);
  EXPECT_THAT(builder.ToString(), testing::HasSubstr(kPatchString));
}

TEST(MapBuilderTest, CreateGridPatch) {
  const Eigen::Vector3d center = {2.0, 0.0, 1.5};
  const Eigen::Vector3d normal = {0.0, 1.0, 0.0};
  const Eigen::Vector3d up = {0.0, 0.0, 1.0};
  const Eigen::Vector2d size = {4.0, 3.0};
  const Eigen::Vector2i grid_size = {3, 3};

  auto patch = brush_util::CreateGridPatch(center, normal, up, size, grid_size,
                                           {"test/texture"});
  EXPECT_EQ(grid_size.x() * grid_size.y(), patch.num_points());

  EXPECT_EQ(Eigen::Vector3d(0, 0, 0), patch.point({0, 0}).pos);
  EXPECT_EQ(Eigen::Vector2d(0, 1), patch.point({0, 0}).uv);

  EXPECT_EQ(Eigen::Vector3d(size.x(), 0.0, size.y()), patch.point({2, 2}).pos);
  EXPECT_EQ(Eigen::Vector2d(1, 0), patch.point({2, 2}).uv);
}

constexpr char kBrushString[] = R"({
    ( 0 0 0 ) ( 0 32 0 ) ( 0 0 32 ) test_texture 0 0 0 0 0 0 0 0
    ( 64 0 0 ) ( 64 0 32 ) ( 64 32 0 ) test_texture 0 0 0 0 0 0 0 0
    ( 0 32 0 ) ( 0 32 32 ) ( 32 32 0 ) test_texture 0 0 0 0 0 0 0 0
    ( 0 64 0 ) ( 32 64 0 ) ( 0 64 32 ) test_texture 0 0 0 0 0 0 0 0
    ( 0 0 160 ) ( 32 0 160 ) ( 0 32 160 ) test_texture 0 0 0 0 0 0 0 0
    ( 0 0 64 ) ( 0 32 64 ) ( 32 0 64 ) test_texture 0 0 0 0 0 0 0 0
  })";

TEST(MapBuilderTest, ParseBrush) {
  auto brushes = brush_util::ParseBrushes(kBrushString);
  ASSERT_EQ(1, brushes.size());

  const auto& brush = brushes[0];

  ASSERT_EQ(6, brush.planes.size());
  EXPECT_EQ(Eigen::Vector3d(0, 0, 0), brush.planes[0].a);  // ( 0 0 0 )
  EXPECT_EQ(Eigen::Vector3d(2, 0, 1), brush.planes[1].b);  // ( 64 0 32 )
  EXPECT_EQ(Eigen::Vector3d(1, 1, 0), brush.planes[2].c);  // ( 32 32 0 )

  EXPECT_EQ(kBrushString, brush.ToString());

  brushes =
      brush_util::ParseBrushes(std::string(kBrushString) + "\n" + kBrushString);

  ASSERT_EQ(2, brushes.size());
  EXPECT_EQ(brushes[0].ToString(), brushes[1].ToString());

  // Check mal-formed brush (ie. brush with no planes).
  brushes = brush_util::ParseBrushes("{}");
  EXPECT_EQ(0, brushes.size());
}

constexpr char kNestedBrushes[] = R"(
{
  {
    ( 0 0 0 ) ( 0 32 0 ) ( 0 0 32 ) test_texture 0 0 0 0 0 0 0 0
    ( 64 0 0 ) ( 64 0 32 ) ( 64 32 0 ) test_texture 0 0 0 0 0 0 0 0
  }
})";

TEST(MapBuilderTest, NestedBrushes) {
  auto brushes = brush_util::ParseBrushes(kNestedBrushes);
  ASSERT_EQ(1, brushes.size());

  const auto& brush = brushes[0];

  ASSERT_EQ(2, brush.planes.size());
  EXPECT_EQ(Eigen::Vector3d(2, 0, 0), brush.planes[1].a);  // ( 64 0 0 )

  brushes = brush_util::ParseBrushes(
      std::string(kNestedBrushes) + kNestedBrushes + "\n");

  EXPECT_EQ(2, brushes.size());
}

}  // namespace

}  // namespace map_builder
}  // namespace lab
}  // namespace deepmind
