// Copyright (C) 2017-2018 Google Inc.
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

#ifndef DML_DEEPMIND_UTIL_FILES_H_
#define DML_DEEPMIND_UTIL_FILES_H_

#include <string>

#include "absl/strings/string_view.h"

namespace deepmind {
namespace lab {
namespace util {

// Recursively builds the directory for the `path` specified.
// `path` shall be in canonicalised form.
// Returns whether the folder was successfully made.
bool MakeDirectory(const std::string& path);

// Recursively removes the directory for the `path` specified.
void RemoveDirectory(const std::string& path);

// Returns an existing temporary directory.
std::string GetTempDirectory();

// A file is written with `contents` into a temporary file first, then renamed
// to `file_name`. The directory name of `file_name` must be an existing
// directory. The temporary file is created in the `scratch_directory` if it is
// not null or empty, otherwise it is created in the system temporary directory.
// Returns whether the file was successfully created with contents.
bool SetContents(const std::string& file_name, absl::string_view contents,
                 const char* scratch_directory = nullptr);

// File at `file_name` is read into `contents`.
// Returns whether `file_name` was successfully read.
bool GetContents(const std::string& file_name, std::string* contents);

}  // namespace util
}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_UTIL_FILES_H_
