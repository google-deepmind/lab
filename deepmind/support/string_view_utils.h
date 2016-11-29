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

#ifndef DEEPMIND_SUPPORT_STRING_VIEW_UTILS_H
#define DEEPMIND_SUPPORT_STRING_VIEW_UTILS_H

#include <cctype>

#include "deepmind/support/string_view.h"

namespace strings {

inline StringPiece::size_type RemoveLeadingWhitespace(StringPiece* text) {
  StringPiece::size_type n = 0;
  for (auto* p = text->begin(); p != text->end() && std::isspace(*p); ++p) {
    ++n;
  }
  text->remove_prefix(n);
  return n;
}

}  // namespace strings

#endif  // DEEPMIND_SUPPORT_STRING_VIEW_UTILS_H
