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

#include "deepmind/engine/context_entities.h"

#include "gtest/gtest.h"
#include "absl/types/span.h"
#include "deepmind/lua/bind.h"
#include "deepmind/lua/call.h"
#include "deepmind/lua/n_results_or_test_util.h"
#include "deepmind/lua/push.h"
#include "deepmind/lua/push_script.h"
#include "deepmind/lua/read.h"
#include "deepmind/lua/table_ref.h"
#include "deepmind/lua/vm_test_util.h"

namespace deepmind {
namespace lab {
namespace {

using ::deepmind::lab::lua::testing::IsOkAndHolds;

class ContextEntitiesTest : public lua::testing::TestWithVm {
 protected:
  ContextEntitiesTest() {
    vm()->AddCModuleToSearchers("dmlab.system.game_entities",
                                lua::Bind<ContextEntities::Module>, {&ctx_});
  }

  ContextEntities ctx_;
};

constexpr char kGetEntity[] = R"(
local game_entities = require 'dmlab.system.game_entities'
local index = ...
return game_entities:entities()[index]
)";

TEST_F(ContextEntitiesTest, UpdateEntities) {
  for (int i = 0; i < 10; ++i) {
    int entity_id = i * 10;
    int type = 10;
    int flags = 0x80;
    float position[3] = {i * 1.0f, i * 2.0f, i * 3.0f};
    ctx_.Add(entity_id, 0, type, flags, position, "classname");
  }

  const int index = 4;

  lua::PushScript(L, kGetEntity, sizeof(kGetEntity) - 1, "kGetEntity");
  lua::Push(L, index + 1);
  ASSERT_THAT(lua::Call(L, 1), IsOkAndHolds(1));
  lua::TableRef table;
  ASSERT_TRUE(lua::Read(L, 1, &table));

  int val = 0;
  EXPECT_TRUE(table.LookUp("entityId", &val));
  EXPECT_EQ(val, index * 10 + 1);
  bool visible = true;
  EXPECT_TRUE(table.LookUp("visible", &visible));
  EXPECT_FALSE(visible);
  float new_position[3];
  EXPECT_TRUE(table.LookUp("position", absl::MakeSpan(new_position)));
  float old_position[3] = {index * 1.0f, index * 2.0f, index * 3.0f};
  EXPECT_EQ(absl::MakeConstSpan(old_position),
            absl::MakeConstSpan(new_position));
}

}  // namespace
}  // namespace lab
}  // namespace deepmind
