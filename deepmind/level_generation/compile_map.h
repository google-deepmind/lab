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
// C++ bindings to the map compilation shell script.

#ifndef DML_DEEPMIND_LEVEL_GENERATION_COMPILE_MAP_H_
#define DML_DEEPMIND_LEVEL_GENERATION_COMPILE_MAP_H_

#include <string>

namespace deepmind {
namespace lab {

// Runs the map compiler for the map <base>.map, producing <base>.pk3.
// The rundir parameter contains the name of base directory of the executable,
// with respect to which the compilation script is located.
//
// Returns whether the compilation succeeded.
bool RunMapCompileFor(const std::string& rundir, const std::string& base);

}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_LEVEL_GENERATION_COMPILE_MAP_H_
