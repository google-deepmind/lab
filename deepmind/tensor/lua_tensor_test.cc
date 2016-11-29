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
#include <tuple>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "deepmind/lua/call.h"
#include "deepmind/lua/n_results_or_test_util.h"
#include "deepmind/lua/push_script.h"
#include "deepmind/lua/read.h"
#include "deepmind/lua/table_ref.h"
#include "deepmind/lua/vm.h"

namespace deepmind {
namespace lab {
namespace {

using ::deepmind::lab::lua::testing::IsOkAndHolds;
using ::testing::HasSubstr;
using ::testing::ElementsAre;

class LuaTensorTest : public ::testing::Test {
 protected:
  LuaTensorTest() : lua_vm_(lua::CreateVm()) {
    tensor::LuaTensorRegister(lua_vm_.get());
    lua_vm_.AddCModuleToSearchers("dmlab.system.tensor",
                                  tensor::LuaTensorConstructors);
  }
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

constexpr char kApply[] = R"(
local tensor = require 'dmlab.system.tensor'
local bt = tensor.ByteTensor{{1, 2}, {3, 4}, {5, 6}}
local bt_apply = bt:apply(function(val) return val + 3 end)
local apply = tensor.ByteTensor{{4, 5}, {6, 7}, {8, 9}}
assert (bt_apply == apply)
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
local bt_apply = bt:applyIndexed(function(val, index)
  return index[1] * index[2] + val
end)
local apply = tensor.ByteTensor{{2, 4}, {5, 8}, {8, 12}}
assert (bt_apply == apply)
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
local bt_apply = bt:fill(10)
local apply = tensor.ByteTensor{{10, 10}, {10, 10}, {10, 10}}
assert (bt_apply == apply)
)";

TEST_F(LuaTensorTest, Fill) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kFill, sizeof(kFill) - 1, "kFill"),
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

}  // namespace
}  // namespace lab
}  // namespace deepmind
