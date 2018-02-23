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

#include "deepmind/model_generation/transform_lua.h"

#include "gtest/gtest.h"
#include "Eigen/Geometry"
#include "deepmind/lua/call.h"
#include "deepmind/lua/n_results_or_test_util.h"
#include "deepmind/lua/push_script.h"
#include "deepmind/lua/vm.h"
#include "deepmind/model_generation/geometry_util.h"
#include "deepmind/support/test_srcdir.h"
#include "deepmind/tensor/lua_tensor.h"

namespace deepmind {
namespace lab {
namespace {

using geometry::kEpsilon;

TEST(DeepmindTransformTest, PushRead) {
  Eigen::Matrix4f ref_data;
  ref_data << 0.0f, 1.0f, 2.0f, 3.0f,  //
      4.0f, 5.0f, 6.0f, 7.0f,          //
      8.0f, 9.0f, 10.0f, 11.0f,        //
      12.0f, 13.0f, 14.0f, 15.0f;
  auto lua_vm = lua::CreateVm();
  tensor::LuaTensorRegister(lua_vm.get());
  Transform ref_xfrm;
  ref_xfrm = ref_data;
  Push(lua_vm.get(), ref_xfrm);
  Transform tst_xfrm;
  ASSERT_TRUE(IsFound(Read(lua_vm.get(), -1, &tst_xfrm)));
  EXPECT_NEAR((ref_data - tst_xfrm.matrix()).norm(), 0.0f, kEpsilon);
}

constexpr char kTransformWrongDims[] = R"(
local tensor = require 'dmlab.system.tensor'
local matrix = tensor.FloatTensor{
  { 0.0, 1.0, 2.0, 3.0 },
  { 4.0, 5.0, 6.0, 7.0 }
}
return matrix
)";

TEST(DeepmindTransformTest, ReadWrongDims) {
  auto lua_vm = lua::CreateVm();
  lua_vm.AddPathToSearchers(TestSrcDir());
  lua_vm.AddCModuleToSearchers("dmlab.system.tensor",
                               tensor::LuaTensorConstructors);
  auto* L = lua_vm.get();
  deepmind::lab::tensor::LuaTensorRegister(L);
  ASSERT_THAT(lua::PushScript(L, kTransformWrongDims, "kTransformWrongDims"),
              lua::testing::IsOkAndHolds(1))
      << "Missing script";
  ASSERT_THAT(lua::Call(L, 0), lua::testing::IsOkAndHolds(1))
      << "Missing result";
  Transform tst_matrix;
  EXPECT_TRUE(IsTypeMismatch(Read(L, -1, &tst_matrix)));
}

}  // namespace
}  // namespace lab
}  // namespace deepmind
