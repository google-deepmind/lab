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

#include <array>
#include <string>
#include <utility>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "deepmind/lua/bind.h"
#include "deepmind/lua/call.h"
#include "deepmind/lua/n_results_or_test_util.h"
#include "deepmind/lua/push_script.h"
#include "deepmind/lua/read.h"
#include "deepmind/lua/table_ref.h"
#include "deepmind/lua/vm_test_util.h"

namespace deepmind {
namespace lab {
namespace lua {
namespace {

using ::deepmind::lab::lua::testing::IsOkAndHolds;
using ::deepmind::lab::lua::testing::StatusIs;
using ::testing::AllOf;
using ::testing::HasSubstr;

// Simple demo class to test and demonstrate the functionality of Class.
class Foo final : public Class<Foo> {
 public:
  explicit Foo(std::string name) : name_(std::move(name)) {}

  // Foo(name): returns a new Foo object with the given name.
  static NResultsOr CreateFoo(lua_State* L) {
    std::string name;
    switch (Read(L, 1, &name).Value()) {
      case ReadResult::kFound:
        Class::CreateObject(L, std::move(name));
        return 1;
      case ReadResult::kNotFound:
        return std::string("Missing string arg1 when constructing: ") +
               ClassName();
      case ReadResult::kTypeMismatch:
      default:
        return std::string(
                   "Type missmatch arg1 is not a string when constructing: ") +
               ClassName();
    }
  }

  // Returns whatever was provided as the first argument, twice.
  NResultsOr Duplicate(lua_State* L) {
    lua_pushvalue(L, -1);
    return 2;
  }

  // Returns the name of the object.
  NResultsOr Name(lua_State* L) {
    Push(L, name_);
    return 1;
  }

  // Returns an error message.
  NResultsOr Error(lua_State* L) { return "Something went wrong!"; }

  static void Register(TableRef module, lua_State* L) {
    const Class::Reg methods[] = {
        {"duplicate", Member<&Foo::Duplicate>},  //
        {"name", Member<&Foo::Name>},            //
        {"error", Member<&Foo::Error>},          //
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

  static const char* ClassName() { return "system.Foo"; }

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
  ASSERT_TRUE(IsFound(Read(L, -1, &result)));
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
  ASSERT_TRUE(IsFound(Read(L, -1, &result0)));
  ASSERT_TRUE(IsFound(Read(L, -2, &result1)));
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

constexpr char kScript2[] = R"(
local foo_module = require 'foo_module'
return {
    foo_module.Foo('Hello'),
    foo_module.Foo('Hello2'),
}
)";

TEST_F(ClassTest, TestRead2) {
  vm()->AddCModuleToSearchers("foo_module", Foo::Module);
  ASSERT_THAT(PushScript(L, kScript2, "kScript2"), IsOkAndHolds(1));
  ASSERT_THAT(Call(L, 0), IsOkAndHolds(1));
  std::array<Foo*, 2> results;
  ASSERT_TRUE(IsFound(Read(L, 1, &results)));
  EXPECT_EQ("Hello", results[0]->name());
  EXPECT_EQ("Hello2", results[1]->name());
}

constexpr char kScript[] = R"(
local foo_module = require 'foo_module'
return foo_module.Foo('hello')
)";

TEST_F(ClassTest, TestModule) {
  vm()->AddCModuleToSearchers("foo_module", Foo::Module);
  ASSERT_THAT(PushScript(L, kScript, "kScript"), IsOkAndHolds(1));
  ASSERT_THAT(Call(L, 0), IsOkAndHolds(1));
  Foo* foo = Foo::ReadObject(L, 1);
  ASSERT_TRUE(foo != nullptr);
  EXPECT_EQ("hello", foo->name());
}

constexpr char kScriptMethodError[] = R"(
local foo_module = require 'foo_module'
local foo = foo_module.Foo('Hello')
return foo:error()
)";

TEST_F(ClassTest, MethodErrorMessage) {
  vm()->AddCModuleToSearchers("foo_module", Foo::Module);
  ASSERT_THAT(PushScript(L, kScriptMethodError, "kScriptMethodError"),
              IsOkAndHolds(1));
  ASSERT_THAT(Call(L, 0), StatusIs(AllOf(HasSubstr("system.Foo.error"),
                                         HasSubstr("Something went wrong!"))));
}

constexpr char kScriptCallError[] = R"(
local foo_module = require 'foo_module'
local foo = foo_module.Foo('Hello')
-- Calling with '.'' instead of ':'
return foo.duplicate('World')
)";

TEST_F(ClassTest, CallErrorMessage) {
  vm()->AddCModuleToSearchers("foo_module", Foo::Module);
  ASSERT_THAT(PushScript(L, kScriptCallError, "kScriptCallError"),
              IsOkAndHolds(1));
  ASSERT_THAT(Call(L, 0),
              StatusIs(AllOf(HasSubstr("'system.Foo'"), HasSubstr("'World'"))));
}

}  // namespace
}  // namespace lua
}  // namespace lab
}  // namespace deepmind
