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

#include <iomanip>
#include <sstream>

namespace deepmind {
namespace lab {
namespace tensor {

void Layout::PrintToStream(
    std::ostream* os,
    std::function<void(std::ostream* os, std::size_t offset)> printer) const {
  std::size_t num_elements = this->num_elements();
  *os << std::setfill(' ');
  if (num_elements == 0) {
    *os << "Empty Tensor\n";
  } else if (num_elements == 1) {
    printer(os, offset_);
    *os << "\n";
    return;
  }

  const auto& s = shape();
  *os << "Shape: [";
  for (auto it = s.begin(); it != s.end(); ++it) {
    if (it != s.begin()) {
      *os << ", ";
    }
    *os << *it;
  }
  *os << "]\n";
  if (num_elements > 64) {
    *os << "Too many elements to display.\n";
    return;
  }

  std::size_t max_width = 0;
  ForEachOffset([&max_width, &printer](std::size_t offset) {
    std::ostringstream vals;
    printer(&vals, offset);
    max_width = std::max<std::size_t>(max_width, vals.tellp());
    return true;
  });

  ForEachIndexedOffset([os, &s, &printer, max_width](
      const std::vector<std::size_t>& index, std::size_t offset) {
    int open_brackets = std::distance(
        index.rbegin(), std::find_if(index.rbegin(), index.rend(),
                                     [](std::size_t val) { return val != 0; }));

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
      int closing_brackets = 0;
      for (auto it = index.rbegin(); it != index.rend() && (*it) + 1 == *s_iter;
           ++it, ++s_iter) {
        ++closing_brackets;
      }

      if (closing_brackets != 0) {
        *os << std::string(closing_brackets, ']');
      }
      *os << "\n";
    }
    return true;
  });
}

}  // namespace tensor
}  // namespace lab
}  // namespace deepmind
