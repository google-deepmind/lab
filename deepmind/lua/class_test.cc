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

#include "deepmind/lua/class.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "deepmind/lua/bind.h"
#include "deepmind/lua/call.h"
#include "deepmind/lua/push_script.h"
#include "deepmind/lua/read.h"
#include "deepmind/lua/table_ref.h"
#include "deepmind/lua/vm_test_util.h"

namespace deepmind {
namespace lab {
namespace lua {
namespace {

// Simple demo class to test and demonstrate the functionality of Class.
class Foo final : public Class<Foo> {
 public:
  explicit Foo(std::string name) : name_(std::move(name)) {}

  // Foo(name): returns a new Foo object with the given name.
  static NResultsOr CreateFoo(lua_State* L) {
    std::string name;
    if (Read(L, 1, &name)) {
      Class::CreateObject(L, std::move(name));
      return 1;
    }
    return std::string("Missing string arg1 when constructing: ") + ClassName();
  }

  // Returns whatever was provided as the first argument, twice.
  //
  // [-1, +2, -]
  NResultsOr Duplicate(lua_State* L) {
    lua_pushvalue(L, -1);
    return 2;
  }

  // Returns the name of the object.
  //
  // [-0, +1, -]
  NResultsOr Name(lua_State* L) {
    Push(L, name_);
    return 1;
  }

  static void Register(TableRef module, lua_State* L) {
    const Class::Reg methods[] = {
        {"duplicate", Member<&Foo::Duplicate>},  //
        {"name", Member<&Foo::Name>},            //
    };
    Class::Register(L, methods);
    lua_CFunction f = Bind<Foo::CreateFoo>;
    module.Insert("Foo", f);
  }

  static int Module(lua_State* L) {
    TableRef module = TableRef::Create(L);
    Register(module, L);
    Push(L, module);
    return 1;
  }

  const std::string& name() const { return name_; }

 protected:
  friend class Class;

  static const char* ClassName() { return "deepmind.lab.Foo"; }

 private:
  std::string name_;
};

using ClassTest = testing::TestWithVm;

TEST_F(ClassTest, TestName) {
  TableRef module = TableRef::Create(L);
  Foo::Register(module, L);
  Push(L, "Hello");
  Foo::CreateFoo(L);
  lua_getfield(L, -1, "name");
  lua_pushvalue(L, -2);
  int error = lua_pcall(L, 1, 1, 0);
  ASSERT_EQ(error, 0) << "Reason - " << lua_tostring(L, -1);
  std::string result;
  ASSERT_TRUE(Read(L, -1, &result));
  EXPECT_EQ("Hello", result);
}

TEST_F(ClassTest, TestDuplicate) {
  TableRef module = TableRef::Create(L);
  Foo::Register(module, L);
  Push(L, "Hello");
  Foo::CreateFoo(L);
  lua_getfield(L, -1, "duplicate");
  lua_pushvalue(L, -2);
  Push(L, "There");
  int error = lua_pcall(L, 2, 2, 0);
  ASSERT_EQ(error, 0) << "Reason - " << lua_tostring(L, -1);
  std::string result0;
  std::string result1;
  ASSERT_TRUE(Read(L, -1, &result0));
  ASSERT_TRUE(Read(L, -2, &result1));
  EXPECT_EQ(result0, result1);
}

TEST_F(ClassTest, TestRead) {
  TableRef module = TableRef::Create(L);
  Foo::Register(module, L);
  Push(L, "Hello");
  Foo::CreateFoo(L);
  Foo* foo = Foo::ReadObject(L, -1);
  ASSERT_TRUE(foo != nullptr);
  EXPECT_EQ("Hello", foo->name());
}

constexpr char kScript[] = R"(
local foo_module = require 'foo_module'
return foo_module.Foo('hello')
)";

TEST_F(ClassTest, TestModule) {
  vm()->AddCModuleToSearchers("foo_module", Foo::Module);
  auto result = PushScript(L, kScript, sizeof(kScript) - 1, "kScript");
  ASSERT_TRUE(result.ok()) << result.error();
  result = Call(L, 0);
  ASSERT_TRUE(result.ok()) << result.error();
  Foo* foo = Foo::ReadObject(L, -1);
  ASSERT_TRUE(foo != nullptr);
  EXPECT_EQ("hello", foo->name());
}

}  // namespace
}  // namespace lua
}  // namespace lab
}  // namespace deepmind
