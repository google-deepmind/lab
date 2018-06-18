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

#include "deepmind/tensor/lua_tensor.h"

#include <cstdint>
#include <fstream>
#include <tuple>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "deepmind/lua/bind.h"
#include "deepmind/lua/call.h"
#include "deepmind/lua/n_results_or_test_util.h"
#include "deepmind/lua/push_script.h"
#include "deepmind/lua/read.h"
#include "deepmind/lua/table_ref.h"
#include "deepmind/lua/vm.h"
#include "deepmind/util/default_read_only_file_system.h"

namespace deepmind {
namespace lab {
namespace {

using ::deepmind::lab::lua::testing::IsOkAndHolds;
using ::deepmind::lab::lua::testing::StatusIs;
using ::testing::AllOf;
using ::testing::ElementsAre;
using ::testing::HasSubstr;

class LuaTensorTest : public ::testing::Test {
 protected:
  LuaTensorTest() : lua_vm_(lua::CreateVm()) {
    auto* L = lua_vm_.get();
    LuaRandom::Register(L);
    void* default_fs = const_cast<DeepMindReadOnlyFileSystem*>(
        util::DefaultReadOnlyFileSystem());
    lua_vm_.AddCModuleToSearchers(
        "dmlab.system.sys_random", &lua::Bind<LuaRandom::Require>,
        {&prbg_, reinterpret_cast<void*>(static_cast<std::uintptr_t>(0))});
    tensor::LuaTensorRegister(L);
    lua_vm_.AddCModuleToSearchers("dmlab.system.tensor",
                                  tensor::LuaTensorConstructors, {default_fs});
  }
  std::mt19937_64 prbg_;
  lua::Vm lua_vm_;
};

constexpr char kLuaTensorCreate[] = R"(
local tensor = require 'dmlab.system.tensor'
local tensor_types = {
    "ByteTensor",
    "CharTensor",
    "Int16Tensor",
    "Int32Tensor",
    "Int64Tensor",
    "FloatTensor",
    "DoubleTensor"
}

local result = {}
for i, tensor_type in ipairs(tensor_types) do
  result[tensor_type] = tensor[tensor_type](3, 3)
  local k = 0
  result[tensor_type]:apply(function(val)
    assert(val == 0)
    k = k + 1
    return k
  end)
  assert(result[tensor_type]:type() == "deepmind.lab.tensor." .. tensor_type)
end
return result
)";

TEST_F(LuaTensorTest, CreateTensor) {
  lua_State* L = lua_vm_.get();

  auto syntax = lua::PushScript(
      L, kLuaTensorCreate, sizeof(kLuaTensorCreate) - 1, "kLuaTensorCreate");
  ASSERT_TRUE(syntax.ok()) << syntax.error();
  auto result = lua::Call(L, 0);
  ASSERT_THAT(result, IsOkAndHolds(1));
  {
    lua::TableRef tableref;
    ASSERT_TRUE(lua::Read(L, -1, &tableref));
    lua_pop(L, 1);
    tableref.LookUpToStack("DoubleTensor");
  }
  auto* tensor_double = tensor::LuaTensor<double>::ReadObject(L, -1);
  int counter = 0;
  tensor_double->tensor_view().ForEach([&counter](double val) {
    ++counter;
    EXPECT_DOUBLE_EQ(counter, val);
    return true;
  });
}

constexpr char kLuaTensorRange[] = R"(
local tensor = require 'dmlab.system.tensor'

local v0, v1, v2, v3, v4
v0 = tensor.DoubleTensor{range = {5}}
v1 = tensor.DoubleTensor{range = {2, 5}}
v2 = tensor.DoubleTensor{range = {2, 5, 0.5}}
v3 = tensor.DoubleTensor{range = {2, 4.9, 0.5}}
v4 = tensor.DoubleTensor{range = {2, -1, -1}}
assert(v0 == tensor.DoubleTensor{1, 2, 3, 4, 5})
assert(v1 == tensor.DoubleTensor{2, 3, 4, 5})
assert(v2 == tensor.DoubleTensor{2, 2.5, 3, 3.5, 4, 4.5, 5})
assert(v3 == tensor.DoubleTensor{2, 2.5, 3, 3.5, 4, 4.5})
assert(v4 == tensor.DoubleTensor{2, 1, 0, -1})
)";

TEST_F(LuaTensorTest, TensorRange) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kLuaTensorRange, sizeof(kLuaTensorRange) - 1,
                              "kLuaTensorRange"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr char kInvalidLuaTensorRange[] = R"(
local tensor = require 'dmlab.system.tensor'

local v0 = tensor.DoubleTensor{range={1, -5}}
)";

TEST_F(LuaTensorTest, InvalidLuaTensorRange) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kInvalidLuaTensorRange,
                              sizeof(kInvalidLuaTensorRange) - 1,
                              "kInvalidLuaTensorRange"),
              IsOkAndHolds(1));
  auto result = lua::Call(L, 0);
  EXPECT_FALSE(result.ok());
  EXPECT_THAT(result.error(), HasSubstr("Invalid Tensor range."));
}

