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

#include "deepmind/model_generation/model_lua.h"

#include "gtest/gtest.h"
#include "deepmind/lua/call.h"
#include "deepmind/lua/n_results_or_test_util.h"
#include "deepmind/lua/push_script.h"
#include "deepmind/lua/vm.h"
#include "deepmind/model_generation/geometry_util.h"
#include "deepmind/support/test_srcdir.h"
#include "deepmind/tensor/lua_tensor.h"
#include "deepmind/util/default_read_only_file_system.h"

namespace deepmind {
namespace lab {
namespace {

using geometry::kEpsilon;

TEST(DeepmindModelTest, PushRead) {
  Transform xfrm;
  xfrm = Eigen::Matrix4f::Identity();
  const Model ref_model = {
      "ref_model",  //
      {
          {
              "cube_surface",  //
              {
                  -0.5, -0.5, -0.5, 0.0,  0.0,  -1.0, 0.0, 0.0,  //
                  0.5,  -0.5, -0.5, 0.0,  0.0,  -1.0, 1.0, 0.0,  //
                  0.5,  0.5,  -0.5, 0.0,  0.0,  -1.0, 1.0, 1.0,  //
                  -0.5, 0.5,  -0.5, 0.0,  0.0,  -1.0, 0.0, 1.0,  //
                  0.5,  -0.5, -0.5, 1.0,  0.0,  0.0,  0.0, 0.0,  //
                  0.5,  -0.5, 0.5,  1.0,  0.0,  0.0,  1.0, 0.0,  //
                  0.5,  0.5,  0.5,  1.0,  0.0,  0.0,  1.0, 1.0,  //
                  0.5,  0.5,  -0.5, 1.0,  0.0,  0.0,  0.0, 1.0,  //
                  0.5,  -0.5, 0.5,  0.0,  0.0,  1.0,  0.0, 0.0,  //
                  -0.5, -0.5, 0.5,  0.0,  0.0,  1.0,  1.0, 0.0,  //
                  -0.5, 0.5,  0.5,  0.0,  0.0,  1.0,  1.0, 1.0,  //
                  0.5,  0.5,  0.5,  0.0,  0.0,  1.0,  0.0, 1.0,  //
                  -0.5, -0.5, 0.5,  -1.0, 0.0,  0.0,  0.0, 0.0,  //
                  -0.5, -0.5, -0.5, -1.0, 0.0,  0.0,  1.0, 0.0,  //
                  -0.5, 0.5,  -0.5, -1.0, 0.0,  0.0,  1.0, 1.0,  //
                  -0.5, 0.5,  0.5,  -1.0, 0.0,  0.0,  0.0, 1.0,  //
                  -0.5, 0.5,  -0.5, 0.0,  1.0,  0.0,  0.0, 0.0,  //
                  0.5,  0.5,  -0.5, 0.0,  1.0,  0.0,  1.0, 0.0,  //
                  0.5,  0.5,  0.5,  0.0,  1.0,  0.0,  1.0, 1.0,  //
                  -0.5, 0.5,  0.5,  0.0,  1.0,  0.0,  0.0, 1.0,  //
                  0.5,  -0.5, -0.5, 0.0,  -1.0, 0.0,  0.0, 0.0,  //
                  -0.5, -0.5, -0.5, 0.0,  -1.0, 0.0,  1.0, 0.0,  //
                  -0.5, -0.5, 0.5,  0.0,  -1.0, 0.0,  1.0, 1.0,  //
                  0.5,  -0.5, 0.5,  0.0,  -1.0, 0.0,  0.0, 1.0   //
              },                                                 //
              {
                  0,  1,  2,         //
                  0,  2,  3,         //
                  4,  5,  6,         //
                  4,  6,  7,         //
                  8,  9,  10,        //
                  8,  10, 11,        //
                  12, 13, 14,        //
                  12, 14, 15,        //
                  16, 17, 18,        //
                  16, 18, 19,        //
                  20, 21, 22,        //
                  20, 22, 23         //
              },                     //
              "textures/model/beam"  //
          }                          //
      },                             //
      {
          {
              "centre_locator",  //
              xfrm               //
          }                      //
      }                          //
  };
  Model test_model;

  auto lua_vm = lua::CreateVm();
  auto* L = lua_vm.get();
  tensor::LuaTensorRegister(L);

  int top = lua_gettop(L);
  Push(L, ref_model);
  ASSERT_TRUE(IsFound(Read(L, -1, &test_model)));
  lua_pop(L, 1);
  ASSERT_EQ(top, lua_gettop(L));

  ASSERT_EQ(ref_model.surfaces.size(), test_model.surfaces.size());
  for (std::size_t i = 0; i < ref_model.surfaces.size(); ++i) {
    const Model::Surface& ref_surface = ref_model.surfaces[i];
    const Model::Surface& test_surface = test_model.surfaces[i];
    EXPECT_EQ(ref_surface.name, test_surface.name);
    EXPECT_EQ(ref_surface.vertices, test_surface.vertices);
    EXPECT_EQ(ref_surface.indices, test_surface.indices);
    EXPECT_EQ(ref_surface.shader_name, test_surface.shader_name);
  }

  for (const auto& ref_locator : ref_model.locators) {
    auto lit = test_model.locators.find(ref_locator.first);
    ASSERT_NE(lit, test_model.locators.end());
    const auto ref_matrix = ref_locator.second.matrix();
    const auto test_matrix = lit->second.matrix();
    EXPECT_NEAR((test_matrix - ref_matrix).norm(), 0.0f, kEpsilon);
  }
}

constexpr char kModelIndicesOutOfRange[] = R"(
local tensor = require 'dmlab.system.tensor'
local cube = {
    surfaces = {
        cube_surface = {
            vertices = tensor.FloatTensor{
                { -0.5, -0.5, -0.5,  0.0,  0.0, -1.0, 0.0, 0.0 }
            },
            indices = tensor.Int32Tensor{
                {  1,  2,  3 }
            },
            shader_name = 'textures/model/beam'
        }
    }
}
return cube
)";

TEST(DeepmindModelTest, ReadIndicesOutOfRange) {
  auto lua_vm = lua::CreateVm();
  lua_vm.AddPathToSearchers(TestSrcDir());
  void* default_fs = const_cast<DeepMindReadOnlyFileSystem*>(
      util::DefaultReadOnlyFileSystem());
  lua_vm.AddCModuleToSearchers("dmlab.system.tensor",
                               tensor::LuaTensorConstructors, {default_fs});
  auto* L = lua_vm.get();
  tensor::LuaTensorRegister(L);

  ASSERT_THAT(
      lua::PushScript(L, kModelIndicesOutOfRange, "kModelIndicesOutOfRange"),
      lua::testing::IsOkAndHolds(1))
      << "Missing script";
  ASSERT_THAT(lua::Call(L, 0), lua::testing::IsOkAndHolds(1))
      << "Missing result";

  Model test_model;
  EXPECT_TRUE(IsTypeMismatch(Read(L, -1, &test_model)));
}

constexpr char kModelNoVertices[] = R"(
local tensor = require 'dmlab.system.tensor'
local cube = {
    surfaces = {
        cube_surface = {
            indices = tensor.Int32Tensor{
                {  1,  2,  3 }
            },
            shader_name = 'textures/model/beam'
        }
    }
}
return cube
)";

TEST(DeepmindModelTest, ReadNoVertices) {
  auto lua_vm = lua::CreateVm();
  lua_vm.AddPathToSearchers(TestSrcDir());
  void* default_fs = const_cast<DeepMindReadOnlyFileSystem*>(
      util::DefaultReadOnlyFileSystem());
  lua_vm.AddCModuleToSearchers("dmlab.system.tensor",
                               tensor::LuaTensorConstructors, {default_fs});
  auto* L = lua_vm.get();
  tensor::LuaTensorRegister(L);

  ASSERT_THAT(lua::PushScript(L, kModelNoVertices, "kModelNoVertices"),
              lua::testing::IsOkAndHolds(1))
      << "Missing script";
  ASSERT_THAT(lua::Call(L, 0), lua::testing::IsOkAndHolds(1))
      << "Missing result";

  Model test_model;
  EXPECT_TRUE(IsTypeMismatch(Read(L, -1, &test_model)));
}

constexpr char kModelVerticesWrongSize[] = R"(
local tensor = require 'dmlab.system.tensor'
local cube = {
    surfaces = {
        cube_surface = {
            vertices = tensor.FloatTensor{
                { -0.5, -0.5, -0.5,  0.0,  0.0, -1.0 }
            },
            indices = tensor.Int32Tensor{
                { 1, 1, 1 }
            },
            shader_name = 'textures/model/beam'
        }
    }
}
return cube
)";

