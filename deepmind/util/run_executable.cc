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

#include "deepmind/util/run_executable.h"

#include <stddef.h>

#include <array>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "absl/log/check.h"
#include "absl/log/log.h"
#include "absl/strings/str_cat.h"

namespace {

// Produce a human-readable description of the platform-dependent result of
// running the command line on the system. On our Linux, uses the semantics of
// wait(2).
bool ParseStatus(int s, std::string* msg) {
  if (s == -1) {
    LOG(QFATAL) << "Failed to call the system. " << std::strerror(errno);
  } else if (WIFEXITED(s)) {
    int retval = WEXITSTATUS(s);
    if (retval == 0) {
      *msg = "exited successfully (return value 0)";
      return true;
    } else if (retval == 127) {
      *msg = absl::StrCat("system() failed to run command. ", retval);
      return false;
    } else {
      *msg = absl::StrCat("exited with failure, return value ", retval);
      return false;
    }
  } else if (WIFSIGNALED(s)) {
    int signum = WTERMSIG(s);
    *msg = absl::StrCat("exited with signal ", signum);
    return false;
  } else {
    LOG(QFATAL) << "The system returned something implausible.";
  }
}

}  // namespace

namespace deepmind {
namespace lab {
namespace util {

bool RunExecutable(const char* command_line, std::string* message) {
  CHECK(command_line != nullptr) << "Must provide command_line!";
  LOG(INFO) << "Running command:\n" << command_line << "\n";
  return ParseStatus(std::system(command_line), message);
}

bool RunExecutableWithOutput(const char* command_line, std::string* message,
                             std::string* output) {
  CHECK(command_line != nullptr) << "Must provide command_line!";
  int& err = errno;
  err = 0;
  if (FILE* pipe = popen(command_line, "r")) {
    std::array<char, 4096> buffer;
    while (size_t read = std::fread(buffer.data(), 1, buffer.size(), pipe)) {
      output->append(buffer.data(), read);
    }
    return ParseStatus(pclose(pipe), message);
  } else {
    *message = "Failed to run command!\n";
    if (err != 0) {
      *message += std::strerror(err);
    }
    return false;
  }
}

}  // namespace util
}  // namespace lab
}  // namespace deepmind
