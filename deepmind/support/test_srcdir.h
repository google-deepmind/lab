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

#ifndef DML_DEEPMIND_SUPPORT_TEST_SRCDIR_H_
#define DML_DEEPMIND_SUPPORT_TEST_SRCDIR_H_

#include <string>

namespace deepmind {
namespace lab {

// Returns the name of the directory in which the test binary is located.
std::string TestSrcDir();

}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_SUPPORT_TEST_SRCDIR_H_
