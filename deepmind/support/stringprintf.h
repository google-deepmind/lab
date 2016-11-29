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

#ifndef DEEPMIND_SUPPORT_STRINGPRINTF_H
#define DEEPMIND_SUPPORT_STRINGPRINTF_H

#include <cstdarg>
#include <string>

#ifdef __GNUC__
#  define PRINTF_ATTRIBUTE(fmt_arg, first_to_check)         \
       __attribute__((format(printf, fmt_arg, first_to_check)))
#else
#  define PRINTF_ATTRIBUTE(fmt_arg, first_to_check)
#endif

std::string StringPrintf(const char* format, ...)
    PRINTF_ATTRIBUTE(1, 2);

void StringAppendF(std::string* dst, const char* format, ...)
    PRINTF_ATTRIBUTE(2, 3);

void StringAppendV(std::string* dst, const char* format, va_list ap)
    PRINTF_ATTRIBUTE(2, 0);

#endif  // DEEPMIND_SUPPORT_STRINGPRINTF_H
