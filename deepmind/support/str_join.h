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

#ifndef DEEPMIND_SUPPORT_STR_JOIN_H
#define DEEPMIND_SUPPORT_STR_JOIN_H

#include <iterator>
#include <string>

namespace strings {

template <typename T, typename F>
inline std::string Join(const T& t, const std::string& sep, F f) {
  using std::begin;
  using std::end;

  std::string result;

  auto it = begin(t), eit = end(t);

  if (it != eit) {
    f(&result, *it);
    ++it;
  }

  for (; it != eit; ++it) {
    result.append(sep);
    f(&result, *it);
  }

  return result;
}

}  // namespace strings

#endif  // DEEPMIND_SUPPORT_STR_JOIN_H
