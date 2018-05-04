// Copyright (C) 2017-2018 Google Inc.
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

#ifndef DML_DEEPMIND_UTIL_RUN_EXECUTABLE_H_
#define DML_DEEPMIND_UTIL_RUN_EXECUTABLE_H_

#include <string>

namespace deepmind {
namespace lab {
namespace util {

// Runs the provided 'command_line' on the system.
// 'message' will contain a human-readable description of the platform-dependent
// result of running the command. On our Linux, uses the semantics of
// wait(2).
// Returns whether the command ran successfully.
bool RunExecutable(const char* command_line, std::string* message);

// Runs the provided 'command_line' on the system.
// 'output' will be appended with the output of stdout.
// 'message' will contain a human-readable description of the platform-dependent
// result of running the command. On our Linux, uses the semantics of
// wait(2).
// Returns whether the command ran successfully.
bool RunExecutableWithOutput(const char* command_line, std::string* message,
                             std::string* output);

}  // namespace util
}  // namespace lab
}  // namespace deepmind
#endif  // DML_DEEPMIND_UTIL_RUN_EXECUTABLE_H_
