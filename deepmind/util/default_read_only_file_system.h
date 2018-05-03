// Copyright (C) 2018 Google Inc.
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

#ifndef DML_DEEPMIND_UTIL_DEFAULT_READ_ONLY_FILE_SYSTEM_H_
#define DML_DEEPMIND_UTIL_DEFAULT_READ_ONLY_FILE_SYSTEM_H_

#include "public/file_reader_types.h"

namespace deepmind {
namespace lab {
namespace util {

// Returns the default DeepMindReadOnlyFileSystem implemented on std::ifstream.
const DeepMindReadOnlyFileSystem* DefaultReadOnlyFileSystem();

}  // namespace util
}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_UTIL_DEFAULT_READ_ONLY_FILE_SYSTEM_H_