constexpr char kValueOps[] = R"(
local tensor = require 'dmlab.system.tensor'

local v0, v1
v0 = tensor.ByteTensor{{1, 2}, {3, 4}}
assert(v0:select(1, 1) == tensor.ByteTensor{1, 2})
assert(v0:select(1, 2) == tensor.ByteTensor{3, 4})
assert(v0:select(2, 1) == tensor.ByteTensor{1, 3})
assert(v0:select(2, 2) == tensor.ByteTensor{2, 4})
v0 = tensor.ByteTensor(3, 3):fill(1):add(1)
v1 = tensor.ByteTensor(3, 3):fill(3)
assert(v0 ~= v1)
v0:add(1)
assert(v0 == v1)
v0 = tensor.ByteTensor{{1, 2}, {3, 4}}
local k = 0
v1 = tensor.ByteTensor(2,2):apply(function(val)
    k = k + 1
    return k
  end)
assert(v0 == v1)
assert(v0(1, 1):val() == 1)
assert(v0(1, 2):val() == 2)
assert(v0(2, 1):val() == 3)
assert(v0(2, 2):val() == 4)
)";

TEST_F(LuaTensorTest, ValueOps) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kValueOps, sizeof(kValueOps) - 1, "kValueOps"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr char kShape[] = R"(
local tensor = require 'dmlab.system.tensor'
local bt = tensor.ByteTensor{{1, 2}, {3, 4}, {5, 6}}
local shape = bt:shape()
assert(shape[1] == 3)
assert(shape[2] == 2)
assert(#shape == 2)
)";

TEST_F(LuaTensorTest, Shape) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kShape, sizeof(kShape) - 1, "kShape"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr char kClone[] = R"(
local tensor = require 'dmlab.system.tensor'
local bt = tensor.ByteTensor{{1, 2}, {3, 4}, {5, 6}}
return bt, bt:clone()
)";

TEST_F(LuaTensorTest, Clone) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kClone, sizeof(kClone) - 1, "kClone"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(2));
  auto* bt = tensor::LuaTensor<std::uint8_t>::ReadObject(L, -2);
  auto* bt_clone = tensor::LuaTensor<std::uint8_t>::ReadObject(L, -1);
  ASSERT_TRUE(bt != nullptr);
  ASSERT_TRUE(bt_clone != nullptr);
  EXPECT_TRUE(bt->tensor_view().storage() != bt_clone->tensor_view().storage());
  EXPECT_TRUE(bt->tensor_view() == bt_clone->tensor_view());
}

constexpr char kSum[] = R"(
local tensor = require 'dmlab.system.tensor'
local bt = tensor.ByteTensor{{1, 2}, {3, 4}}
return bt:sum()
)";

TEST_F(LuaTensorTest, Sum) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kSum, std::strlen(kSum), "kSum"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(1));
  double sum;
  ASSERT_TRUE(IsFound(lua::Read(L, 1, &sum)));
  EXPECT_EQ(sum, 10);
}

constexpr char kProduct[] = R"(
local tensor = require 'dmlab.system.tensor'
local bt = tensor.ByteTensor{{1, 2}, {3, 4}}
return bt:product()
)";

TEST_F(LuaTensorTest, Product) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kProduct, std::strlen(kProduct), "kProduct"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(1));
  double prod;
  ASSERT_TRUE(IsFound(lua::Read(L, 1, &prod)));
  EXPECT_EQ(prod, 24.0);
}

constexpr char kLengthSquared[] = R"(
local tensor = require 'dmlab.system.tensor'
local bt = tensor.ByteTensor{{1, 2}, {3, 4}}
return bt:lengthSquared()
)";

TEST_F(LuaTensorTest, LengthSquared) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kLengthSquared, std::strlen(kLengthSquared),
                              "kLengthSquared"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(1));
  double length_sqr;
  ASSERT_TRUE(IsFound(lua::Read(L, 1, &length_sqr)));
  EXPECT_EQ(length_sqr, 1 + 4 + 9 + 16);
}

constexpr char kDotProduct[] = R"(
local tensor = require 'dmlab.system.tensor'
local t1 = tensor.Int32Tensor{1, 2, 3, 4}
local t2 = tensor.Int32Tensor{-1, 2, -3, 4}
return t1:dot(t2)
)";

TEST_F(LuaTensorTest, DotProduct) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kDotProduct, std::strlen(kDotProduct),
                              "kDotProduct"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(1));
  double result;
  ASSERT_TRUE(IsFound(lua::Read(L, 1, &result)));
  EXPECT_EQ(result, -1 + 4 - 9 + 16);
}

constexpr char kTranspose[] = R"(
local tensor = require 'dmlab.system.tensor'
local bt = tensor.ByteTensor{{1, 2}, {3, 4}, {5, 6}}
local transpose = tensor.ByteTensor{{1, 3, 5}, {2, 4, 6}}
assert (bt:transpose(1, 2) == transpose)
return bt, bt:transpose(1, 2)
)";

