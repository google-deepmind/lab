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

#ifndef DML_DEEPMIND_MODEL_GENERATION_MODEL_SETTERS_H_
#define DML_DEEPMIND_MODEL_GENERATION_MODEL_SETTERS_H_

#include <string>
#include <vector>

#include "deepmind/include/deepmind_model_setters.h"
#include "deepmind/model_generation/model.h"

namespace deepmind {
namespace lab {

// Structure to be passed to the mutator API as 'model_data'.
struct ModelSettersData {
  Model model;
  std::vector<std::string> locatorNames;
};

// Returns the mutator API for custom models. This is used to modify model data
// from the ioq3 engine.
DeepmindModelSetters ModelSetters();

}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_MODEL_GENERATION_MODEL_SETTERS_H_
