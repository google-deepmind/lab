// Copyright (C) 2017 Google Inc.
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

#include "deepmind/engine/lua_image.h"

#include <algorithm>
#include <random>
#include <string>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "deepmind/lua/call.h"
#include "deepmind/lua/n_results_or_test_util.h"
#include "deepmind/lua/push_script.h"
#include "deepmind/lua/vm_test_util.h"
#include "deepmind/tensor/lua_tensor.h"
#include "deepmind/util/default_read_only_file_system.h"

namespace deepmind {
namespace lab {

using ::deepmind::lab::lua::testing::IsOkAndHolds;
using ::deepmind::lab::lua::testing::StatusIs;
using ::testing::HasSubstr;

class LuaImageTest : public ::testing::Test {
 protected:
  LuaImageTest() : lua_vm_(lua::CreateVm()) {
    auto* L = lua_vm_.get();
    void* default_fs = const_cast<DeepMindReadOnlyFileSystem*>(
        util::DefaultReadOnlyFileSystem());
    lua_vm_.AddCModuleToSearchers("dmlab.system.image", LuaImageRequire,
                                  {default_fs});
    tensor::LuaTensorRegister(L);
    lua_vm_.AddCModuleToSearchers("dmlab.system.tensor",
                                  tensor::LuaTensorConstructors);
  }
  lua::Vm lua_vm_;
};

constexpr char kLuaImageLinearScaleXMagYMag[] = R"(
local image = require 'dmlab.system.image'
local tensor = require 'dmlab.system.tensor'

local src = tensor.ByteTensor{
  {{255, 0, 0}, {0, 255, 0}},
  {{0, 0, 255}, {255, 255, 255}}
}
local tgt = image.scale(src, 4, 4)
assert(tgt == tensor.ByteTensor{
  {{255, 0, 0}, {170, 85, 0}, {85, 170, 0}, {0, 255, 0}},
  {{170, 0, 85}, {141, 85, 85}, {113, 170, 85}, {85, 255, 85}},
  {{85, 0, 170}, {113, 85, 170}, {141, 170, 170}, {170, 255, 170}},
  {{0, 0, 255}, {85, 85, 255}, {170, 170, 255}, {255, 255, 255}}
})
)";

TEST_F(LuaImageTest, LinearScaleXMagYMag) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kLuaImageLinearScaleXMagYMag,
                              sizeof(kLuaImageLinearScaleXMagYMag) - 1,
                              "kLuaImageLinearScaleXMagYMag"),
              IsOkAndHolds(1));
  EXPECT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr char kLuaImageLinearScaleXMaxYMin[] = R"(
local image = require 'dmlab.system.image'
local tensor = require 'dmlab.system.tensor'

local src = tensor.ByteTensor{
  {{255, 0, 0}, {170, 85, 0}, {85, 170, 0}, {0, 255, 0}},
  {{170, 0, 85}, {141, 85, 85}, {113, 170, 85}, {85, 255, 85}},
  {{85, 0, 170}, {113, 85, 170}, {141, 170, 170}, {170, 255, 170}},
  {{0, 0, 255}, {85, 85, 255}, {170, 170, 255}, {255, 255, 255}}
}
local tgt = image.scale(src, 2, 6)
assert(tgt == tensor.ByteTensor{
  {{212, 0, 42}, {184, 42, 42}, {155, 85, 42}, {99, 170, 42}, {70, 212, 42}, {42, 255, 42}},
  {{42, 0, 212}, {70, 42, 212}, {99, 85, 212}, {155, 170, 212}, {184, 212, 212}, {212, 255, 212}}
})
)";

TEST_F(LuaImageTest, LinearScaleXMaxYMin) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kLuaImageLinearScaleXMaxYMin,
                              sizeof(kLuaImageLinearScaleXMaxYMin) - 1,
                              "kLuaImageLinearScaleXMaxYMin"),
              IsOkAndHolds(1));
  EXPECT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr char kLuaImageLinearScaleXMinYMax[] = R"(
local image = require 'dmlab.system.image'
local tensor = require 'dmlab.system.tensor'

