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

#include "deepmind/model_generation/lua_transform.h"

#include "gtest/gtest.h"
#include "deepmind/lua/bind.h"
#include "deepmind/lua/call.h"
#include "deepmind/lua/n_results_or_test_util.h"
#include "deepmind/lua/push_script.h"
#include "deepmind/lua/vm.h"
#include "deepmind/model_generation/geometry_util.h"
#include "deepmind/model_generation/transform_lua.h"
#include "deepmind/support/test_srcdir.h"
#include "deepmind/tensor/lua_tensor.h"

namespace deepmind {
namespace lab {
namespace {

using ::deepmind::lab::TestSrcDir;

constexpr char kTransformTranslate[] = R"(
local transform = require 'dmlab.system.transform'
return transform.translate{1.0, 2.0, 3.0}
)";

TEST(DeepmindTransformTest, Translate) {
  auto lua_vm = lua::CreateVm();
  lua_vm.AddPathToSearchers(TestSrcDir());
  lua_vm.AddCModuleToSearchers("dmlab.system.tensor",
                               tensor::LuaTensorConstructors);
  lua_vm.AddCModuleToSearchers("dmlab.system.transform", LuaTransform::Require);
  auto* L = lua_vm.get();
  tensor::LuaTensorRegister(L);
  ASSERT_THAT(lua::PushScript(L, kTransformTranslate, "kTransformTranslate"),
              lua::testing::IsOkAndHolds(1))
      << "Missing script";
  ASSERT_THAT(lua::Call(L, 0), lua::testing::IsOkAndHolds(1))
      << "Missing result";
  Eigen::Matrix4f ref_data;
  ref_data << 1.0f, 0.0, 0.0, 1.0f,  //
      0.0f, 1.0f, 0.0f, 2.0f,        //
      0.0f, 0.0f, 1.0f, 3.0f,        //
      0.0f, 0.0f, 0.0f, 1.0f;
  Transform tst_xfrm;
  EXPECT_TRUE(Read(L, -1, &tst_xfrm));
  EXPECT_NEAR((tst_xfrm.matrix() - ref_data).norm(), 0.0f, geometry::kEpsilon);
}

constexpr char kTransformRotate[] = R"(
local transform = require 'dmlab.system.transform'
return transform.rotate(30.0, {0.57735026919, 0.57735026919, 0.57735026919})
)";

TEST(DeepmindTransformTest, Rotate) {
  auto lua_vm = lua::CreateVm();
  lua_vm.AddPathToSearchers(TestSrcDir());
  lua_vm.AddCModuleToSearchers("dmlab.system.tensor",
                               tensor::LuaTensorConstructors);
  lua_vm.AddCModuleToSearchers("dmlab.system.transform", LuaTransform::Require);
  auto* L = lua_vm.get();
  tensor::LuaTensorRegister(L);
  ASSERT_THAT(lua::PushScript(L, kTransformRotate, "kTransformRotate"),
              lua::testing::IsOkAndHolds(1))
      << "Missing script";
  ASSERT_THAT(lua::Call(L, 0), lua::testing::IsOkAndHolds(1))
      << "Missing result";
  Eigen::Matrix4f ref_data;
  ref_data << 0.910684f, -0.244017f, 0.333333f, 0.0f,  //
      0.333333f, 0.910684f, -0.244017f, 0.0f,          //
      -0.244017f, 0.333333f, 0.910684f, 0.0f,          //
      0.0f, 0.0f, 0.0f, 1.0f;
  Transform tst_xfrm;
  EXPECT_TRUE(Read(L, -1, &tst_xfrm));
  EXPECT_NEAR((tst_xfrm.matrix() - ref_data).norm(), 0.0f, geometry::kEpsilon);
}
constexpr char kTransformScale[] = R"(
local transform = require 'dmlab.system.transform'
return transform.scale{1.0, 2.0, 3.0}
)";

TEST(DeepmindTransformTest, Scale) {
  auto lua_vm = lua::CreateVm();
  lua_vm.AddPathToSearchers(TestSrcDir());
  lua_vm.AddCModuleToSearchers("dmlab.system.tensor",
                               tensor::LuaTensorConstructors);
  lua_vm.AddCModuleToSearchers("dmlab.system.transform", LuaTransform::Require);
  auto* L = lua_vm.get();
  tensor::LuaTensorRegister(L);
  ASSERT_THAT(lua::PushScript(L, kTransformScale, "kTransformScale"),
              lua::testing::IsOkAndHolds(1))
      << "Missing script";
  ASSERT_THAT(lua::Call(L, 0), lua::testing::IsOkAndHolds(1))
      << "Missing result";
  Eigen::Matrix4f ref_data;
  ref_data << 1.0f, 0.0, 0.0, 0.0f,  //
      0.0f, 2.0f, 0.0f, 0.0f,        //
      0.0f, 0.0f, 3.0f, 0.0f,        //
      0.0f, 0.0f, 0.0f, 1.0f;
  Transform tst_xfrm;
  EXPECT_TRUE(Read(L, -1, &tst_xfrm));
  EXPECT_NEAR((tst_xfrm.matrix() - ref_data).norm(), 0.0f, geometry::kEpsilon);
}

}  // namespace
}  // namespace lab
}  // namespace deepmind