TEST_F(LuaTensorTest, Transpose) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(
      lua::PushScript(L, kTranspose, sizeof(kTranspose) - 1, "kTranspose"),
      IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(2));
  auto* bt = tensor::LuaTensor<std::uint8_t>::ReadObject(L, -2);
  auto* bt_alt = tensor::LuaTensor<std::uint8_t>::ReadObject(L, -1);
  ASSERT_TRUE(bt != nullptr);
  ASSERT_TRUE(bt_alt != nullptr);
  EXPECT_TRUE(bt->tensor_view().storage() == bt_alt->tensor_view().storage());
}

constexpr char kSelect[] = R"(
local tensor = require 'dmlab.system.tensor'
local bt = tensor.ByteTensor{{1, 2}, {3, 4}, {5, 6}}
local select = tensor.ByteTensor{2, 4, 6}
assert (bt:select(2, 2) == select)
return bt, bt:select(2, 2)
)";

TEST_F(LuaTensorTest, Select) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kSelect, sizeof(kSelect) - 1, "kSelect"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(2));
  auto* bt = tensor::LuaTensor<std::uint8_t>::ReadObject(L, -2);
  auto* bt_alt = tensor::LuaTensor<std::uint8_t>::ReadObject(L, -1);
  ASSERT_TRUE(bt != nullptr);
  ASSERT_TRUE(bt_alt != nullptr);
  EXPECT_TRUE(bt->tensor_view().storage() == bt_alt->tensor_view().storage());
}

constexpr char kNarrow[] = R"(
local tensor = require 'dmlab.system.tensor'
local bt = tensor.ByteTensor{{1, 2}, {3, 4}, {5, 6}}
local narrow = tensor.ByteTensor{{3, 4}, {5, 6}}
assert (bt:narrow(1, 2, 2) == narrow)
return bt, bt:narrow(1, 2, 2)
)";

TEST_F(LuaTensorTest, Narrow) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kNarrow, sizeof(kNarrow) - 1, "kNarrow"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(2));
  auto* bt = tensor::LuaTensor<std::uint8_t>::ReadObject(L, -2);
  auto* bt_alt = tensor::LuaTensor<std::uint8_t>::ReadObject(L, -1);
  ASSERT_TRUE(bt != nullptr);
  ASSERT_TRUE(bt_alt != nullptr);
  EXPECT_TRUE(bt->tensor_view().storage() == bt_alt->tensor_view().storage());
}

constexpr char kReverse[] = R"(
local tensor = require 'dmlab.system.tensor'
local bt = tensor.ByteTensor{{1, 2}, {3, 4}, {5, 6}}
local narrow = tensor.ByteTensor{{3, 4}, {5, 6}}
assert (bt:reverse(1) == tensor.ByteTensor{{5, 6}, {3, 4}, {1, 2}})
assert (bt:reverse(2) == tensor.ByteTensor{{2, 1}, {4, 3}, {6, 5}})
assert (bt:reverse(2):reverse(1) == tensor.ByteTensor{{6, 5}, {4, 3}, {2, 1}})
)";

TEST_F(LuaTensorTest, Reverse) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kReverse, sizeof(kReverse) - 1, "kReverse"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr char kApply[] = R"(
local tensor = require 'dmlab.system.tensor'
local bt = tensor.ByteTensor{{1, 2}, {3, 4}, {5, 6}}
local btApply = bt:apply(function(val) return val + 3 end)
local apply = tensor.ByteTensor{{4, 5}, {6, 7}, {8, 9}}
assert (btApply == apply)
)";

TEST_F(LuaTensorTest, Apply) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kApply, sizeof(kApply) - 1, "kApply"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr char kApplyIndexed[] = R"(
local tensor = require 'dmlab.system.tensor'
local bt = tensor.ByteTensor{{1, 2}, {3, 4}, {5, 6}}
local btApply = bt:applyIndexed(function(val, index)
  return index[1] * index[2] + val
end)
local apply = tensor.ByteTensor{{2, 4}, {5, 8}, {8, 12}}
assert (btApply == apply)
)";

TEST_F(LuaTensorTest, ApplyIndexed) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kApplyIndexed, sizeof(kApplyIndexed) - 1,
                              "kApplyIndexed"),
              IsOkAndHolds(1));

  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr char kFill[] = R"(
local tensor = require 'dmlab.system.tensor'
local bt = tensor.ByteTensor{{1, 2}, {3, 4}, {5, 6}}
local btApply = bt:fill(10)
local apply = tensor.ByteTensor{{10, 10}, {10, 10}, {10, 10}}
assert (btApply == apply)
)";

TEST_F(LuaTensorTest, Fill) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kFill, sizeof(kFill) - 1, "kFill"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr char kFillTable[] = R"(
local tensor = require 'dmlab.system.tensor'
local bt = tensor.ByteTensor{{1, 2}, {3, 4}, {5, 6}}
local btApply = bt:fill{10, 12}
local apply = tensor.ByteTensor{{10, 12}, {10, 12}, {10, 12}}
assert (btApply == apply)
)";