local src = tensor.ByteTensor{
  {{255, 0, 0}, {170, 85, 0}, {85, 170, 0}, {0, 255, 0}},
  {{170, 0, 85}, {141, 85, 85}, {113, 170, 85}, {85, 255, 85}},
  {{85, 0, 170}, {113, 85, 170}, {141, 170, 170}, {170, 255, 170}},
  {{0, 0, 255}, {85, 85, 255}, {170, 170, 255}, {255, 255, 255}}
}
local tgt = image.scale(src, 6, 2)
assert(tgt == tensor.ByteTensor{
  {{212, 42, 0}, {42, 212, 0}},
  {{184, 42, 42}, {70, 212, 42}},
  {{155, 42, 85}, {99, 212, 85}},
  {{99, 42, 170}, {155, 212, 170}},
  {{70, 42, 212}, {184, 212, 212}},
  {{42, 42, 255}, {212, 212, 255}}
})
)";

TEST_F(LuaImageTest, LinearScaleXMinYMax) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kLuaImageLinearScaleXMinYMax,
                              sizeof(kLuaImageLinearScaleXMinYMax) - 1,
                              "kLuaImageLinearScaleXMinYMax"),
              IsOkAndHolds(1));
  EXPECT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr char kLuaImageLinearScaleXMinYMin[] = R"(
local image = require 'dmlab.system.image'
local tensor = require 'dmlab.system.tensor'

local src = tensor.ByteTensor{
  {{255, 0, 0}, {170, 85, 0}, {85, 170, 0}, {0, 255, 0}},
  {{170, 0, 85}, {141, 85, 85}, {113, 170, 85}, {85, 255, 85}},
  {{85, 0, 170}, {113, 85, 170}, {141, 170, 170}, {170, 255, 170}},
  {{0, 0, 255}, {85, 85, 255}, {170, 170, 255}, {255, 255, 255}}
}
local tgt = image.scale(src, 2, 3)
assert(tgt == tensor.ByteTensor{
  {{198,  21, 42}, {127, 127, 42}, {56, 233, 42}},
  {{56, 21, 212}, {127, 127, 212}, {198, 233, 212}}
})
)";

TEST_F(LuaImageTest, LinearScaleXMinYMin) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kLuaImageLinearScaleXMinYMin,
                              sizeof(kLuaImageLinearScaleXMinYMin) - 1,
                              "kLuaImageLinearScaleXMinYMin"),
              IsOkAndHolds(1));
  EXPECT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr char kLuaImageLinearScaleLineXMaxYMax[] = R"(
local image = require 'dmlab.system.image'
local tensor = require 'dmlab.system.tensor'

local src = tensor.ByteTensor{
  {{255, 0, 0}, {0, 255, 0}},
}
local tgt = image.scale(src, 4, 3)
assert(tgt == tensor.ByteTensor{
  {{255, 0, 0}, {127, 127, 0}, {0, 255, 0}},
  {{255, 0, 0}, {127, 127, 0}, {0, 255, 0}},
  {{255, 0, 0}, {127, 127, 0}, {0, 255, 0}},
  {{255, 0, 0}, {127, 127, 0}, {0, 255, 0}}
})
)";

TEST_F(LuaImageTest, LinearScaleLineXMaxYMax) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kLuaImageLinearScaleLineXMaxYMax,
                              sizeof(kLuaImageLinearScaleLineXMaxYMax) - 1,
                              "kLuaImageLinearScaleLineXMaxYMax"),
              IsOkAndHolds(1));
  EXPECT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr char kLuaImageLinearScaleEmpty[] = R"(
local image = require 'dmlab.system.image'
local tensor = require 'dmlab.system.tensor'

local src = tensor.ByteTensor(4, 4, 3):narrow(1, 1, 0)
local tgt = image.scale(src, 4, 3)
assert(tgt == nil)
)";

TEST_F(LuaImageTest, LinearScaleEmpty) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kLuaImageLinearScaleEmpty,
                              sizeof(kLuaImageLinearScaleEmpty) - 1,
                              "kLuaImageLinearScaleEmpty"),
              IsOkAndHolds(1));
  EXPECT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr char kLuaImageLinearScaleZeroRows[] = R"(
local image = require 'dmlab.system.image'
local tensor = require 'dmlab.system.tensor'

local src = tensor.ByteTensor{
  {{255, 0, 0}, {0, 255, 0}},
  {{0, 0, 255}, {255, 255, 255}}
}
local tgt = image.scale(src, 0, 3)
assert(tgt == nil)
)";

TEST_F(LuaImageTest, LinearScaleZeroRows) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kLuaImageLinearScaleZeroRows,
                              sizeof(kLuaImageLinearScaleZeroRows) - 1,
                              "kLuaImageLinearScaleZeroRows"),
              IsOkAndHolds(1));
  EXPECT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr char kLuaImageLinearScaleZeroCols[] = R"(
