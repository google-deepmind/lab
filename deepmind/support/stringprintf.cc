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

#include "deepmind/support/stringprintf.h"

#include <cstdio>
#include <memory>

std::string StringPrintf(const char* format, ...) {
  std::string result;

  va_list ap;
  va_start(ap, format);
  StringAppendV(&result, format, ap);
  va_end(ap);

  return result;
}

void StringAppendF(std::string* dst, const char* format, ...) {
  va_list ap;
  va_start(ap, format);
  StringAppendV(dst, format, ap);
  va_end(ap);
}

void StringAppendV(std::string* dst, const char* format, va_list ap) {
  static const int kSpaceLength = 1024;
  char space[kSpaceLength];

  va_list backup_ap;
  va_copy(backup_ap, ap);
  int result = std::vsnprintf(space, kSpaceLength, format, backup_ap);
  va_end(backup_ap);

  if (result < 0) {
    // An error.
  } else if (result < kSpaceLength) {
    // Normal case, everything fit.
    dst->append(space, result);
  } else {
    // Output too long, need to allocate dynamic buffer.
    const int length = result + 1;
    std::unique_ptr<char[]> buf(new char[length]);
    result = std::vsnprintf(buf.get(), length, format, ap);

    if (result >= 0 && result < length) {
      // It fit.
      dst->append(buf.get(), result);
    } else {
      // An error.
    }
  }
}