TEST_F(LuaTensorTest, FillTable) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(
      lua::PushScript(L, kFillTable, sizeof(kFillTable) - 1, "kFillTable"),
      IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr char kVal[] = R"(
local tensor = require 'dmlab.system.tensor'
local bt = tensor.ByteTensor(3, 3)
assert(bt(2, 2):val() == 0)
assert(bt(2, 2):val(10) == 10)
assert (bt == tensor.ByteTensor{{0, 0, 0}, {0, 10, 0}, {0, 0, 0}})
)";

TEST_F(LuaTensorTest, kVal) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kVal, sizeof(kVal) - 1, "kVal"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr char kScalarOp[] = R"(
local tensor = require 'dmlab.system.tensor'
local bt = tensor.ByteTensor{{1, 2}, {3, 4}, {5, 6}}
assert (bt:mul(2) == tensor.ByteTensor{{2, 4}, {6, 8}, {10, 12}})
assert (bt:add(2) == tensor.ByteTensor{{4, 6}, {8, 10}, {12, 14}})
assert (bt:div(2) == tensor.ByteTensor{{2, 3}, {4, 5}, {6, 7}})
assert (bt:sub(2) == tensor.ByteTensor{{0, 1}, {2, 3}, {4, 5}})
)";

TEST_F(LuaTensorTest, kScalarOp) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kScalarOp, sizeof(kScalarOp) - 1, "kScalarOp"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr char kScalarOpTable[] = R"(
local tensor = require 'dmlab.system.tensor'
local bt = tensor.ByteTensor{{1, 2}, {3, 4}, {5, 6}}
assert (bt:mul{2, 4} == tensor.ByteTensor{{2, 8}, {6, 16}, {10, 24}}, "1")
assert (bt:add{2, 4} == tensor.ByteTensor{{4, 12}, {8, 20}, {12, 28}}, "2")
assert (bt:div{2, 4} == tensor.ByteTensor{{2, 3}, {4, 5}, {6, 7}}, "3")
assert (bt:sub{2, 3} == tensor.ByteTensor{{0, 0}, {2, 2}, {4, 4}}, "4")
)";

TEST_F(LuaTensorTest, kScalarOpTable) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kScalarOpTable, sizeof(kScalarOpTable) - 1,
                              "kScalarOpTable"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr char kComponentOp[] = R"(
local tensor = require 'dmlab.system.tensor'
local bt = tensor.ByteTensor{{1, 2}, {3, 4}, {5, 6}}
local bt2 = tensor.ByteTensor{{1, 2, 3}, {4, 5, 6}}
assert (bt:cmul(bt2) == tensor.ByteTensor{{1, 4}, {9, 16}, {25, 36}})
assert (bt:cadd(bt2) == tensor.ByteTensor{{2, 6}, {12, 20}, {30, 42}})
assert (bt:cdiv(bt2) == tensor.ByteTensor{{2, 3}, {4, 5}, {6, 7}})
assert (bt:csub(bt2) == tensor.ByteTensor{{1, 1}, {1, 1}, {1, 1}})
assert (bt:copy(bt2) == tensor.ByteTensor{{1, 2}, {3, 4}, {5, 6}})
)";

TEST_F(LuaTensorTest, kComponentOp) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kComponentOp, sizeof(kComponentOp) - 1,
                              "kComponentOp"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr char kConvertOp[] = R"(
local tensor = require 'dmlab.system.tensor'
local bt = tensor.ByteTensor{{1, 2}, {3, 4}, {5, 6}}
assert (bt:byte() == tensor.ByteTensor{{1, 2}, {3, 4}, {5, 6}})
assert (bt:char() == tensor.CharTensor{{1, 2}, {3, 4}, {5, 6}})
assert (bt:int16() == tensor.Int16Tensor{{1, 2}, {3, 4}, {5, 6}})
assert (bt:int32() == tensor.Int32Tensor{{1, 2}, {3, 4}, {5, 6}})
assert (bt:int64() == tensor.Int64Tensor{{1, 2}, {3, 4}, {5, 6}})
assert (bt:float() == tensor.FloatTensor{{1, 2}, {3, 4}, {5, 6}})
assert (bt:double() == tensor.DoubleTensor{{1, 2}, {3, 4}, {5, 6}})
)";

TEST_F(LuaTensorTest, kConvertOp) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(
      lua::PushScript(L, kConvertOp, sizeof(kConvertOp) - 1, "kConvertOp"),
      IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr char kTestValid[] = R"(
local tensor = require 'dmlab.system.tensor'
local api = {}
function api.readTensor(tensor)
  api._tensorWillBeInvalid = tensor
  api._tensorValid = tensor:clone()
  assert(api._tensorValid:ownsStorage() == true)
  assert(api._tensorWillBeInvalid:ownsStorage() == false)
  assert(api._tensorValid(1, 1):val() == 10)
  assert(api._tensorWillBeInvalid(1, 1):val() == 10)
end

function api.testValid()
  return api._tensorValid(1, 1):val() == 10
end

function api.testInvalid(tensor)
  return api._tensorWillBeInvalid(0, 0):val() == 10
end

return api
)";

