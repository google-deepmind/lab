// Copyright (C) 2018 Google Inc.
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

#include "testing/base/public/benchmark.h"
#include "deepmind/lua/call.h"
#include "deepmind/lua/lua.h"
#include "deepmind/lua/n_results_or_test_util.h"
#include "deepmind/lua/push_script.h"
#include "deepmind/lua/vm.h"
#include "deepmind/util/default_read_only_file_system.h"

namespace deepmind {
namespace lab {
namespace {

void PerformOperation(benchmark::State& state, const char* ctor,
                      const char* operation) {
  using lua::testing::IsOkAndHolds;
  auto lua_vm = lua::CreateVm();
  auto* L = lua_vm.get();
  LuaRandom::Register(L);
  tensor::LuaTensorRegister(L);
  void* default_fs = const_cast<DeepMindReadOnlyFileSystem*>(
      util::DefaultReadOnlyFileSystem());
  lua_vm.AddCModuleToSearchers("dmlab.system.tensor",
                               tensor::LuaTensorConstructors, {default_fs});

  ASSERT_THAT(lua::PushScript(L, ctor, "Construction"), IsOkAndHolds(1));
  ASSERT_EQ(1, lua_gettop(L));

  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(2));
  ASSERT_EQ(2, lua_gettop(L));

  ASSERT_THAT(lua::PushScript(L, operation, "Operation"), IsOkAndHolds(1));

  ASSERT_EQ(3, lua_gettop(L));

  // Lua stack: bt1, bt2, function
  for (auto _ : state) {
    lua_pushvalue(L, -1);
    // Lua stack: bt1, bt2, function, function,
    lua_pushvalue(L, -4);
    // Lua stack: bt1, bt2, function, function, bt1
    lua_pushvalue(L, -4);
    // Lua stack: bt1, bt2, function, function, bt1, bt2
    ASSERT_THAT(lua::Call(L, 2), IsOkAndHolds(0));
    // Lua stack: bt1, bt2, function
  }
  lua_pop(L, 3);
}

// Constructors:

constexpr char kTwoContigTensors[] = R"(
local tensor = require 'dmlab.system.tensor'
return tensor.ByteTensor(1000, 1000, 2), tensor.ByteTensor(1000, 1000, 2)
)";

constexpr char kContigNonContigTensors[] = R"(
local tensor = require 'dmlab.system.tensor'
local contiguous = tensor.ByteTensor(1000, 1000, 2)
local nonContiguous = tensor.ByteTensor(1000, 1000, 3):narrow(3, 1, 2)
return nonContiguous, contiguous
)";

constexpr char kTwoNonContigTensors[] = R"(
local tensor = require 'dmlab.system.tensor'
local contiguous = tensor.ByteTensor(1000, 1000, 3):narrow(3, 1, 2)
local nonContiguous = tensor.ByteTensor(1000, 1000, 3):narrow(3, 1, 2)
return nonContiguous, contiguous
)";

// Operations:

constexpr char kCopyTensors[] = R"(
local bt1, bt2 = ...
bt1:copy(bt2)
)";

constexpr char kCopyTensorsBySlice[] = R"(
local bt1, bt2 = ...
bt1:select(3, 1):copy(bt2:select(3, 1))
bt1:select(3, 2):copy(bt2:select(3, 2))
)";

constexpr char kDoubleTensors[] = R"(
local bt1, bt2 = ...
bt1:mul(2)
bt2:mul(2)
)";

constexpr char kDoubleBySlice[] = R"(
local bt1, bt2 = ...
bt1:select(3,1):mul(2)
bt1:select(3,2):mul(2)
bt2:select(3,1):mul(2)
bt2:select(3,2):mul(2)
)";

constexpr char kFill[] = R"(
local bt1, bt2 = ...
bt1:fill(0)
bt2:fill(0)
)";

constexpr char kFillByTable[] = R"(
local bt1, bt2 = ...
bt1:fill{0, 0}
bt2:fill{0, 0}
)";

constexpr char kFillBySlice[] = R"(
local bt1, bt2 = ...
local fill = {0, 0}
for i = 1, 2 do
  bt1:select(3, i):fill(fill[i])
end
for i = 1, 2 do
  bt2:select(3, i):fill(fill[i])
end
)";

void BM_ContiguousTensorCopy(benchmark::State& state) {
  PerformOperation(state, kTwoContigTensors, kCopyTensors);
}

BENCHMARK(BM_ContiguousTensorCopy);

void BM_ContiguousToNonContiguousCopy(benchmark::State& state) {
  PerformOperation(state, kContigNonContigTensors, kCopyTensors);
}

BENCHMARK(BM_ContiguousToNonContiguousCopy);

void BM_TwoNonContiguousCopy(benchmark::State& state) {
  PerformOperation(state, kTwoNonContigTensors, kCopyTensors);
}

BENCHMARK(BM_TwoNonContiguousCopy);

void BM_TwoNonContiguousCopyBySlice(benchmark::State& state) {
  PerformOperation(state, kTwoNonContigTensors, kCopyTensorsBySlice);
}

BENCHMARK(BM_TwoNonContiguousCopyBySlice);

void BM_ContiguousTensorDouble(benchmark::State& state) {
  PerformOperation(state, kTwoContigTensors, kDoubleTensors);
}

BENCHMARK(BM_ContiguousTensorDouble);

void BM_NonContiguousTensorDouble(benchmark::State& state) {
  PerformOperation(state, kTwoNonContigTensors, kDoubleTensors);
}

BENCHMARK(BM_NonContiguousTensorDouble);

void BM_NonContiguousTensorDoubleBySlice(benchmark::State& state) {
  PerformOperation(state, kTwoNonContigTensors, kDoubleBySlice);
}

BENCHMARK(BM_NonContiguousTensorDoubleBySlice);

void BM_ContiguousFill(benchmark::State& state) {
  PerformOperation(state, kTwoContigTensors, kFill);
}

BENCHMARK(BM_ContiguousFill);

void BM_NonContiguousFill(benchmark::State& state) {
  PerformOperation(state, kTwoNonContigTensors, kFill);
}

BENCHMARK(BM_NonContiguousFill);

void BM_ContiguousFillByTable(benchmark::State& state) {
  PerformOperation(state, kTwoContigTensors, kFillByTable);
}

BENCHMARK(BM_ContiguousFillByTable);

void BM_ContiguousFillBySelect(benchmark::State& state) {
  PerformOperation(state, kTwoContigTensors, kFillBySlice);
}

BENCHMARK(BM_ContiguousFillBySelect);

void BM_NonContiguousFillByTable(benchmark::State& state) {
  PerformOperation(state, kTwoNonContigTensors, kFillByTable);
}

BENCHMARK(BM_NonContiguousFillByTable);

void BM_NonContiguousFillBySelect(benchmark::State& state) {
  PerformOperation(state, kTwoNonContigTensors, kFillBySlice);
}

BENCHMARK(BM_NonContiguousFillBySelect);

}  // namespace
}  // namespace lab
}  // namespace deepmind
