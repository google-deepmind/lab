// Copyright (C) 2016-2019 Google Inc.
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

#include "deepmind/support/test_srcdir.h"

#include <cstdlib>
#include <string>

#include "absl/strings/str_cat.h"

#ifndef SUPPRESS_COMMANDLINE_FLAGS
#include "deepmind/support/commandlineflags.h"
#include "absl/flags/flag.h"
DECLARE_string(test_srcdir);
#endif

namespace deepmind {
namespace lab {

std::string TestSrcDir() {
  if (const char* e = std::getenv("TEST_SRCDIR")) {
    return absl::StrCat(e, "/org_deepmind_lab");
  } else {
#ifndef SUPPRESS_COMMANDLINE_FLAGS
    return absl::StrCat(absl::GetFlag(FLAGS_test_srcdir),
                        "/org_deepmind_lab");
#else
    return "[undefined TEST_SRCDIR environment variable]";
#endif
  }
}

}  // namespace lab
}  // namespace deepmind