local image = require 'dmlab.system.image'
local tensor = require 'dmlab.system.tensor'

local src = tensor.ByteTensor{
  {{255, 0, 0}, {0, 255, 0}},
  {{0, 0, 255}, {255, 255, 255}}
}
local tgt = image.scale(src, 1, 0)
assert(tgt == nil)
)";

TEST_F(LuaImageTest, LinearScaleZeroCols) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kLuaImageLinearScaleZeroCols,
                              sizeof(kLuaImageLinearScaleZeroCols) - 1,
                              "kLuaImageLinearScaleZeroCols"),
              IsOkAndHolds(1));
  EXPECT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr char kLuaImageNearestScaleXMagYMag[] = R"(
local image = require 'dmlab.system.image'
local tensor = require 'dmlab.system.tensor'

local src = tensor.ByteTensor{
  {{255, 0, 0}, {0, 255, 0}},
  {{0, 0, 255}, {255, 255, 255}}
}
local tgt = image.scale(src, 4, 4, 'nearest')
assert(tgt == tensor.ByteTensor{
  {{255, 0, 0}, {255, 0, 0}, {0, 255, 0}, {0, 255, 0}},
  {{255, 0, 0}, {255, 0, 0}, {0, 255, 0}, {0, 255, 0}},
  {{0, 0, 255}, {0, 0, 255}, {255, 255, 255}, {255, 255, 255}},
  {{0, 0, 255}, {0, 0, 255}, {255, 255, 255}, {255, 255, 255}}
})
)";

TEST_F(LuaImageTest, NearestScaleXMagYMag) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kLuaImageNearestScaleXMagYMag,
                              sizeof(kLuaImageNearestScaleXMagYMag) - 1,
                              "kLuaImageNearestScaleXMagYMag"),
              IsOkAndHolds(1));
  EXPECT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr char kLuaImageNearestScaleXMaxYMin[] = R"(
local image = require 'dmlab.system.image'
local tensor = require 'dmlab.system.tensor'

local src = tensor.ByteTensor{
  {{255, 0, 0}, {170, 85, 0}, {85, 170, 0}, {0, 255, 0}},
  {{170, 0, 85}, {141, 85, 85}, {113, 170, 85}, {85, 255, 85}},
  {{85, 0, 170}, {113, 85, 170}, {141, 170, 170}, {170, 255, 170}},
  {{0, 0, 255}, {85, 85, 255}, {170, 170, 255}, {255, 255, 255}}
}
local tgt = image.scale(src, 2, 6, 'nearest')
assert(tgt == tensor.ByteTensor{
  {{212, 0, 42}, {212, 0, 42}, {155, 85, 42}, {99, 170, 42}, {99, 170, 42}, {42, 255, 42}},
  {{42, 0, 212}, {42, 0, 212}, {99, 85, 212}, {155, 170, 212}, {155, 170, 212}, {212, 255, 212}}
})
)";

TEST_F(LuaImageTest, NearestScaleXMaxYMin) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kLuaImageNearestScaleXMaxYMin,
                              sizeof(kLuaImageNearestScaleXMaxYMin) - 1,
                              "kLuaImageNearestScaleXMaxYMin"),
              IsOkAndHolds(1));
  EXPECT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr char kLuaImageNearestScaleXMinYMax[] = R"(
local image = require 'dmlab.system.image'
local tensor = require 'dmlab.system.tensor'

local src = tensor.ByteTensor{
  {{255, 0, 0}, {170, 85, 0}, {85, 170, 0}, {0, 255, 0}},
  {{170, 0, 85}, {141, 85, 85}, {113, 170, 85}, {85, 255, 85}},
  {{85, 0, 170}, {113, 85, 170}, {141, 170, 170}, {170, 255, 170}},
  {{0, 0, 255}, {85, 85, 255}, {170, 170, 255}, {255, 255, 255}}
}
local tgt = image.scale(src, 6, 2, 'nearest')
assert(tgt == tensor.ByteTensor{
  {{212, 42, 0}, {42, 212, 0}},
  {{212, 42, 0}, {42, 212, 0}},
  {{155, 42, 85}, {99, 212, 85}},
  {{99, 42, 170}, {155, 212, 170}},
  {{99, 42, 170}, {155, 212, 170}},
  {{42, 42, 255}, {212, 212, 255}}
})
)";