TEST_F(LuaTensorTest, Invalidate) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(
      lua::PushScript(L, kTestValid, sizeof(kTestValid) - 1, "kTestValid"),
      IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(1));
  lua::TableRef table_ref;
  lua::Read(L, -1, &table_ref);
  lua_pop(L, 1);
  table_ref.PushFunction("readTensor");
  {
    std::vector<double> doubles(4, 10.0);
    auto shared = std::make_shared<tensor::StorageValidity>();
    tensor::LuaTensor<double>::CreateObject(
        L, tensor::TensorView<double>(tensor::Layout({2, 2}), doubles.data()),
        shared);
    ASSERT_THAT(lua::Call(L, 1), IsOkAndHolds(0));
    shared->Invalidate();
  }
  table_ref.PushFunction("testValid");
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(1));
  bool bool_result;
  lua::Read(L, -1, &bool_result);
  EXPECT_EQ(true, bool_result);
  lua_pop(L, 1);

  table_ref.PushFunction("testInvalid");
  auto result = lua::Call(L, 0);
  EXPECT_FALSE(result.ok());
  EXPECT_THAT(result.error(), HasSubstr("invalid"));
}

constexpr char kTestToString[] = R"(
local tensor = require 'dmlab.system.tensor'
local data = '123'
local empty = ''
return tensor.ByteTensor{data:byte(1, -1)}, tensor.ByteTensor{empty:byte(1, -1)}
)";

TEST_F(LuaTensorTest, kTestToString) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kTestToString, sizeof(kTestToString) - 1,
                              "kTestToString"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(2));
  auto* bt = tensor::LuaTensor<std::uint8_t>::ReadObject(L, -2);
  EXPECT_THAT(std::make_tuple(bt->tensor_view().storage(),
                              bt->tensor_view().num_elements()),
              ElementsAre('1', '2', '3'));

  auto* bt_alt = tensor::LuaTensor<std::uint8_t>::ReadObject(L, -1);
  EXPECT_EQ(0, bt_alt->tensor_view().num_elements());
}

constexpr char kMMulOpLHSNonMatrix[] = R"(
local tensor = require 'dmlab.system.tensor'
local at = tensor.FloatTensor(2, 2, 2)
local bt = tensor.FloatTensor(2, 3)
return at:mmul(bt)
)";

TEST_F(LuaTensorTest, kMMulOpLHSNonMatrix) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(
      lua::PushScript(L, kMMulOpLHSNonMatrix, sizeof(kMMulOpLHSNonMatrix) - 1,
                      "kMMulOpLHSNonMatrix"),
      IsOkAndHolds(1));
  auto result = lua::Call(L, 0);
  EXPECT_FALSE(result.ok());
  EXPECT_THAT(result.error(), HasSubstr("LHS is not a matrix"));
}

constexpr char kMMulOpRHSNonMatrix[] = R"(
local tensor = require 'dmlab.system.tensor'
local at = tensor.FloatTensor(2, 2, 2)
local bt = tensor.FloatTensor(2, 3)
return bt:mmul(at)
)";

TEST_F(LuaTensorTest, kMMulOpRHSNonMatrix) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(
      lua::PushScript(L, kMMulOpRHSNonMatrix, sizeof(kMMulOpRHSNonMatrix) - 1,
                      "kMMulOpRHSNonMatrix"),
      IsOkAndHolds(1));
  auto result = lua::Call(L, 0);
  EXPECT_FALSE(result.ok());
  EXPECT_THAT(result.error(), HasSubstr("RHS is not a matrix"));
}

constexpr char kMMulOpIncompatibleDims[] = R"(
local tensor = require 'dmlab.system.tensor'
local at = tensor.FloatTensor(2, 3)
local bt = tensor.FloatTensor(2, 2)
return at:mmul(bt)
)";

TEST_F(LuaTensorTest, kMMulOpIncompatibleDims) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kMMulOpIncompatibleDims,
                              sizeof(kMMulOpIncompatibleDims) - 1,
                              "kMMulOpIncompatibleDims"),
              IsOkAndHolds(1));
  auto result = lua::Call(L, 0);
  EXPECT_FALSE(result.ok());
  EXPECT_THAT(result.error(), HasSubstr("Incorrect matrix dimensions"));
}

constexpr char kMMulOpIncompatibleType[] = R"(
local tensor = require 'dmlab.system.tensor'
local at = tensor.FloatTensor(2, 2)
local bt = tensor.ByteTensor(2, 2)
return at:mmul(bt)
)";

TEST_F(LuaTensorTest, kMMulOpIncompatibleType) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kMMulOpIncompatibleType,
                              sizeof(kMMulOpIncompatibleType) - 1,
                              "kMMulOpIncompatibleType"),
              IsOkAndHolds(1));
  auto result = lua::Call(L, 0);
  EXPECT_FALSE(result.ok());
  EXPECT_THAT(
      result.error(),
      HasSubstr(
          "Must contain 1 RHS tensor of type deepmind.lab.tensor.FloatTensor"));
}

