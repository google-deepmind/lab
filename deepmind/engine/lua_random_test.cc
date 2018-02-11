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

#include "deepmind/engine/lua_random.h"

#include <random>
#include <sstream>
#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "deepmind/lua/bind.h"
#include "deepmind/lua/call.h"
#include "deepmind/lua/push_script.h"
#include "deepmind/lua/n_results_or_test_util.h"
#include "deepmind/lua/vm_test_util.h"

namespace deepmind {
namespace lab {

using ::deepmind::lab::lua::testing::IsOkAndHolds;

class LuaRandomTest : public lua::testing::TestWithVm {
 protected:
  LuaRandomTest() {
    LuaRandom::Register(L);
    vm()->AddCModuleToSearchers(
        "dmlab.system.sys_random", &lua::Bind<LuaRandom::Require>,
        {&prbg_, reinterpret_cast<void*>(static_cast<std::uintptr_t>(0))});
  }

  std::mt19937_64 prbg_;
};

TEST_F(LuaRandomTest, SeedWithNumber) {
  std::ostringstream expected_state, actual_state;

  prbg_.seed(999);
  expected_state << prbg_;

  prbg_.seed(0);

  const char kSeed[] = R"(
    local sys_random = require 'dmlab.system.sys_random'
    sys_random:seed(999)
  )";

  ASSERT_THAT(lua::PushScript(L, kSeed, "kSeed"), IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
  actual_state << prbg_;

  EXPECT_EQ(expected_state.str(), actual_state.str());
}

TEST_F(LuaRandomTest, SeedWithString) {
  std::ostringstream expected_state, actual_state;

  prbg_.seed(888);
  expected_state << prbg_;

  prbg_.seed(0);

  const char kSeed[] = R"(
    local sys_random = require 'dmlab.system.sys_random'
    sys_random:seed("888")
  )";

  ASSERT_THAT(lua::PushScript(L, kSeed, "kSeed"), IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
  actual_state << prbg_;

  EXPECT_EQ(expected_state.str(), actual_state.str());
}

}  // namespace lab
}  // namespace deepmind