TEST_F(LuaImageTest, NearestScaleXMinYMax) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kLuaImageNearestScaleXMinYMax,
                              sizeof(kLuaImageNearestScaleXMinYMax) - 1,
                              "kLuaImageNearestScaleXMinYMax"),
              IsOkAndHolds(1));
  EXPECT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr char kLuaImageNearestScaleXMinYMin[] = R"(
local image = require 'dmlab.system.image'
local tensor = require 'dmlab.system.tensor'

local src = tensor.ByteTensor{
  {{255, 0, 0}, {170, 85, 0}, {85, 170, 0}, {0, 255, 0}},
  {{170, 0, 85}, {141, 85, 85}, {113, 170, 85}, {85, 255, 85}},
  {{85, 0, 170}, {113, 85, 170}, {141, 170, 170}, {170, 255, 170}},
  {{0, 0, 255}, {85, 85, 255}, {170, 170, 255}, {255, 255, 255}}
}
local tgt = image.scale(src, 2, 3, 'nearest')
assert(tgt == tensor.ByteTensor{
  {{198,  21, 42}, {127, 127, 42}, {56, 233, 42}},
  {{56, 21, 212}, {127, 127, 212}, {198, 233, 212}}
})
)";

TEST_F(LuaImageTest, NearestScaleXMinYMin) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kLuaImageNearestScaleXMinYMin,
                              sizeof(kLuaImageNearestScaleXMinYMin) - 1,
                              "kLuaImageNearestScaleXMinYMin"),
              IsOkAndHolds(1));
  EXPECT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr char kLuaImageNearestScaleLineXMaxYMax[] = R"(
local image = require 'dmlab.system.image'
local tensor = require 'dmlab.system.tensor'

local src = tensor.ByteTensor{
  {{255, 0, 0}, {0, 255, 0}},
}
local tgt = image.scale(src, 4, 3, 'nearest')
assert(tgt == tensor.ByteTensor{
  {{255, 0, 0}, {255, 0, 0}, {0, 255, 0}},
  {{255, 0, 0}, {255, 0, 0}, {0, 255, 0}},
  {{255, 0, 0}, {255, 0, 0}, {0, 255, 0}},
  {{255, 0, 0}, {255, 0, 0}, {0, 255, 0}}
})
)";

TEST_F(LuaImageTest, NearestScaleLineXMaxYMax) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kLuaImageNearestScaleLineXMaxYMax,
                              sizeof(kLuaImageNearestScaleLineXMaxYMax) - 1,
                              "kLuaImageNearestScaleLineXMaxYMax"),
              IsOkAndHolds(1));
  EXPECT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr char kLuaImageScaleRandom[] = R"(
local image = require 'dmlab.system.image'
local tensor = require 'dmlab.system.tensor'
local s, t = ...
local src = tensor.ByteTensor(s, 1, 3)
local tgt = image.scale(src, t, 1, 'nearest')
assert(tgt == tensor.ByteTensor(t, 1, 3))
)";

TEST_F(LuaImageTest, kLuaImageScaleRandom) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(
      lua::PushScript(L, kLuaImageScaleRandom, sizeof(kLuaImageScaleRandom) - 1,
                      "kLuaImageScaleRandom"),
      IsOkAndHolds(1));

  std::mt19937 gen(1234);
  std::uniform_int_distribution<int> dist(1, 256);
  for (int i = 0; i < 1000; ++i) {
    lua_pushvalue(L, -1);
    lua::Push(L, dist(gen));
    lua::Push(L, dist(gen));
    EXPECT_THAT(lua::Call(L, 2), IsOkAndHolds(0));
  }
  lua_pop(L, 1);
}

constexpr char kLuaImageScaleDown1Pix[] = R"(
local image = require 'dmlab.system.image'
local tensor = require 'dmlab.system.tensor'
local t = ...
local src = tensor.ByteTensor(t + 1, 1, 3)
local tgt = image.scale(src, t, 1, 'nearest')
assert(tgt == tensor.ByteTensor(t, 1, 3))
)";

TEST_F(LuaImageTest, kLuaImageScaleDown1Pix) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kLuaImageScaleDown1Pix,
                              sizeof(kLuaImageScaleDown1Pix) - 1,
                              "kLuaImageScaleDown1Pix"),
              IsOkAndHolds(1));
  for (int i : {6, 20, 24, 6388, 6758}) {
    lua_pushvalue(L, -1);
    lua::Push(L, i);
    EXPECT_THAT(lua::Call(L, 1), IsOkAndHolds(0))
        << "Size: " << i + 1 << " " << i;
  }
  lua_pop(L, 1);
}

