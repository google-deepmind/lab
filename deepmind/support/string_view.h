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

#ifndef DEEPMIND_SUPPORT_STRING_VIEW_H
#define DEEPMIND_SUPPORT_STRING_VIEW_H

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <string>

struct StringPiece {
  std::size_t size() const {
    return size_;
  }

  const char* data() const { return ptr_; }

  char operator[](std::size_t i) const {
    return ptr_[i];
  }

  const char* begin() const { return data(); }
  const char* end() const { return data() + size(); }

  using size_type = std::size_t;

  static constexpr size_type npos = static_cast<size_type>(-1);

  StringPiece()
      : ptr_(nullptr), size_(0) {}

  StringPiece(const char* ntbs)
      : ptr_(ntbs), size_(std::strlen(ntbs)) {}

  StringPiece(const std::string& s)
      : ptr_(s.data()), size_(s.size()) {}

  StringPiece(const char* first, const char* last)
      : ptr_(first), size_(last - first) {}

  size_type find(char c, size_type pos = 0) const {
    if (pos >= size_) return npos;
    auto it = std::find(ptr_ + pos, ptr_ + size_, c);
    return it == ptr_ + size_ ? npos : it - ptr_;
  }

  StringPiece substr(size_type pos, size_type len = npos) {
    if (pos > size_) pos = size_;
    if (len > size_ - pos) len = size_ - pos;
    return StringPiece(ptr_ + pos, ptr_ + pos + len);
  }

  size_type find_first_of(StringPiece chars, size_type pos = 0) {
    if (pos >= size_) return npos;
    const char* it = std::find_first_of(
        begin() + pos, end(), chars.begin(), chars.end());
    return it == end() ? npos : it - begin();
  }

  void remove_prefix(size_type n) {
    assert(n <= size_);
    ptr_ += n;
    size_ -= n;
  }

  const char* ptr_;
  std::size_t size_;
};

#endif  // DEEPMIND_SUPPORT_STRING_VIEW_H
