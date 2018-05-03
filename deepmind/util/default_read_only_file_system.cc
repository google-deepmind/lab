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

#include "deepmind/util/default_read_only_file_system.h"

#include <cstddef>
#include <fstream>
#include <string>

#include "absl/strings/str_cat.h"

namespace deepmind {
namespace lab {
namespace util {
namespace {

class FileReaderDefault {
 public:
  FileReaderDefault(const char* filename)
      : ifs_(filename, std::ifstream::binary) {
    if (!ifs_) {
      error_message_ = absl::StrCat("Failed to open file \"", filename, "\"");
    }
  }

  bool Success() const { return error_message_.empty(); }

  bool GetSize(std::size_t* size) {
    if (!Success()) {
      return false;
    }
    if (!ifs_.seekg(0, std::ios::end)) {
      error_message_ = "Failed to read file size";
      return false;
    }
    *size = ifs_.tellg();
    return true;
  }

  bool Read(std::size_t offset, std::size_t size, char* dest_buf) {
    if (!Success()) {
      return false;
    }
    if (!ifs_.seekg(offset, std::ios::beg) || !ifs_.read(dest_buf, size)) {
      error_message_ =
          absl::StrCat("Failed to read from ", offset, " to ", offset + size);
      return false;
    } else {
      return true;
    }
  }

  const char* Error() const { return error_message_.c_str(); }

 private:
  std::ifstream ifs_;
  std::string error_message_;
};

extern "C" {

static bool deepmind_open(const char* filename,
                          DeepMindReadOnlyFileHandle* handle) {
  auto* readonly_file = new FileReaderDefault(filename);
  *handle = readonly_file;
  return readonly_file->Success();
}

static bool deepmind_get_size(DeepMindReadOnlyFileHandle handle,
                              std::size_t* size) {
  return handle != nullptr &&
         static_cast<FileReaderDefault*>(handle)->GetSize(size);
}

static bool deepmind_read(DeepMindReadOnlyFileHandle handle, std::size_t offset,
                          std::size_t size, char* dest_buf) {
  return handle != nullptr &&
         static_cast<FileReaderDefault*>(handle)->Read(offset, size, dest_buf);
}

static const char* deepmind_error(DeepMindReadOnlyFileHandle handle) {
  return handle ? static_cast<const FileReaderDefault*>(handle)->Error()
                : "Invalid Handle!";
}

static void deepmind_close(DeepMindReadOnlyFileHandle* handle) {
  delete static_cast<FileReaderDefault*>(*handle);
  *handle = nullptr;
}

}  // extern "C"

constexpr DeepMindReadOnlyFileSystem kFileSystem = {
    &deepmind_open,      //
    &deepmind_get_size,  //
    &deepmind_read,      //
    &deepmind_error,     //
    &deepmind_close,     //
};

}  // namespace

const DeepMindReadOnlyFileSystem* DefaultReadOnlyFileSystem() {
  return &kFileSystem;
}

}  // namespace util
}  // namespace lab
}  // namespace deepmind