constexpr char kRoundingOps[] = R"(
local tensor = require 'dmlab.system.tensor'
local bt = tensor.DoubleTensor{{-2.25, -1.75}, {0.5, 1.0}}
assert (bt:clone():floor() == tensor.DoubleTensor{{-3.0, -2.0}, {0.0, 1.0}})
assert (bt:clone():ceil() == tensor.DoubleTensor{{-2.0, -1.0}, {1.0, 1.0}})
assert (bt:clone():round() == tensor.DoubleTensor{{-2.0, -2.0}, {1.0, 1.0}})
)";

TEST_F(LuaTensorTest, kRoundingOps) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kRoundingOps, sizeof(kRoundingOps) - 1,
                              "kRoundingOps"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr char kShuffle[] = R"(
local sys_random = require 'dmlab.system.sys_random'
local tensor = require 'dmlab.system.tensor'

sys_random:seed(123)
local at = tensor.Int64Tensor{range={5}}:shuffle(sys_random)
local bt = tensor.Tensor{0, 0, 0, 0, 0}
for i = 1,5 do
  local j = at(i):val()
  bt(j):val(1)
end
assert (bt == tensor.Tensor{1, 1, 1, 1, 1})
local ct = tensor.Tensor{0}:narrow(1, 1, 0):shuffle(sys_random)
assert (ct:shape()[1] == 0)
)";

TEST_F(LuaTensorTest, kShuffle) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kShuffle, sizeof(kShuffle) - 1, "kShuffle"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr char kReshape[] = R"(
local tensor = require 'dmlab.system.tensor'
local bt = tensor.ByteTensor{{1, 2}, {3, 4}, {5, 6}}
assert(bt:reshape{6} == tensor.ByteTensor{1, 2, 3, 4, 5, 6})
assert(bt:reshape{2, 3} == tensor.ByteTensor{{1, 2, 3}, {4, 5, 6}})
assert(bt:reshape{6, 1} == tensor.ByteTensor{{1}, {2}, {3}, {4}, {5}, {6}})
assert(bt:reshape{1, 6} == tensor.ByteTensor{{1, 2, 3, 4, 5, 6}})
)";

TEST_F(LuaTensorTest, Reshape) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kReshape, sizeof(kReshape) - 1, "kReshape"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr char kValTableRead[] = R"(
local tensor = require 'dmlab.system.tensor'
local bt = tensor.ByteTensor{{1, 2}, {3, 4}, {5, 6}}
local asTable = bt:val()
for i = 1, 3 do
  for j = 1, 2 do
    assert(asTable[i][j] == (i - 1) * 2 + j)
  end
end
)";

TEST_F(LuaTensorTest, kValTableRead) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kValTableRead, sizeof(kValTableRead) - 1,
                              "kValTableRead"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr char kValTableWrite[] = R"(
local tensor = require 'dmlab.system.tensor'
local bt = tensor.ByteTensor{{1, 2}, {3, 4}, {5, 6}}
bt(1):val{1, 1}
bt(2):val{2, 2}
bt(3):val{3, 3}
for i = 1, 3 do
  for j = 1, 2 do
    assert(bt(i, j):val() == i)
  end
end
)";

TEST_F(LuaTensorTest, kValTableWrite) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kValTableWrite, sizeof(kValTableWrite) - 1,
                              "kValTableWrite"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

template <typename T>
void CreateFileWith64IncrementingValues(const std::string& filename) {
  std::ofstream ofs(filename, std::ios::binary);
  for (T val = 0; val < 64; ++val) {
    ofs.write(reinterpret_cast<const char*>(&val), sizeof val);
  }
}

std::string CreateFourRawFiles() {
  std::string temp_dir = testing::TempDir() + "/";
  CreateFileWith64IncrementingValues<unsigned char>(temp_dir + "bytes.bin");
  CreateFileWith64IncrementingValues<double>(temp_dir + "doubles.bin");
  CreateFileWith64IncrementingValues<std::int64_t>(temp_dir + "int64s.bin");
  CreateFileWith64IncrementingValues<float>(temp_dir + "floats.bin");
  return temp_dir;
}

std::string CreateBytesRawFile() {
  std::string temp_dir = testing::TempDir() + "/";
  CreateFileWith64IncrementingValues<unsigned char>(temp_dir + "bytes.bin");
  return temp_dir;
}

constexpr char kLoadWholeFile[] = R"(
local tensor = require 'dmlab.system.tensor'
local path = ...
assert(tensor.ByteTensor{file = {name = path .. 'bytes.bin'}}
       == tensor.ByteTensor{range = {0, 63}})
assert(tensor.DoubleTensor{file = {name = path .. 'doubles.bin'}}
       == tensor.DoubleTensor{range = {0, 63}})
assert(tensor.Int64Tensor{file = {name = path .. 'int64s.bin'}}
       == tensor.Int64Tensor{range = {0, 63}})
assert(tensor.FloatTensor{file = {name = path .. 'floats.bin'}}
       == tensor.FloatTensor{range = {0, 63}})
)";