TEST(DeepmindModelTest, ReadVerticesWrongSize) {
  auto lua_vm = lua::CreateVm();
  lua_vm.AddPathToSearchers(TestSrcDir());
  void* default_fs = const_cast<DeepMindReadOnlyFileSystem*>(
      util::DefaultReadOnlyFileSystem());
  lua_vm.AddCModuleToSearchers("dmlab.system.tensor",
                               tensor::LuaTensorConstructors, {default_fs});
  auto* L = lua_vm.get();
  tensor::LuaTensorRegister(L);

  ASSERT_THAT(
      lua::PushScript(L, kModelVerticesWrongSize, "kModelVerticesWrongSize"),
      lua::testing::IsOkAndHolds(1))
      << "Missing script";
  ASSERT_THAT(lua::Call(L, 0), lua::testing::IsOkAndHolds(1))
      << "Missing result";

  Model test_model;
  EXPECT_TRUE(IsTypeMismatch(Read(L, -1, &test_model)));
}

constexpr char kModelNoIndices[] = R"(
local tensor = require 'dmlab.system.tensor'
local cube = {
    surfaces = {
        cube_surface = {
            vertices = tensor.FloatTensor{
                { -0.5, -0.5, -0.5,  0.0,  0.0, -1.0, 0.0, 0.0 }
            },
            shader_name = 'textures/model/beam'
        }
    }
}
return cube
)";

