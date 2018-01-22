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

#include "deepmind/level_generation/text_level/lua_bindings.h"

#include <functional>
#include <random>
#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "deepmind/level_generation/text_level/text_level_settings.h"
#include "deepmind/level_generation/text_level/translate_text_level.h"
#include "deepmind/lua/call.h"
#include "deepmind/lua/push_script.h"
#include "deepmind/lua/vm.h"

namespace deepmind {
namespace lab {
namespace {

using testing::HasSubstr;
using testing::Not;

constexpr char kLuaFn[] = R"(
i, j, ent, emitter = ...

if ent == "x" then
  return {
      emitter:makeEntity{
          i = i,
          j = j,
          classname = "xyzzy",
          attributes = {a = "goodattr"}
      },
      emitter:makeEntity{
          i = i,
          j = j,
          classname = "xyzzy",
          attributes = {a = "badattr", b = 10}
      }
  }
end

if ent == "y" then
  return emitter:makeDoor{
      i = i,
      j = j,
      isEastWest = true
  }
end

if ent == "z" then
  return emitter:makeSpawnPoint{
      i = i,
      j = j,
      angleRad = 1.25
  }
end

if ent == "w" then
  return {"x", "y", "z"}
end

-- Erroneous function calls.
if ent == "a" then
  return {"poison", emitter:makeEntity()}
end
if ent == "b" then
  return {"poison", emitter:makeDoor()}
end
if ent == "c" then
  return {"poison", emitter:makeSpawnPoint()}
end
)";

TEST(LuaBindings, Simple) {
  auto vm = lua::CreateVm();
  lua_State* L = vm.get();
  std::mt19937_64 rng(123);

  LuaSnippetEmitter::Register(L);

  {
    auto res = lua::PushScript(L, std::string(kLuaFn), "lua_test_function");
    CHECK(res.ok());
    CHECK_EQ(1, res.n_results());
  }

  ASSERT_TRUE(lua_isfunction(L, -1)) << "Actual type: " << lua_type(L, -1);

  using namespace std::placeholders;
  auto cb = std::bind(LuaCustomEntityCallback, L, -1, _1, _2, _3, _4, _5);

  TextLevelSettings settings;
  std::string s = TranslateTextLevel("* xyzw *\n", "", &rng, cb, &settings);

  // From 'x':
  EXPECT_THAT(s, HasSubstr("\"classname\" \"xyzzy\""));
  EXPECT_THAT(s, HasSubstr("\"goodattr\""));
  EXPECT_THAT(s, Not(HasSubstr("\"badattr\"")));

  // From 'y':
  EXPECT_THAT(s, HasSubstr("\"classname\" \"func_door\""));

  // From 'z':
  EXPECT_THAT(s, HasSubstr("\"classname\" \"info_player_start\""));

  // From 'w':
  EXPECT_THAT(s, HasSubstr("x\n\ny\n\nz\n\n"));
}

TEST(LuaBindings, CallbackErrorIsFatal) {
  auto vm = lua::CreateVm();
  lua_State* L = vm.get();
  std::mt19937_64 rng(123);

  LuaSnippetEmitter::Register(L);

  {
    auto res = lua::PushScript(L, std::string(kLuaFn), "lua_test_function");
    CHECK(res.ok());
    CHECK_EQ(1, res.n_results());
  }
  using namespace std::placeholders;
  auto cb = std::bind(LuaCustomEntityCallback, L, -1, _1, _2, _3, _4, _5);

  TextLevelSettings settings;
  EXPECT_DEATH(TranslateTextLevel("* abc *\n", "", &rng, cb, &settings),
               "User callback invocation failed");
}

}  // namespace
}  // namespace lab
}  // namespace deepmind
