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

#ifndef DML_ENGINE_CODE_DEEPMIND_DMLAB_SAVE_MODEL_H_
#define DML_ENGINE_CODE_DEEPMIND_DMLAB_SAVE_MODEL_H_

#include <stdbool.h>
#include <stddef.h>

#include "../../../deepmind/include/deepmind_model_getters.h"

// Returns the buffer size required to serialise the model described by
// 'model_getters' in MD3 format.
size_t dmlab_serialised_model_size(             //
    const DeepmindModelGetters* model_getters,  //
    void* model_data);

// Attempts to serialise the model described by 'model_getters' in MD3 format
// onto 'buffer'.
void dmlab_serialise_model(                     //
    const DeepmindModelGetters* model_getters,  //
    void* model_data,                           //
    void* buffer);

// Attempts to save the model described by 'model_getters' as a MD3 file onto
// the given path.
// Returns whether the save operation succeeded.
bool dmlab_save_model(                          //
    const DeepmindModelGetters* model_getters,  //
    void* model_data,                           //
    const char* model_path);

#endif  // DML_ENGINE_CODE_DEEPMIND_DMLAB_SAVE_MODEL_H_
