// Copyright (C) 2017 Google Inc.
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

#include "deepmind/model_generation/lua_model.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "absl/strings/str_cat.h"
#include "deepmind/include/deepmind_context.h"
#include "deepmind/lua/bind.h"
#include "deepmind/lua/call.h"
#include "deepmind/lua/n_results_or_test_util.h"
#include "deepmind/lua/push_script.h"
#include "deepmind/lua/vm.h"
#include "deepmind/model_generation/geometry_util.h"
#include "deepmind/model_generation/model_lua.h"
#include "deepmind/support/test_srcdir.h"
#include "deepmind/tensor/lua_tensor.h"

namespace deepmind {
namespace lab {
namespace {

using geometry::kEpsilon;
using ::testing::ElementsAre;

TEST(DeepmindModelLibTest, CreateCone) {
  DeepmindContext ctx{};
  ASSERT_EQ(0, dmlab_create_context(TestSrcDir().c_str(), &ctx, nullptr,
                                    nullptr, nullptr));
  ctx.hooks.set_level_name(ctx.userdata, "tests/model_test");
  ASSERT_EQ(0, ctx.hooks.init(ctx.userdata));

  ASSERT_TRUE(ctx.hooks.find_model(ctx.userdata, "cone"));
  DeepmindModelGetters model;
  void* model_data;
  ctx.hooks.model_getters(ctx.userdata, &model, &model_data);

  ASSERT_EQ(model.get_surface_count(model_data), 1);

  ASSERT_EQ(model.get_surface_vertex_count(model_data, 0), 168);
  ASSERT_EQ(model.get_surface_face_count(model_data, 0), 224);

  int face_indices[3];
  model.get_surface_face(model_data, 0, 0, face_indices);
  EXPECT_THAT(face_indices, ElementsAre(5, 4, 0));
  model.get_surface_face(model_data, 0, 112, face_indices);
  EXPECT_THAT(face_indices, ElementsAre(89, 88, 84));

  ctx.hooks.clear_model(ctx.userdata);
  dmlab_release_context(&ctx);
}

TEST(DeepmindModelLibTest, CreateCube) {
  DeepmindContext ctx{};
  ASSERT_EQ(0, dmlab_create_context(TestSrcDir().c_str(), &ctx, nullptr,
                                    nullptr, nullptr));
  ctx.hooks.set_level_name(ctx.userdata, "tests/model_test");
  ASSERT_EQ(0, ctx.hooks.init(ctx.userdata));

  ASSERT_TRUE(ctx.hooks.find_model(ctx.userdata, "cube"));
  DeepmindModelGetters model;
  void* model_data;
  ctx.hooks.model_getters(ctx.userdata, &model, &model_data);

  ASSERT_EQ(model.get_surface_count(model_data), 1);

  ASSERT_EQ(model.get_surface_vertex_count(model_data, 0), 150);
  ASSERT_EQ(model.get_surface_face_count(model_data, 0), 192);

  int face_indices[3];
  model.get_surface_face(model_data, 0, 0, face_indices);
  EXPECT_THAT(face_indices, ElementsAre(0, 5, 6));
  model.get_surface_face(model_data, 0, 1, face_indices);
  EXPECT_THAT(face_indices, ElementsAre(0, 6, 1));

  ctx.hooks.clear_model(ctx.userdata);
  dmlab_release_context(&ctx);
}

TEST(DeepmindModelLibTest, CreateCylinder) {
  DeepmindContext ctx{};
  ASSERT_EQ(0, dmlab_create_context(TestSrcDir().c_str(), &ctx, nullptr,
                                    nullptr, nullptr));
  ctx.hooks.set_level_name(ctx.userdata, "tests/model_test");
  ASSERT_EQ(0, ctx.hooks.init(ctx.userdata));

  ASSERT_TRUE(ctx.hooks.find_model(ctx.userdata, "cylinder"));
  DeepmindModelGetters model;
  void* model_data;
  ctx.hooks.model_getters(ctx.userdata, &model, &model_data);

  ASSERT_EQ(model.get_surface_count(model_data), 1);

  ASSERT_EQ(model.get_surface_vertex_count(model_data, 0), 253);
  ASSERT_EQ(model.get_surface_face_count(model_data, 0), 352);

  ctx.hooks.clear_model(ctx.userdata);
  dmlab_release_context(&ctx);
}

TEST(DeepmindModelLibTest, CreateSphere) {
  DeepmindContext ctx{};
  ASSERT_EQ(0, dmlab_create_context(TestSrcDir().c_str(), &ctx, nullptr,
                                    nullptr, nullptr));
  ctx.hooks.set_level_name(ctx.userdata, "tests/model_test");
  ASSERT_EQ(0, ctx.hooks.init(ctx.userdata));

  ASSERT_TRUE(ctx.hooks.find_model(ctx.userdata, "sphere"));
  DeepmindModelGetters model;
  void* model_data;
  ctx.hooks.model_getters(ctx.userdata, &model, &model_data);

  ASSERT_EQ(model.get_surface_count(model_data), 1);

  ASSERT_EQ(model.get_surface_vertex_count(model_data, 0), 168);
  ASSERT_EQ(model.get_surface_face_count(model_data, 0), 224);

  int face_indices[3];
  model.get_surface_face(model_data, 0, 0, face_indices);
  EXPECT_THAT(face_indices, ElementsAre(5, 4, 0));
  model.get_surface_face(model_data, 0, 112, face_indices);
  EXPECT_THAT(face_indices, ElementsAre(89, 88, 84));

  ctx.hooks.clear_model(ctx.userdata);
  dmlab_release_context(&ctx);
}

TEST(DeepmindModelLibTest, CreateHierarchy) {
  DeepmindContext ctx{};
  ASSERT_EQ(0, dmlab_create_context(TestSrcDir().c_str(), &ctx, nullptr,
                                    nullptr, nullptr));
  ctx.hooks.set_level_name(ctx.userdata, "tests/model_test");
  ASSERT_EQ(0, ctx.hooks.init(ctx.userdata));

  ASSERT_TRUE(ctx.hooks.find_model(ctx.userdata, "hierarchy"));
  DeepmindModelGetters model;
  void* model_data;
  ctx.hooks.model_getters(ctx.userdata, &model, &model_data);

  constexpr int kSurfaceCount = 4;
  ASSERT_EQ(model.get_surface_count(model_data), kSurfaceCount);

  struct SurfaceInfo {
    int vc;
    int fc;
  };

  std::map<std::string, SurfaceInfo> surface_infos = {
      {"root_cone_surface", SurfaceInfo{66, 32}},
      {"root_c0:sphere_surface", SurfaceInfo{168, 224}},
      {"root_c0:c0:cylinder_surface", SurfaceInfo{100, 64}},
      {"root_c0:c0:c0:cone_surface", SurfaceInfo{66, 32}},
  };

  for (size_t i = 0; i < kSurfaceCount; ++i) {
    char name[64];
    model.get_surface_name(model_data, i, sizeof(name), name);
    auto surface_info_it = surface_infos.find(name);
    ASSERT_TRUE(surface_info_it != surface_infos.end())
        << "Cannont find surface: " << name << " - "
        << model.get_surface_vertex_count(model_data, i) << ", "
        << model.get_surface_face_count(model_data, i);
    EXPECT_EQ(model.get_surface_vertex_count(model_data, i),
              surface_info_it->second.vc);
    EXPECT_EQ(model.get_surface_face_count(model_data, i),
              surface_info_it->second.fc);
  }

  ctx.hooks.clear_model(ctx.userdata);
  dmlab_release_context(&ctx);
}

lua::NResultsOr ModelModule(lua_State* L) {
  if (auto* ctx = static_cast<const DeepmindCalls*>(
          lua_touserdata(L, lua_upvalueindex(1)))) {
    LuaModel::CreateObject(L, ctx);
    return 1;
  } else {
    return "Missing context!";
  }
}

constexpr char kLuaModelCircularLayout[] = R"(
local model = require 'dmlab.system.model'
return model:circularLayout(10, 4)
)";

TEST(DeepmindModelLibTest, LayoutCircular) {
  DeepmindContext ctx{};
  ASSERT_EQ(0, dmlab_create_context(TestSrcDir().c_str(), &ctx, nullptr,
                                    nullptr, nullptr));

  auto lua_vm = lua::CreateVm();
  lua_vm.AddPathToSearchers(TestSrcDir());
  lua_vm.AddCModuleToSearchers("dmlab.system.tensor",
                               tensor::LuaTensorConstructors);
  lua_vm.AddCModuleToSearchers("dmlab.system.model", &lua::Bind<ModelModule>,
                               {const_cast<DeepmindCalls*>(&ctx.calls)});
  auto* L = lua_vm.get();
  deepmind::lab::tensor::LuaTensorRegister(L);
  deepmind::lab::LuaModel::Register(L);

  ASSERT_THAT(
      lua::PushScript(L, kLuaModelCircularLayout, "kLuaModelCircularLayout"),
      lua::testing::IsOkAndHolds(1))
      << "Missing script";
  ASSERT_THAT(lua::Call(L, 0), lua::testing::IsOkAndHolds(1))
      << "Missing result";

  Model test_model;
  ASSERT_TRUE(Read(L, -1, &test_model));
  const auto& test_locators = test_model.locators;
  ASSERT_EQ(test_locators.size(), 4);
  const Eigen::Vector3f kRefPos[4] = {{10.0f, 0.0f, 0.0f},
                                      {0.0f, -10.0f, 0.0f},
                                      {-10.0f, 0.0f, 0.0f},
                                      {0.0f, 10.0f, 0.0f}};
  for (int i = 0; i < 4; ++i) {
    const auto locator_name = absl::StrCat("layout_", i);
    const auto lit = test_locators.find(locator_name);
    ASSERT_NE(lit, test_locators.end());
    const auto& test_xfrm = lit->second;
    const auto& ref_pos = kRefPos[i];
    const auto test_pos = test_xfrm.matrix().col(3);
    const auto ref_y_dir = ref_pos.normalized();
    const auto test_y_dir = test_xfrm.matrix().col(1);
    const auto test_z_dir = test_xfrm.matrix().col(2);
    const auto ref_z_dir = Eigen::Vector3f::UnitZ();
    for (int j = 0; j < 3; ++j) {
      EXPECT_NEAR(test_pos[j], ref_pos[j], kEpsilon);
      EXPECT_NEAR(test_y_dir[j], ref_y_dir[j], kEpsilon);
      EXPECT_NEAR(test_z_dir[j], ref_z_dir[j], kEpsilon);
    }
  }

  dmlab_release_context(&ctx);
}

constexpr char kLuaModelLinearLayout[] = R"(
local model = require 'dmlab.system.model'
return model:linearLayout(10, 5)
)";

TEST(DeepmindModelLibTest, LayoutLinear) {
  DeepmindContext ctx{};
  ASSERT_EQ(0, dmlab_create_context(TestSrcDir().c_str(), &ctx, nullptr,
                                    nullptr, nullptr));

  auto lua_vm = lua::CreateVm();
  lua_vm.AddPathToSearchers(TestSrcDir());
  lua_vm.AddCModuleToSearchers("dmlab.system.tensor",
                               tensor::LuaTensorConstructors);
  lua_vm.AddCModuleToSearchers("dmlab.system.model", &lua::Bind<ModelModule>,
                               {const_cast<DeepmindCalls*>(&ctx.calls)});
  auto* L = lua_vm.get();
  deepmind::lab::tensor::LuaTensorRegister(L);
  deepmind::lab::LuaModel::Register(L);

  ASSERT_THAT(
      lua::PushScript(L, kLuaModelLinearLayout, "kLuaModelLinearLayout"),
      lua::testing::IsOkAndHolds(1))
      << "Missing script";
  ASSERT_THAT(lua::Call(L, 0), lua::testing::IsOkAndHolds(1))
      << "Missing result";

  Model test_model;
  ASSERT_TRUE(Read(L, -1, &test_model));
  const auto& test_locators = test_model.locators;
  ASSERT_EQ(test_locators.size(), 5);
  const Eigen::Vector3f kRefPos[5] = {{-5.0f, 0.0f, 0.0f},
                                      {-2.5f, 0.0f, 0.0f},
                                      {0.0f, 0.0f, 0.0f},
                                      {2.5f, 0.0f, 0.0f},
                                      {5.0f, 0.0f, 0.0f}};
  const Eigen::Matrix3f kRefFrame = Eigen::Matrix3f::Identity();
  for (int i = 0; i < 5; ++i) {
    const auto locator_name = absl::StrCat("layout_", i);
    const auto lit = test_locators.find(locator_name);
    ASSERT_NE(lit, test_locators.end());
    const auto& test_xfrm = lit->second;
    const auto& ref_pos = kRefPos[i];
    const auto test_pos = test_xfrm.matrix().col(3);
    for (int j = 0; j < 3; ++j) {
      EXPECT_NEAR(test_pos[j], ref_pos[j], kEpsilon);
    }
    const auto test_frame = test_xfrm.linear();
    EXPECT_NEAR((test_frame - kRefFrame).norm(), 0.0f, kEpsilon);
  }

  dmlab_release_context(&ctx);
}

}  // namespace
}  // namespace lab
}  // namespace deepmind