constexpr char kLuaSetHueRedToGreen[] = R"(
local image = require 'dmlab.system.image'
local tensor = require 'dmlab.system.tensor'
local src = tensor.ByteTensor{
    {255, 0, 0},
    {127, 0, 0},
    {255, 127, 127},
    {127, 64, 64},
    {255, 255, 255},
    {0, 0, 0},
}
local greenHue = 120
image.setHue(src, greenHue)
assert(src == tensor.ByteTensor{
    {0, 255, 0},
    {0, 127, 0},
    {127, 255, 127},
    {64, 127, 64},
    {255, 255, 255},
    {0, 0, 0},
}, tostring(src))
)";

TEST_F(LuaImageTest, kLuaSetHueRedToGreen) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(
      lua::PushScript(L, kLuaSetHueRedToGreen, sizeof(kLuaSetHueRedToGreen) - 1,
                      "kLuaSetHueRedToGreen"),
      IsOkAndHolds(1));
  EXPECT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr char kLuaSetHueRedToYellow[] = R"(
local image = require 'dmlab.system.image'
local tensor = require 'dmlab.system.tensor'
local src = tensor.ByteTensor{
    {255, 0, 0},
    {127, 0, 0},
    {255, 127, 127},
    {127, 64, 64},
    {255, 255, 255},
    {0, 0, 0},
}
local yellowHue = 60
image.setHue(src, yellowHue)
assert(src == tensor.ByteTensor{
    {255, 255, 0},
    {127, 127, 0},
    {255, 255, 127},
    {127, 127, 64},
    {255, 255, 255},
    {0, 0, 0},
}, tostring(src))
)";

TEST_F(LuaImageTest, kLuaSetHueRedToYellow) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kLuaSetHueRedToYellow,
                              sizeof(kLuaSetHueRedToYellow) - 1,
                              "kLuaSetHueRedToYellow"),
              IsOkAndHolds(1));
  EXPECT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr char kLuaSetHueRedToCyan[] = R"(
local image = require 'dmlab.system.image'
local tensor = require 'dmlab.system.tensor'
local src = tensor.ByteTensor{
    {255, 0, 0},
    {127, 0, 0},
    {255, 127, 127},
    {127, 64, 64},
    {255, 255, 255},
    {0, 0, 0},
}
local cyan = 180
image.setHue(src, cyan)
assert(src == tensor.ByteTensor{
    {0, 255, 255},
    {0, 127, 127},
    {127, 255, 255},
    {64, 127, 127},
    {255, 255, 255},
    {0, 0, 0},
}, tostring(src))
)";

TEST_F(LuaImageTest, kLuaSetHueRedToCyan) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(
      lua::PushScript(L, kLuaSetHueRedToCyan, sizeof(kLuaSetHueRedToCyan) - 1,
                      "kLuaSetHueRedToCyan"),
      IsOkAndHolds(1));
  EXPECT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr char kLuaSetHueRedToDeepPink[] = R"(
local image = require 'dmlab.system.image'
local tensor = require 'dmlab.system.tensor'
local src = tensor.ByteTensor{
    {255, 0, 0},
    {127, 0, 0},
    {255, 127, 127},
    {127, 64, 64},
    {255, 255, 255},
    {0, 0, 0},
}
local deepPinkHue = 330
image.setHue(src, deepPinkHue)
assert(src == tensor.ByteTensor{
    {255, 0, 127},
    {127, 0, 63},
    {255, 127, 191},
    {127, 64, 95},
    {255, 255, 255},
    {0, 0, 0},
}, tostring(src))
)";

TEST_F(LuaImageTest, kLuaSetHueRedToDeepPink) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kLuaSetHueRedToDeepPink,
                              sizeof(kLuaSetHueRedToDeepPink) - 1,
                              "kLuaSetHueRedToDeepPink"),
              IsOkAndHolds(1));
  EXPECT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr const char kSetMaskedPattern[] = R"(
local image = require 'dmlab.system.image'
local tensor = require 'dmlab.system.tensor'

local img = tensor.ByteTensor{
    {{255,   0,   0,   0}, {255,   0,   0, 128}, {255,   0,   0, 255}},
    {{255,   0,   0,   0}, {255,   0,   0, 128}, {255,   0,   0, 255}},
    {{255,   0,   0,   0}, {255,   0,   0, 128}, {255,   0,   0, 255}},
}

