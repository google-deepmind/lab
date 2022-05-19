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

#include "deepmind/level_generation/text_level/char_grid.h"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "absl/log/check.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"

namespace deepmind {
namespace lab {

CharGrid::CharGrid(std::string text)
    : raw_data_(std::move(text)),
      rows_(absl::StrSplit(raw_data_, '\n', absl::SkipEmpty())) {
  auto it = std::max_element(rows_.begin(), rows_.end(),
                             [](absl::string_view lhs, absl::string_view rhs) {
                               return lhs.size() < rhs.size();
                             });
  CHECK(it != rows_.end());
  width_ = it->size();
}

}  // namespace lab
}  // namespace deepmind
