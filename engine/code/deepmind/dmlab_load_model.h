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

#ifndef DML_ENGINE_CODE_DEEPMIND_DMLAB_LOAD_MODEL_H_
#define DML_ENGINE_CODE_DEEPMIND_DMLAB_LOAD_MODEL_H_

#include <stdbool.h>

#include "../../../deepmind/include/deepmind_model_setters.h"

// Attempts to load a serialised model from the MD3 data in 'buffer', invoking
// the relevant callbacks in 'model_setters' as the data is traversed.
// Returns whether valid model data was found in the buffer.
bool dmlab_deserialise_model(                   //
    const void* buffer,                         //
    const DeepmindModelSetters* model_setters,  //
    void* model_data);

// Attempts to load a model from a MD3 file at the given path, invoking the
// relevant callbacks in 'model_setters' as the file is traversed.
// Returns whether a valid model file was found at the given path.
bool dmlab_load_model(                          //
    const char* model_path,                     //
    const DeepmindModelSetters* model_setters,  //
    void* model_data);

#endif  // DML_ENGINE_CODE_DEEPMIND_DMLAB_LOAD_MODEL_H_
