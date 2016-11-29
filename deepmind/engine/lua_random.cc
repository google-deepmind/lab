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

#include <cerrno>
#include <cstdlib>
#include <limits>
#include <random>
#include <string>

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

const char* LuaRandom::ClassName() {
  return "deepmind.lab.RandomView";
}

void LuaRandom::Register(lua_State* L) {
  const Class::Reg methods[] = {
    {"seed", &Class::Member<&LuaRandom::Seed>},
    {"uniformInt", &Class::Member<&LuaRandom::UniformInt>},
    {"uniformReal", &Class::Member<&LuaRandom::UniformReal>},
  };
  Class::Register(L, methods);
}

lua::NResultsOr LuaRandom::Seed(lua_State* L) {
  std::string s;
  RbgNumType k;

  if (ReadLargeNumber(L, -1, &k)) {
    prbg_->seed(k);
    return 0;
  } else if (lua::Read(L, -1, &s)) {
    auto& err = errno;  // cache TLS-lookup
    char* ep;
    err = 0;
    unsigned long long int n = std::strtoull(s.data(), &ep, 0);
    if (ep != s.data() && *ep == '\0' && err == 0 &&
        n <= std::numeric_limits<RbgNumType>::max()) {
      prbg_->seed(n);
      return 0;
    }
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

}  // namespace lab
}  // namespace deepmind
