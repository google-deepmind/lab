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

#include "deepmind/lua/table_ref.h"

#include <algorithm>
#include <string>
#include <utility>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "deepmind/lua/call.h"
#include "deepmind/lua/class.h"
#include "deepmind/lua/n_results_or_test_util.h"
#include "deepmind/lua/push_script.h"
#include "deepmind/lua/read.h"
#include "deepmind/lua/vm_test_util.h"

namespace deepmind {
namespace lab {
namespace lua {
namespace {

using ::deepmind::lab::lua::testing::IsOkAndHolds;
using ::testing::Contains;
using ::testing::ElementsAre;

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

  static void Register(lua_State* L) {
    const Class::Reg methods[] = {
        {"duplicate", Member<&Foo::Duplicate>},  //
        {"name", Member<&Foo::Name>},            //
    };
    Class::Register(L, methods);
  }
  const std::string& name() { return name_; }

 protected:
  friend class Class;

  static const char* ClassName() { return "deepmind.lab.Foo"; }

 private:
  std::string name_;
};

using TableRefTest = testing::TestWithVm;

TEST_F(TableRefTest, TestPushMemberFunction) {
  TableRef table;

  Foo::Register(L);
  Push(L, "Hello");
  ASSERT_TRUE(IsTypeMismatch(Read(L, -1, &table)));
  ASSERT_NE(L, table.LuaState());

  Foo::CreateFoo(L);

  ASSERT_TRUE(IsFound(Read(L, -1, &table)));
  ASSERT_EQ(L, table.LuaState());

  // Also has keys "__gc" and "__index" but may have more in the future.
  EXPECT_GE(table.KeyCount(), 4);

  table.PushMemberFunction("name");
  Call(L, 2);
  std::string result;
  ASSERT_TRUE(IsFound(Read(L, -1, &result)));
  EXPECT_EQ("Hello", result);
  const auto& keys = table.Keys<std::string>();
  EXPECT_THAT(keys, Contains("name"));
  EXPECT_THAT(keys, Contains("duplicate"));
}

const char kLuaKeyValueTable[] = R"(return {
  key1 = { key1_1 = "Hello!" },
  key2 = { key2_1 = "Good Bye!" },
})";

TEST_F(TableRefTest, TestReadTable) {
  ASSERT_THAT(PushScript(L, kLuaKeyValueTable, "kLuaKeyValueTable"),
              IsOkAndHolds(1));

  ASSERT_THAT(Call(L, 0), IsOkAndHolds(1));

  TableRef table;

  ASSERT_TRUE(IsFound(Read(L, -1, &table)));
  EXPECT_EQ(2, table.KeyCount());

  std::vector<std::string> keys = table.Keys<std::string>();
  std::sort(keys.begin(), keys.end());
  EXPECT_THAT(keys, ElementsAre("key1", "key2"));

  using KV = std::pair<const char*, const char*>;
  for (const auto& p : {KV("key1", "Hello!"), KV("key2", "Good Bye!")}) {
    TableRef subtable;
    std::string result;

    ASSERT_TRUE(table.Contains(p.first));
    ASSERT_TRUE(IsFound(table.LookUp(p.first, &subtable)));
    ASSERT_TRUE(IsFound(subtable.LookUp(std::string(p.first) + "_1", &result)));
    EXPECT_EQ(p.second, result);
  }
}

TEST_F(TableRefTest, TestCreateTable) {
  TableRef table = TableRef::Create(L);
  for (int i = 0; i < 10; ++i) {
    table.Insert(i + 1, i + 1);
  }

  std::vector<int> result;
  Push(L, table);
  ASSERT_TRUE(IsFound(Read(L, -1, &result)));
  lua_pop(L, 1);
  ASSERT_EQ(10, result.size());
  ASSERT_EQ(10, table.ArraySize());

  for (int i = 0; i < 10; ++i) {
    EXPECT_EQ(i + 1, result[i]);
    int out = 0;
    EXPECT_TRUE(IsFound(table.LookUp(i + 1, &out)));
    EXPECT_EQ(i + 1, out);
  }
}

TEST_F(TableRefTest, TestCreateSubTable) {
  TableRef table = TableRef::Create(L);
  for (int i = 0; i < 10; ++i) {
    TableRef subtable = table.CreateSubTable(i + 1);
    subtable.Insert(i + 1, i + 1);
  }

  std::vector<TableRef> result;
  Push(L, table);
  ASSERT_TRUE(IsFound(Read(L, -1, &result)));
  lua_pop(L, 1);
  ASSERT_EQ(10, result.size());

  for (int i = 0; i < 10; ++i) {
    EXPECT_THAT(result[i].Keys<int>(), ElementsAre(i + 1));
    int out = 0;
    EXPECT_TRUE(IsFound(result[i].LookUp(i + 1, &out)));
    EXPECT_EQ(i + 1, out);
  }
}

TEST_F(TableRefTest, TestCopyMoveAssignTable) {
  TableRef table = TableRef::Create(L);
  TableRef table2 = table;
  for (int i = 0; i < 10; ++i) {
    table.Insert(i + 1, i + 1);
  }

  EXPECT_TRUE(table == table2);

  for (int i = 0; i < 10; ++i) {
    table.LookUpToStack(i + 1);
    table2.LookUpToStack(i + 1);
    ASSERT_NE(0, lua_rawequal(L, -1, -2));
    lua_pop(L, 2);
  }

  TableRef table_alt = TableRef::Create(L);
  table = table_alt;
  EXPECT_FALSE(table == table2);

  table2 = std::move(table_alt);
  EXPECT_TRUE(table == table2);
}

TEST_F(TableRefTest, TestCreateInsertFromStack) {
  TableRef table = TableRef::Create(L);
  for (int i = 0; i < 10; ++i) {
    TableRef subtable = TableRef::Create(L);
    subtable.Insert(i + 1, i + 1);
    Push(L, subtable);
    table.InsertFromStackTop(i + 1);
  }

  std::vector<TableRef> result;
  Push(L, table);
  ASSERT_TRUE(IsFound(Read(L, -1, &result)));
  lua_pop(L, 1);
  ASSERT_EQ(10, result.size());

  for (int i = 0; i < 10; ++i) {
    EXPECT_THAT(result[i].Keys<int>(), ElementsAre(i + 1));
    int out = 0;
    EXPECT_TRUE(IsFound(result[i].LookUp(i + 1, &out)));
    EXPECT_EQ(i + 1, out);
  }
}

TEST_F(TableRefTest, TestInsertAndReadSpan) {
  TableRef table = TableRef::Create(L);
  const int data[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  table.Insert("array", absl::MakeConstSpan(data));
  int result[10] = {};
  ASSERT_TRUE(IsFound(table.LookUp("array", absl::MakeSpan(result))));
  EXPECT_EQ(absl::MakeConstSpan(data), absl::MakeConstSpan(result));
}

}  // namespace
}  // namespace lua
}  // namespace lab
}  // namespace deepmind