TEST_F(LuaTensorTest, kLoadWholeFile) {
  lua_State* L = lua_vm_.get();

  ASSERT_THAT(lua::PushScript(L, kLoadWholeFile, sizeof(kLoadWholeFile) - 1,
                              "kLoadWholeFile"),
              IsOkAndHolds(1));
  lua::Push(L, CreateFourRawFiles());
  ASSERT_THAT(lua::Call(L, 1), IsOkAndHolds(0));
}

constexpr char kLoadStartFile[] = R"(
local tensor = require 'dmlab.system.tensor'
local path = ...
assert(tensor.ByteTensor{file =
           {name = path .. 'bytes.bin', numElements = 10}
       } == tensor.ByteTensor{range = {0, 9}})
assert(tensor.DoubleTensor{file =
           {name = path .. 'doubles.bin', numElements = 10}
       } == tensor.DoubleTensor{range = {0, 9}})
assert(tensor.Int64Tensor{file =
           {name = path .. 'int64s.bin', numElements = 10}
       } == tensor.Int64Tensor{range = {0, 9}})
assert(tensor.FloatTensor{file =
           {name = path .. 'floats.bin', numElements = 10}
       } == tensor.FloatTensor{range = {0, 9}})
)";

TEST_F(LuaTensorTest, kLoadStartFile) {
  lua_State* L = lua_vm_.get();

  ASSERT_THAT(lua::PushScript(L, kLoadStartFile, sizeof(kLoadStartFile) - 1,
                              "kLoadStartFile"),
              IsOkAndHolds(1));
  lua::Push(L, CreateFourRawFiles());
  ASSERT_THAT(lua::Call(L, 1), IsOkAndHolds(0));
}

constexpr char kLoadEndFile[] = R"(
local tensor = require 'dmlab.system.tensor'
local path = ...
assert(tensor.ByteTensor{file =
           {name = path .. 'bytes.bin', byteOffset = 40 * 1}
       } == tensor.ByteTensor{range = {40, 63}})
assert(tensor.DoubleTensor{file =
           {name = path .. 'doubles.bin', byteOffset = 40 * 8}
       } == tensor.DoubleTensor{range = {40, 63}})
assert(tensor.Int64Tensor{file =
           {name = path .. 'int64s.bin', byteOffset = 40 * 8}
       } == tensor.Int64Tensor{range = {40, 63}})
assert(tensor.FloatTensor{file =
           {name = path .. 'floats.bin', byteOffset = 40 * 4}
       } == tensor.FloatTensor{range = {40, 63}})
)";

TEST_F(LuaTensorTest, kLoadEndFile) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kLoadEndFile, sizeof(kLoadEndFile) - 1,
                              "kLoadEndFile"),
              IsOkAndHolds(1));
  lua::Push(L, CreateFourRawFiles());
  ASSERT_THAT(lua::Call(L, 1), IsOkAndHolds(0));
}

constexpr char kLoadMiddleFile[] = R"(
local tensor = require 'dmlab.system.tensor'
local path = ...
assert(tensor.ByteTensor{file =
           {name = path .. 'bytes.bin', byteOffset = 40 * 1, numElements = 6}
       } == tensor.ByteTensor{range = {40, 45}})
assert(tensor.DoubleTensor{file =
           {name = path .. 'doubles.bin', byteOffset = 40 * 8, numElements = 6}
       } == tensor.DoubleTensor{range = {40, 45}})
assert(tensor.Int64Tensor{file =
           {name = path .. 'int64s.bin', byteOffset = 40 * 8, numElements = 6}
       } == tensor.Int64Tensor{range = {40, 45}})
assert(tensor.FloatTensor{file =
           {name = path .. 'floats.bin', byteOffset = 40 * 4, numElements = 6}
       } == tensor.FloatTensor{range = {40, 45}})
)";

TEST_F(LuaTensorTest, kLoadMiddleFile) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kLoadMiddleFile, sizeof(kLoadMiddleFile) - 1,
                              "kLoadMiddleFile"),
              IsOkAndHolds(1));
  lua::Push(L, CreateFourRawFiles());
  ASSERT_THAT(lua::Call(L, 1), IsOkAndHolds(0));
}

constexpr char kBadFileName[] = R"(
local tensor = require 'dmlab.system.tensor'
local path = ...
return tensor.ByteTensor{file = {name = path .. 'bad_file.bin'}}
)";

TEST_F(LuaTensorTest, kBadFileName) {
  std::string temp_dir = testing::TempDir() + "/";
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kBadFileName, sizeof(kBadFileName) - 1,
                              "kBadFileName"),
              IsOkAndHolds(1));
  lua::Push(L, CreateBytesRawFile());
  ASSERT_THAT(lua::Call(L, 1), StatusIs(HasSubstr("bad_file.bin")));
}

constexpr char kBadNumElements[] = R"(
local tensor = require 'dmlab.system.tensor'
local path = ...
return tensor.ByteTensor{file = {name = path .. 'bytes.bin', numElements = 65}}
)";

