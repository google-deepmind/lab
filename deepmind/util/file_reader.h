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

#ifndef DML_DEEPMIND_UTIL_READONLY_FILE_SYSTEM_H_
#define DML_DEEPMIND_UTIL_READONLY_FILE_SYSTEM_H_

#include <cstddef>

#include "public/file_reader_types.h"

namespace deepmind {
namespace lab {
namespace util {

// Used for opening/reading the contents of a file using a
// DeepMindReadOnlyFileSystem. If the operation was not successful 'Error()' may
// be called to retrieve an error message.
class FileReader {
 public:
  // Opens 'filename' for read. Users should call 'Success()' to retrieve
  // whether the file was successfully opened. 'readonly_fs' must outlive the
  // FileReader.
  FileReader(const DeepMindReadOnlyFileSystem* readonly_fs,
             const char* filename);

  FileReader(const FileReader&) = delete;
  FileReader& operator=(const FileReader&) = delete;
  ~FileReader();

  // Returns whether last operation was successful.
  bool Success() const;

  // Sets 'size' to the size of the file. Returns whether the size was
  // retrieved.
  bool GetSize(std::size_t* size);

  // Reads 'size' bytes from 'offset' in the file into 'dest_buf'. Returns
  // whether read completed successfully. If the operation was not successful,
  // 'dest_buf' may be partially written to.
  bool Read(std::size_t offset, std::size_t size, char* dest_buf);

  // Returns associated error message if an operation was not successful.
  // Only can be called after an unsuccessful operation.
  const char* Error();

 private:
  bool success_;  // Stores whether an operation was successful.
  DeepMindReadOnlyFileHandle handle_;  // Stores current file handle.
  const DeepMindReadOnlyFileSystem* readonly_fs_;
};

}  // namespace util
}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_UTIL_READONLY_FILE_SYSTEM_H_
