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

#include "deepmind/tensor/tensor_view.h"

#include <cstddef>
#include <iomanip>
#include <sstream>

namespace deepmind {
namespace lab {
namespace tensor {

void Layout::PrintToStream(
    std::size_t max_num_elements, std::ostream* os,
    std::function<void(std::ostream* os, std::size_t offset)> printer) const {
  std::size_t num_elements = this->num_elements();
  *os << std::setfill(' ');
  const auto& s = shape();
  *os << "Shape: [";
  for (auto it = s.begin(); it != s.end(); ++it) {
    if (it != s.begin()) {
      *os << ", ";
    }
    *os << *it;
  }
  *os << "]";
  if (num_elements == 0) {
    *os << " Empty\n";
    return;
  } else if (num_elements == 1) {
    printer(os, offset_);
    *os << "\n";
    return;
  }
  *os << "\n";

  struct Skip {
    std::size_t low = 0;
    std::size_t high = 0;
  };

  std::vector<Skip> skips(shape().size());
  if (num_elements > max_num_elements) {
    for (std::size_t i = 0; i < skips.size(); ++i) {
      if (shape()[i] < 7) continue;
      skips[i].low = 3;
      skips[i].high = shape()[i] - 3;
    }
  }

  std::size_t max_width = 0;
  ForEachOffset([&max_width, &printer](std::size_t offset) {
    std::ostringstream vals;
    printer(&vals, offset);
    max_width = std::max<std::size_t>(max_width, vals.tellp());
    return true;
  });

  ForEachIndexedOffset([os, &s, &printer, max_width, &skips](
                           const ShapeVector& index, std::size_t offset) {
    int open_brackets = std::distance(
        index.rbegin(), std::find_if(index.rbegin(), index.rend(),
                                     [](std::size_t val) { return val != 0; }));
    for (std::size_t i = 0; i < index.size(); ++i) {
      if (skips[i].low >= skips[i].high) {
        continue;
      }
      if (skips[i].low <= index[i] && index[i] < skips[i].high) {
        // If first skipped at top rank print '...'.
        if (index[i] == skips[i].low && open_brackets + i + 1 >= index.size()) {
          if (open_brackets != 0) {
            *os << std::string((s.size() - open_brackets), ' ') << "...,"
                << std::string(open_brackets, '\n');
          } else {
            *os << "..., ";
          }
        }
        return true;
      }
    }

    if (open_brackets != 0) {
      *os << std::string((s.size() - open_brackets), ' ');
      *os << std::string(open_brackets, '[');
    }
    *os << std::setw(max_width);
    printer(os, offset);
    if (index.back() + 1 != s.back()) {
      *os << ", ";
    } else {
      auto s_iter = s.rbegin();
      std::size_t closing_brackets = 0;
      for (auto it = index.rbegin(); it != index.rend() && (*it) + 1 == *s_iter;
           ++it, ++s_iter) {
        ++closing_brackets;
      }
      *os << std::string(closing_brackets, ']');
      if (closing_brackets < index.size()) {
        *os << ',' << std::string(closing_brackets, '\n');
      }
    }
    return true;
  });
}

}  // namespace tensor
}  // namespace lab
}  // namespace deepmind