TEST_F(LuaTensorTest, kBadNumElements) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kBadNumElements, sizeof(kBadNumElements) - 1,
                              "kBadNumElements"),
              IsOkAndHolds(1));
  lua::Push(L, CreateBytesRawFile());
  ASSERT_THAT(lua::Call(L, 1),
              StatusIs(AllOf(HasSubstr("numElements"), HasSubstr("65"))));
}

constexpr char kBadNumElementsOffset[] = R"(
local tensor = require 'dmlab.system.tensor'
local path = ...
return tensor.ByteTensor{
    file = {name = path .. 'bytes.bin', byteOffset = 1, numElements = 64}
}
)";

TEST_F(LuaTensorTest, kBadNumElementsOffset) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kBadNumElementsOffset,
                              sizeof(kBadNumElementsOffset) - 1,
                              "kBadNumElementsOffset"),
              IsOkAndHolds(1));
  lua::Push(L, CreateBytesRawFile());
  ASSERT_THAT(lua::Call(L, 1),
              StatusIs(AllOf(HasSubstr("numElements"), HasSubstr("63"))));
}

constexpr char kBadNumElementsNegative[] = R"(
local tensor = require 'dmlab.system.tensor'
local path = ...
return tensor.ByteTensor{file = {name = path .. 'bytes.bin', numElements = -1}}
)";

TEST_F(LuaTensorTest, kBadNumElementsNegative) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kBadNumElementsNegative,
                              sizeof(kBadNumElementsNegative) - 1,
                              "kBadNumElementsNegative"),
              IsOkAndHolds(1));
  lua::Push(L, CreateBytesRawFile());
  ASSERT_THAT(lua::Call(L, 1), StatusIs(HasSubstr("numElements")));
}

constexpr char kBadByteOffset[] = R"(
local tensor = require 'dmlab.system.tensor'
local path = ...
return tensor.ByteTensor{file = {name = path .. 'bytes.bin', byteOffset = 65}}
)";

TEST_F(LuaTensorTest, kBadByteOffset) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kBadByteOffset, sizeof(kBadByteOffset) - 1,
                              "kBadByteOffset"),
              IsOkAndHolds(1));
  lua::Push(L, CreateBytesRawFile());
  ASSERT_THAT(lua::Call(L, 1), StatusIs(HasSubstr("byteOffset")));
}

constexpr char kBadByteOffsetNegative[] = R"(
local tensor = require 'dmlab.system.tensor'
local path = ...
return tensor.ByteTensor{file = {name = path .. 'bytes.bin', byteOffset = -1}}
)";

TEST_F(LuaTensorTest, kBadByteOffsetNegative) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kBadByteOffsetNegative,
                              sizeof(kBadByteOffsetNegative) - 1,
                              "kBadByteOffsetNegative"),
              IsOkAndHolds(1));
  lua::Push(L, CreateBytesRawFile());
  ASSERT_THAT(lua::Call(L, 1), StatusIs(HasSubstr("byteOffset")));
}

constexpr char kNumElements[] = R"(
local tensor = require 'dmlab.system.tensor'
assert(tensor.ByteTensor(4, 5):size() == 20)
)";

TEST_F(LuaTensorTest, kNumElements) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kNumElements,
                              sizeof(kNumElements) - 1,
                              "kNumElements"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr char kClamp[] = R"(
local tensor = require 'dmlab.system.tensor'
local doubles = tensor.DoubleTensor(3, 2)
doubles(1):fill(-500)
doubles(2):fill(25)
doubles(3):fill(500)
local clampBoth = doubles:clone():clamp(0, 255)
assert(clampBoth == tensor.DoubleTensor{{0, 0}, {25, 25}, {255, 255}},
       tostring(clampBoth))
local clampLower = doubles:clone():clamp(0, nil)
assert(clampLower == tensor.DoubleTensor{{0, 0}, {25, 25}, {500, 500}},
       tostring(clampLower))
local clampUpper = doubles:clone():clamp(nil, 255)
assert(clampUpper == tensor.DoubleTensor{{-500, -500}, {25, 25}, {255, 255}},
       tostring(clampUpper))
)";

TEST_F(LuaTensorTest, kClamp) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kClamp, sizeof(kClamp) - 1, "kClamp"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr char kClampErrorMaxMin[] = R"(
local tensor = require 'dmlab.system.tensor'
tensor.DoubleTensor(3, 2):clamp(255, 0)
)";

TEST_F(LuaTensorTest, kClampErrorMaxMin) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kClampErrorMaxMin,
                              sizeof(kClampErrorMaxMin) - 1, "kClampMaxMin"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), StatusIs(HasSubstr("must not exceed")));
}

constexpr char kClampTypeMismatch[] = R"(
local tensor = require 'dmlab.system.tensor'
tensor.ByteTensor(3, 2):clamp(-10, 100)
)";

TEST_F(LuaTensorTest, kClampTypeMismatch) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(
      lua::PushScript(L, kClampTypeMismatch, sizeof(kClampTypeMismatch) - 1,
                      "kClampTypeMismatch"),
      IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), StatusIs(HasSubstr("TypeMismatch")));
}

}  // namespace
}  // namespace lab
}  // namespace deepmind
