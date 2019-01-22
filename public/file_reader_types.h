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
//
// Function hooks for overriding local file operations.

#ifndef DML_PUBLIC_FILE_READER_TYPES_H_
#define DML_PUBLIC_FILE_READER_TYPES_H_

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DeepMindReadOnlyFileSystem_s DeepMindReadOnlyFileSystem;
typedef void* DeepMindReadOnlyFileHandle;

typedef bool (DeepmindFileReaderType)(const char* file_name, char** buff,
                                      size_t* size);

// These are optional function pointers. If null they will not be used and the
// local filesystem is used instead.
//
// Implementations of these functions must provide the following thread-safety
// properties: For a given handle value, the functions are only called
// sequentially (though potentially from different threads). For different
// handle values, the functions may be called concurrently.
//
// Users of this code may, once a file is opened, only call the functions in
// sequential order for any particular handle.
//
// Whenever a function indicates that an error has occurred, error(handle) can
// be called to retrieve a corresponding error message.
struct DeepMindReadOnlyFileSystem_s {
  // Attempts to open a file named 'filename'. Returns whether file opening was
  // successful. If successful 'get_size(handle)' and 'read(handle)' may be
  // called. Whether the call was successful or not the handle must be closed by
  // calling the corresponding 'close(&handle)'.
  bool (*open)(const char* filename, DeepMindReadOnlyFileHandle* handle);

  // Attempts to retrieve the current size of the file. Returns whether size was
  // successfully retrieved. If successful 'size' contains the current size of
  // the file.
  bool (*get_size)(DeepMindReadOnlyFileHandle handle, size_t* size);

  // Attempts to read size bytes from an absolute offset in the file. Returns
  // whether read is successful. If successful 'dest_buf' has 'size' bytes
  // written to it from 'offset' bytes within the file.
  bool (*read)(DeepMindReadOnlyFileHandle handle, size_t offset, size_t size,
               char* dest_buf);

  // If any of the previous functions returns false, a call to this function
  // will return the corresponding error.
  const char* (*error)(DeepMindReadOnlyFileHandle handle);

  // All handles must be closed. No other parts of
  // DeepMindReadOnlyFileHandleSystem may be called, for a given handle, after
  // this function is called. Passed by pointer to allow handle invalidation.
  void (*close)(DeepMindReadOnlyFileHandle* handle);
};

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // DML_PUBLIC_FILE_READER_TYPES_H_