local pat = tensor.ByteTensor{
    {{255}, {255}, {255}},
    {{128}, {128}, {128}},
    {{  0}, {  0}, {  0}},
}

local newImage = image.setMaskedPattern(img:clone(), pat, {0, 255, 0}, {0, 0, 255})

local target = tensor.ByteTensor{
    {{255,   0,   0, 255}, {127, 128,   0, 255}, {  0, 255,   0, 255}},
    {{255,   0,   0, 255}, {127,  64,  64, 255}, {  0, 128, 127, 255}},
    {{255,   0,   0, 255}, {127,   0, 128, 255}, {  0,   0, 255, 255}},
}

assert(newImage == target, tostring(newImage))

local function setMaskedPatternRef(src, pat, color1, color2)
  local hSrc, wSrc, cSrc = unpack(src:shape())
  local hPat, wPat, cPat = unpack(pat:shape())
  assert(hSrc * wSrc == hPat * wPat)
  assert(cSrc == 4)
  assert(cPat > 0)
  local rC1, gC1, bC1 = unpack(color1)
  local rC2, gC2, bC2 = unpack(color2)
  for i = 1, hSrc do
    local rowSrc = src(i)
    local rowPat = pat(i)
    for j = 1, wSrc do
      local rSrc, gSrc, bSrc, aSrc = unpack(rowSrc(j):val())
      local aPat = cPat > 1 and unpack(rowPat(j):val()) or rowPat(j):val()
      local rPat = math.floor((rC1 * aPat + (255 - aPat) * rC2 + 127) / 255)
      local gPat = math.floor((gC1 * aPat + (255 - aPat) * gC2 + 127) / 255)
      local bPat = math.floor((bC1 * aPat + (255 - aPat) * bC2 + 127) / 255)
      rowSrc(j):val{
          math.floor((rSrc * (255 - aSrc) + rPat * aSrc + 127) / 255),
          math.floor((gSrc * (255 - aSrc) + gPat * aSrc + 127) / 255),
          math.floor((bSrc * (255 - aSrc) + bPat * aSrc + 127) / 255),
          255
      }
    end
  end
  return src
end

local newImage2 = setMaskedPatternRef(img:clone(), pat, {0, 255, 0}, {0, 0, 255})
assert(newImage == newImage2, tostring(newImage2))
)";

TEST_F(LuaImageTest, kSetMaskedPattern) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(
      lua::PushScript(L, kSetMaskedPattern, sizeof(kSetMaskedPattern) - 1,
                      "kSetMaskedPattern"),
      IsOkAndHolds(1));
  EXPECT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr const char kSetMaskedPatternInvalidShape[] = R"(
local image = require 'dmlab.system.image'
local tensor = require 'dmlab.system.tensor'
image.setMaskedPattern(
    tensor.ByteTensor(4, 3, 4),
    tensor.ByteTensor(3, 4, 1),
    {0, 0, 0},
    {0, 0, 0})
)";

TEST_F(LuaImageTest, kSetMaskedPatternInvalidShape) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kSetMaskedPatternInvalidShape,
                              sizeof(kSetMaskedPatternInvalidShape) - 1,
                              "kSetMaskedPatternInvalidShape"),
              IsOkAndHolds(1));
  EXPECT_THAT(lua::Call(L, 0), StatusIs(HasSubstr("[4, 3, 4]")));
}

constexpr const char kSetMaskedPatternInvalidShape2[] = R"(
local image = require 'dmlab.system.image'
local tensor = require 'dmlab.system.tensor'
image.setMaskedPattern(
    tensor.ByteTensor(4, 3, 1, 4),
    tensor.ByteTensor(4, 3, 1),
    {0, 0, 0},
    {0, 0, 0})
)";

TEST_F(LuaImageTest, kSetMaskedPatternInvalidShape2) {
  lua_State* L = lua_vm_.get();
  ASSERT_THAT(lua::PushScript(L, kSetMaskedPatternInvalidShape2,
                              sizeof(kSetMaskedPatternInvalidShape2) - 1,
                              "kSetMaskedPatternInvalidShape2"),
              IsOkAndHolds(1));
  EXPECT_THAT(lua::Call(L, 0), StatusIs(HasSubstr("same number")));
}

}  // namespace lab
}  // namespace deepmind
