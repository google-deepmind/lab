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
// A small Lua class that exposes an interface to an underlying C++ random bit
// generator. Instances of this class do not own the generator; ownership and
// management of actual random generators must be provided elsewhere.
//
// The PRBG's fundamental integer type is a 64-bit unsigned integer. Care needs
// to be taken when using Lua's numbers, which may not have the same amount of
// precision. The documentation for the individual member functions points out
// details.

#ifndef DML_DEEPMIND_ENGINE_LUA_RANDOM_H_
#define DML_DEEPMIND_ENGINE_LUA_RANDOM_H_

#include <random>

#include "deepmind/lua/class.h"
#include "deepmind/lua/lua.h"
#include "deepmind/lua/n_results_or.h"

namespace deepmind {
namespace lab {

class LuaRandom : public lua::Class<LuaRandom> {
  friend class Class;
  static const char* ClassName();

 public:
  // Constructed with a non-owning view of a PRBG instance.
  explicit LuaRandom(std::mt19937_64* prbg, std::uint32_t mixer_seed)
      : prbg_(prbg), mixer_seq_(static_cast<std::uint64_t>(mixer_seed) << 32) {}

  // Registers the class as well as member functions:
  //
  // * seed(value):   [-1, 0, e]
  //
  //   Attempts to set the seed for the underlying generator to "value".
  //
  //   Sequences of random numbers generated with the distribution functions
  //   below are determined by the seed, so by resetting the seed to a stored
  //   value, random sequences can be replayed. (Note that the result of
  //   random sampling is only determined within one program execution; it is
  //   *not* deterministic across platforms. The sampling algorithms are not
  //   specified.)
  //
  //   If the argument is a number, then it is an error if that number is not
  //   representable as the generator's integer type. Even if it is, though,
  //   beware that large numbers may lose precision. Alternatively, the
  //   argument may be a string, which is then parsed as an integer, and it
  //   is again an error if the parsing fails or the number is out of range.
  //
  // * uniformInt(a, b):   [-2, 0, e]
  //
  //   Returns an integer sampled uniformly from the interval [a, b]. It is
  //   an error if the two arguments are not both numbers, or if  the numbers
  //   are not representable as the generator's integer type, or if "a < b" is
  //   not true (as integers).
  //
  // * uniformReal(a, b):   [-2, 0, -]
  //
  //   Returns a real number sampled uniformly from the interval [a, b). It
  //   is an error if the two arguments are not both numbers, or if "a <= b"
  //   is not true, or if "b - a" exceeds the largest representable number.
  //   (This disallows either value being "not-a-number" or "infinity".)
  //
  // * normalDistribution(mean, stddev):   [-2, 0, -]
  //
  //   Returns a real number sampled from a normal distribution centered around
  //   mean with standard deviation stddev.  It is an error if the two
  //   arguments are not both numbers.
  //
  //
  // * discreteDistribution(listOfWeights):   [-1, 0, -]
  //
  //   Returns an integer in the range [1, n] where the probability of each
  //   integer i is the value of the ith weight divided by the sum of all n
  //   weights.  It is an error if the argument is not a list of 1 or more
  //   numbers.
  //
  static void Register(lua_State* L);

  // Returns a constructed LuaRandom object on the Lua stack. Lua's upvalue
  // shall be a pointer to an std::mt19937_64 object that shall outlast the
  // Lua-VM.
  //
  // LuaRandom must be registered first.
  // [-0, +1, e]
  static lua::NResultsOr Require(lua_State* L);

  // Sets a new seed for the PRBG. Results in an error if the first argument of
  // the call is not a number that can be represented by the seed type, which is
  // an unsigned 64-bit integer. The argument may be given either as a number or
  // as a string. In the latter case, it is parsed according to the usual rules
  // as decimal, octal or hexadecimal.
  //
  // [-1, 0, e]
  lua::NResultsOr Seed(lua_State* L);

  // Distributions. See above for semantics. Includes range-checking.
  //
  // [-2, 0, e]
  lua::NResultsOr UniformInt(lua_State* L);
  lua::NResultsOr UniformReal(lua_State* L);
  lua::NResultsOr NormalDistribution(lua_State* L);
  lua::NResultsOr DiscreteDistribution(lua_State* L);

  // Returns the PRBG instance used by this class.
  std::mt19937_64* GetPrbg() { return prbg_; }

 private:
  std::mt19937_64* prbg_;
  std::uint64_t mixer_seq_;
};

}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_ENGINE_LUA_RANDOM_H_
