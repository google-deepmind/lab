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

#include <cstdint>
#include <limits>
#include <random>
#include <vector>

#include "absl/strings/numbers.h"
#include "absl/strings/string_view.h"
#include "deepmind/lua/class.h"
#include "deepmind/lua/push.h"
#include "deepmind/lua/read.h"

namespace deepmind {
namespace lab {
namespace {

// The fundamental integer type of our pseudo-random bit generator,
// an unsigned 64(ish)-bit integer.
using RbgNumType = ::std::mt19937_64::result_type;

// A range-checking version of lua::Read that results in an error if the number
// on the Lua stack is too large to be represented by a 64-bit integer.
//
// Warning: this code is subtle. E.g. it has to be "x < " below, and not "x <=",
// and we must check for "true" comparisons, not false ones, in order to catch
// NaNs.
bool ReadLargeNumber(lua_State* L, int idx, RbgNumType* num) {
  lua_Number x;

  if (lua::Read(L, idx, &x) &&
      std::numeric_limits<RbgNumType>::min() <= x &&
      x < std::numeric_limits<RbgNumType>::max()) {
    *num = x;
    return true;
  } else {
    return false;
  }
}

}  // namespace

lua::NResultsOr LuaRandom::Require(lua_State* L) {
  if (auto* prbg = static_cast<std::mt19937_64*>(
          lua_touserdata(L, lua_upvalueindex(1)))) {
    std::uintptr_t mixer_seed = reinterpret_cast<std::uintptr_t>(
        lua_touserdata(L, lua_upvalueindex(2)));
    LuaRandom::CreateObject(L, prbg, mixer_seed);
    return 1;
  } else {
    return "Missing std::mt19937_64 pointer in up value!";
  }
}

const char* LuaRandom::ClassName() {
  return "deepmind.lab.RandomView";
}

void LuaRandom::Register(lua_State* L) {
  const Class::Reg methods[] = {
    {"discreteDistribution", &Class::Member<&LuaRandom::DiscreteDistribution>},
    {"normalDistribution", &Class::Member<&LuaRandom::NormalDistribution>},
    {"seed", &Class::Member<&LuaRandom::Seed>},
    {"uniformInt", &Class::Member<&LuaRandom::UniformInt>},
    {"uniformReal", &Class::Member<&LuaRandom::UniformReal>},
  };
  Class::Register(L, methods);
}

lua::NResultsOr LuaRandom::Seed(lua_State* L) {
  absl::string_view s;
  RbgNumType k;

  if (ReadLargeNumber(L, -1, &k) ||
      (lua::Read(L, -1, &s) && absl::SimpleAtoi(s, &k))) {
    prbg_->seed(k ^ mixer_seq_);
    return 0;
  }

  return "Argument is not a valid seed value.";
}

lua::NResultsOr LuaRandom::UniformInt(lua_State* L) {
  RbgNumType a, b;
  if (!ReadLargeNumber(L, -2, &a) ||
      !ReadLargeNumber(L, -1, &b) ||
      !(a <= b)) {
    return "Arguments do not form a valid range.";
  }
  lua::Push(L, std::uniform_int_distribution<RbgNumType>(a, b)(*prbg_));
  return 1;
}

lua::NResultsOr LuaRandom::UniformReal(lua_State* L) {
  lua_Number a, b;
  if (!lua::Read(L, -2, &a) || !lua::Read(L, -1, &b) || !(a <= b) ||
      !(b - a <= std::numeric_limits<lua_Number>::max())) {
    return "Arguments do not form a valid range.";
  }
  lua::Push(L, std::uniform_real_distribution<lua_Number>(a, b)(*prbg_));
  return 1;
}

lua::NResultsOr LuaRandom::NormalDistribution(lua_State* L) {
  lua_Number mean, stddev;
  if (!lua::Read(L, -2, &mean) || !lua::Read(L, -1, &stddev)) {
    return "Invalid arguments - 2 numbers expected..";
  }
  lua::Push(L, std::normal_distribution<lua_Number>(mean, stddev)(*prbg_));
  return 1;
}

lua::NResultsOr LuaRandom::DiscreteDistribution(lua_State* L) {
  std::vector<double> weights;

  if (!lua::Read(L, -1, &weights) || weights.empty()) {
    return "Invalid arguments - non empty list of numeric weights expected.";
  }
  lua::Push(L, std::discrete_distribution<lua_Integer>(
      weights.begin(), weights.end())(*prbg_) + 1);
  return 1;
}

}  // namespace lab
}  // namespace deepmind
