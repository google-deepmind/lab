// Copyright (C) 2017 Google Inc.
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

#ifndef DML_PUBLIC_LEVEL_CACHE_TYPES_H_
#define DML_PUBLIC_LEVEL_CACHE_TYPES_H_

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DeepMindLabLevelCacheParams_s DeepMindLabLevelCacheParams;

struct DeepMindLabLevelCacheParams_s {
  // Tries to fetch a level from the specified caches into pk3_path.
  // Returns true if and only if the level was found.
  // This function is thread-safe.
  bool (*fetch_level_from_cache)(void* level_cache_context,
                                 const char* const cache_paths[],
                                 int num_cache_paths,
                                 const char* key,
                                 const char* pk3_path);

  // Tries to write a level into all of the specified cache paths.
  // This function is thread-safe.
  void (*write_level_to_cache)(void* level_cache_context,
                               const char* const cache_paths[],
                               int num_cache_paths,
                               const char* key,
                               const char* pk3_path);

  // Context pointer that must be passed to the functions above.
  void* context;
};

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // DML_PUBLIC_LEVEL_CACHE_TYPES_H_
