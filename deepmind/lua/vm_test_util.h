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
//
// A unit testing test fixture that creates a Lua VM and exposes the
// usual "L" pointer.
//
// Example usage:
//
//    using MyWidgetTest = lua::TestWithVm;
//
//    TEST_F(MyWidgetTest, Frob) {
//      int top = lua_top(L);  // "L" is available
//      // ...
//    }

#ifndef DML_DEEPMIND_LUA_VM_TEST_UTIL_H_
#define DML_DEEPMIND_LUA_VM_TEST_UTIL_H_

#include "absl/log/check.h"
#include "deepmind/lua/vm.h"
#include "gtest/gtest.h"

namespace deepmind {
namespace lab {
namespace lua {
namespace testing {

class TestWithVm : public ::testing::Test {
 private:
  Vm vm_;

 protected:
  TestWithVm() : vm_(Vm::Create()), L(vm_.get()) {
    CHECK_EQ(lua_gettop(L), 0);
  }

  ~TestWithVm() override = default;

  Vm* vm() { return &vm_; }

  lua_State* const L;
};

}  // namespace testing
}  // namespace lua
}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_LUA_VM_TEST_UTIL_H_
