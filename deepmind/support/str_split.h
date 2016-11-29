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

#ifndef DEEPMIND_SUPPORT_STR_SPLIT_H
#define DEEPMIND_SUPPORT_STR_SPLIT_H

#include <algorithm>
#include <vector>

#include "deepmind/support/string_view.h"

namespace strings {

struct SkipEmpty {};

inline std::vector<StringPiece> Split(StringPiece s, char sep, SkipEmpty = {}) {
  std::vector<StringPiece> result;
  for (const char* first = s.ptr_, * last = first + s.size_; ; ) {
    while (first != last && *first == sep) ++first;
    if (first == last) break;

    const char* middle = std::find(first, last, sep);
    result.emplace_back(first, middle);

    first = middle;
  }
  return result;
}

}  // namespace strings

#endif  // DEEPMIND_SUPPORT_STR_SPLIT_H