TEST(DeepmindModelTest, ReadNoIndices) {
  auto lua_vm = lua::CreateVm();
  lua_vm.AddPathToSearchers(TestSrcDir());
  void* default_fs = const_cast<DeepMindReadOnlyFileSystem*>(
      util::DefaultReadOnlyFileSystem());
  lua_vm.AddCModuleToSearchers("dmlab.system.tensor",
                               tensor::LuaTensorConstructors, {default_fs});
  auto* L = lua_vm.get();
  tensor::LuaTensorRegister(L);

  ASSERT_THAT(lua::PushScript(L, kModelNoIndices, "kModelNoIndices"),
              lua::testing::IsOkAndHolds(1))
      << "Missing script";
  ASSERT_THAT(lua::Call(L, 0), lua::testing::IsOkAndHolds(1))
      << "Missing result";

  Model test_model;
  EXPECT_TRUE(IsTypeMismatch(Read(L, -1, &test_model)));
}

constexpr char kModelIndicesWrongSize[] = R"(
local tensor = require 'dmlab.system.tensor'
local cube = {
    surfaces = {
        cube_surface = {
            vertices = tensor.FloatTensor{
                { -0.5, -0.5, -0.5,  0.0,  0.0, -1.0, 0.0, 0.0 }
            },
            indices = tensor.Int32Tensor{
                { 1, 1, 1, 1 }
            },
            shader_name = 'textures/model/beam'
        }
    }
}
return cube
)";

TEST(DeepmindModelTest, ReadIndicesWrongSize) {
  auto lua_vm = lua::CreateVm();
  lua_vm.AddPathToSearchers(TestSrcDir());
  void* default_fs = const_cast<DeepMindReadOnlyFileSystem*>(
      util::DefaultReadOnlyFileSystem());
  lua_vm.AddCModuleToSearchers("dmlab.system.tensor",
                               tensor::LuaTensorConstructors, {default_fs});
  auto* L = lua_vm.get();
  tensor::LuaTensorRegister(L);

  ASSERT_THAT(
      lua::PushScript(L, kModelIndicesWrongSize, "kModelIndicesWrongSize"),
      lua::testing::IsOkAndHolds(1))
      << "Missing script";
  ASSERT_THAT(lua::Call(L, 0), lua::testing::IsOkAndHolds(1))
      << "Missing result";

  Model test_model;
  EXPECT_TRUE(IsTypeMismatch(Read(L, -1, &test_model)));
}

constexpr char kModelNoShader[] = R"(
local tensor = require 'dmlab.system.tensor'
local cube = {
    surfaces = {
        cube_surface = {
            vertices = tensor.FloatTensor{
                { -0.5, -0.5, -0.5,  0.0,  0.0, -1.0, 0.0, 0.0 }
            },
            indices = tensor.Int32Tensor{
                { 1, 1, 1 }
            }
        }
    }
}
return cube
)";

TEST(DeepmindModelTest, ReadNoShader) {
  auto lua_vm = lua::CreateVm();
  lua_vm.AddPathToSearchers(TestSrcDir());
  void* default_fs = const_cast<DeepMindReadOnlyFileSystem*>(
      util::DefaultReadOnlyFileSystem());
  lua_vm.AddCModuleToSearchers("dmlab.system.tensor",
                               tensor::LuaTensorConstructors, {default_fs});
  auto* L = lua_vm.get();
  tensor::LuaTensorRegister(L);

  ASSERT_THAT(lua::PushScript(L, kModelNoShader, "kModelNoShader"),
              lua::testing::IsOkAndHolds(1))
      << "Missing script";
  ASSERT_THAT(lua::Call(L, 0), lua::testing::IsOkAndHolds(1))
      << "Missing result";

  Model test_model;
  EXPECT_TRUE(IsTypeMismatch(Read(L, -1, &test_model)));
}

}  // namespace
}  // namespace lab
}  // namespace deepmind
