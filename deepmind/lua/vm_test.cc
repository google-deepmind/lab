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

#include "deepmind/lua/vm.h"

#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "deepmind/lua/call.h"
#include "deepmind/lua/n_results_or.h"
#include "deepmind/lua/n_results_or_test_util.h"
#include "deepmind/lua/push_script.h"
#include "deepmind/lua/read.h"
#include "deepmind/lua/table_ref.h"
#include "deepmind/support/test_srcdir.h"

namespace deepmind {
namespace lab {
namespace lua {
namespace {

using ::deepmind::lab::lua::testing::IsOkAndHolds;

TEST(VmTest, TestVm) {
  std::vector<Vm> vms;

  for (int i = 0; i != 10; ++i) {
    vms.push_back(CreateVm());
  }

  for (std::size_t i = 0; i != vms.size(); ++i) {
    lua_State* L = vms[i].get();
    EXPECT_EQ(0, lua_gettop(L));
    lua_pushstring(L, "Hello");
    lua_pushnumber(L, i);
  }

  for (std::size_t i = 0; i != vms.size(); ++i) {
    lua_State* L = vms[i].get();
    EXPECT_EQ(2, lua_gettop(L));
    EXPECT_STREQ("Hello", lua_tostring(L, 1));
    EXPECT_EQ(static_cast<int>(i), lua_tointeger(L, 2));
  }
}

constexpr char kUseModule[] = R"(
local mod = require 'test.module'
return mod.hello
)";

int CModule(lua_State* L) {
  auto table = TableRef::Create(L);
  table.Insert("hello", 11);
  Push(L, table);
  return 1;
}

TEST(VmTest, TestEmbedC) {
  Vm vm = CreateVm();
  vm.AddCModuleToSearchers("test.module", CModule);
  auto* L = vm.get();

  ASSERT_THAT(PushScript(L, kUseModule, "kUseModule"), IsOkAndHolds(1))
      << "Missing script";

  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(1))
      << "Missing result";

  int val;
  ASSERT_TRUE(IsFound(lua::Read(L, -1, &val)));
  EXPECT_EQ(11, val);
}

int CModuleUpValue(lua_State* L) {
  int* up1 = static_cast<int*>(lua_touserdata(L, lua_upvalueindex(1)));
  int* up2 = static_cast<int*>(lua_touserdata(L, lua_upvalueindex(2)));
  auto table = TableRef::Create(L);
  table.Insert("hello", *up1 / *up2);
  Push(L, table);
  return 1;
}

TEST(VmTest, TestEmbedCClosure) {
  Vm vm = CreateVm();
  int val1 = 55;
  int val2 = 5;
  vm.AddCModuleToSearchers("test.module", CModuleUpValue, {&val1, &val2});

  auto* L = vm.get();
  ASSERT_THAT(PushScript(L, kUseModule, "kUseModule"), IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(1));

  int val;
  ASSERT_TRUE(IsFound(lua::Read(L, -1, &val)));
  EXPECT_EQ(11, val);
}

constexpr char kLuaModule[] = R"(
local mod = { hello = 11 }
return mod
)";

TEST(VmTest, TestEmbedLua) {
  Vm vm = CreateVm();
  vm.AddLuaModuleToSearchers("test.module", kLuaModule, sizeof(kLuaModule) - 1);
  auto* L = vm.get();

  ASSERT_THAT(PushScript(L, kUseModule, "kUseModule"), IsOkAndHolds(1))
      << "Missing script";

  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(1))
      << "Missing result";

  int val;
  ASSERT_TRUE(IsFound(lua::Read(L, -1, &val)));
  EXPECT_EQ(11, val);
}

constexpr char kUsePath[] = R"(
local mod = require 'deepmind.lua.vm_test_module'
return mod.hello
)";

TEST(VmTest, TestLuaPath) {
  Vm vm = CreateVm();
  vm.AddPathToSearchers(TestSrcDir());

  auto* L = vm.get();

  ASSERT_THAT(PushScript(L, kUsePath, "kUsePath"), IsOkAndHolds(1))
      << "Missing script";

  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(1))
      << "Missing result";

  int val;
  ASSERT_TRUE(IsFound(lua::Read(L, -1, &val)));
  EXPECT_EQ(11, val);
}

}  // namespace
}  // namespace lua
}  // namespace lab
}  // namespace deepmind
