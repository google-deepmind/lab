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

#include "deepmind/level_generation/compile_map.h"

#include <sys/types.h>
#include <sys/wait.h>

#include <string>

#include "deepmind/support/logging.h"
#include "deepmind/support/str_cat.h"

namespace deepmind {
namespace lab {
namespace {

// Produce a human-readable description of the platform-dependent result of
// system(). On our Linux, uses the semantics of wait(2).
bool ParseStatus(int s, std::string* msg) {
  if (s == -1) {
    LOG(QFATAL) << "Failed to call system().";
  } else if (WIFEXITED(s)) {
    int retval = WEXITSTATUS(s);
    if (retval == 0) {
      *msg = "exited successfully (return value 0)";
      return true;
    } else if (retval == 127) {
      *msg = StrCat("system() failed to run /bin/sh.", retval);
      return false;
    } else {
      *msg = StrCat("exited with failure, return value ", retval);
      return false;
    }
  } else if (WIFSIGNALED(s)) {
    int signum = WTERMSIG(s);
    *msg = StrCat("exited with signal ", signum);
    return false;
  } else {
    LOG(QFATAL) << "system() returned something implausible.";
  }
}

constexpr char kScript[] = "deepmind/level_generation/compile_map.sh";

}  // namespace

bool RunMapCompileFor(const std::string& rundir, const std::string& base) {
  std::string msg, cmd = StrCat(rundir, "/", kScript, " \"", base, "\"");
  LOG(INFO) << "Running map generation for map '" << base
            << "', command:\n" << cmd << "\n";
  bool res = ParseStatus(std::system(cmd.c_str()), &msg);
  LOG(INFO) << "Return status: " << msg << "\n";
  return res;
}

}  // namespace lab
}  // namespace deepmind
