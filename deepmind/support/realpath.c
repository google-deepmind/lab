// Copyright (C) 2018 Google Inc.
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
// A minimal driver for Posix realpath(). Not every Posix system supplies such
// a driver, and even on those that do it might require a separate installation
// step. This implements the equivalent of "realpath -e" on Linux.

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]) {
  int num_errors = 0;
  errno = 0;

  for (int i = 1; i < argc; ++i) {
    char* p = realpath(argv[i], NULL);
    if (p == NULL) {
      fprintf(stderr, "Error resolving path '%s', error was: '%s'\n",
              argv[i], strerror(errno));
      errno = 0;
      ++num_errors;
    } else {
      fprintf(stdout, "%s\n", p);
      free(p);
    }
  }

  return num_errors == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
