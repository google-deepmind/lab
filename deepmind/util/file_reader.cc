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

#include "deepmind/util/file_reader.h"

#include <cstddef>

#include "absl/strings/str_cat.h"
#include "public/file_reader_types.h"

namespace deepmind {
namespace lab {
namespace util {

FileReader::FileReader(const DeepMindReadOnlyFileSystem* readonly_fs,
                       const char* filename)
    : readonly_fs_(readonly_fs) {
  success_ = readonly_fs_->open(filename, &handle_);
}

FileReader::~FileReader() { readonly_fs_->close(&handle_); }

bool FileReader::GetSize(std::size_t* size) {
  success_ = success_ && readonly_fs_->get_size(handle_, size);
  return success_;
}

bool FileReader::Read(std::size_t offset, std::size_t size, char* dest_buf) {
  success_ = success_ && readonly_fs_->read(handle_, offset, size, dest_buf);
  return success_;
}

bool FileReader::Success() const { return success_; }

const char* FileReader::Error() { return readonly_fs_->error(handle_); }

}  // namespace util
}  // namespace lab
}  // namespace deepmind
