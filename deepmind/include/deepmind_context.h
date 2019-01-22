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
//
// Entry point to our ML system. The modified game engine creates a context and
// calls the hooks provided by it.

#ifndef DML_DEEPMIND_INCLUDE_DEEPMIND_CONTEXT_H_
#define DML_DEEPMIND_INCLUDE_DEEPMIND_CONTEXT_H_

#include <stddef.h>

#include "deepmind/include/deepmind_calls.h"
#include "deepmind/include/deepmind_hooks.h"
#include "public/file_reader_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DeepmindContext_s DeepmindContext;

struct DeepmindContext_s {
  DeepmindCalls calls;  // Calls from context to engine provided by engine.
  void* context;        // Calls may require context to be called with.
  DeepmindHooks hooks;  // Calls from engine to context provided by context.
  void* userdata;       // Hooks require userdata to be called with.
};

// Create a new context. If successful, the new context is stored in *ctx and
// the function returns 0. Otherwise, a non-zero value is returned.
//
// When the context is no longer needed, it can be passed to
// `dmlab_release_context` to release the associated resources.
// The context contains a Lua VM.
// 'runfiles_path' must be the path to where DeepMind Lab assets are stored.
// 'ctx': pointer to the new context that is to be initialized.
// 'ctx->calls': are methods provided by the engine to call into it.
// 'file_reader_override': an optional function for reading from the file
// system. If set, a call returns whether the file 'file_name' was read
// successfully and if so 'buff' points to the content and 'size' contains the
// size of the file and after use 'buff' must be freed with 'free'. Otherwise
// returns false.
int dmlab_create_context(
    const char* runfiles_path,
    DeepmindContext* ctx,
    DeepmindFileReaderType* file_reader_override,
    const DeepMindReadOnlyFileSystem* read_only_file_system,
    const char* temp_folder);

// Release the resources associated with *ctx. The context shall have been
// obtained by a successful call of the previous function.
void dmlab_release_context(DeepmindContext* ctx);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // DML_DEEPMIND_INCLUDE_DEEPMIND_CONTEXT_H_
