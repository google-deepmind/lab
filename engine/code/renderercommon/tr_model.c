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

#include <math.h>

#include "../deepmind/context.h"
#include "tr_common.h"

// Load a custom model as a little endian MD3 data structure, stored in
// *mod_md3.
// Custom models are returned by the level API in response to a call to
// api:createModel with a given model name (mod_name). This function uses the
// DMLab context to query for such models.
// When the custom model is retrieved successfully from the level API, this
// function dynamically allocates the returned MD3 data structure through the
// file-loading system allocator. Ownership is transferred to the caller, which
// must release such memory via Hunk_FreeTempMemory.
// Returns whether the custom model was loaded successfully.
qboolean R_DMLabToMD3(const char *mod_name, md3Header_t **mod_md3) {
  DeepmindContext *ctx = dmlab_context();

  // Load the model in the context.
  if (!ctx->hooks.find_model(ctx->userdata, mod_name)) {
    return qfalse;
  }

  // Retrieve the model's accessor API.
  DeepmindModelGetters model_getters;
  void* model_data;
  ctx->hooks.model_getters(ctx->userdata, &model_getters, &model_data);

  // Compute the required buffer size.
  size_t buffer_len =
      ctx->calls.serialised_model_size(&model_getters, model_data);

  // Allocate the data.
  *mod_md3 = ri.Malloc(buffer_len);

  // Fill up the buffer with MD3 model data.
  ctx->calls.serialise_model(&model_getters, model_data, *mod_md3);

  // Clear the loaded model.
  ctx->hooks.clear_model(ctx->userdata);

  return qtrue;
}
