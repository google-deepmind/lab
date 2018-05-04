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

#include "deepmind/util/files.h"

#include <ftw.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

namespace deepmind {
namespace lab {
namespace util {

extern "C" {
static int delete_entry(const char* fpath, const struct stat* sb, int typeflag,
                        struct FTW* ftwbuf) {
  return (typeflag == FTW_DP ? rmdir : unlink)(fpath);
}
}  // extern "C"

void RemoveDirectory(const std::string& path) {
  nftw(path.c_str(), delete_entry, 10, FTW_DEPTH | FTW_PHYS);
}

// Recursively build the directories for the folder `path`.
bool MakeDirectory(const std::string& path) {
  struct stat st = {0};
  if (stat(path.c_str(), &st) == 0) {
    return S_ISDIR(st.st_mode);
  }

  auto pos = path.find_last_of('/');
  if (pos == std::string::npos || MakeDirectory(path.substr(0, pos))) {
    mkdir(path.c_str(), 0777);
  }
  return stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

std::string GetTempDirectory() {
  // In unit tests, "TEST_TMPDIR" is the preferred temporary directory.
  const char* tempdir = getenv("TEST_TMPDIR");
  if (tempdir == nullptr) {
    tempdir = getenv("TMPDIR");
    if (tempdir == nullptr) {
      tempdir = "/tmp";
    }
  }
  return tempdir;
}

bool SetContents(const std::string& file_name, absl::string_view contents,
                 const char* scratch_directory) {
  std::string temp_file =
      (scratch_directory != nullptr && scratch_directory[0] != '\0')
          ? std::string(scratch_directory)
          : util::GetTempDirectory();
  temp_file += "/dmlab_temp_file_XXXXXX";
  // Thread safe temporary file generation.
  {
    std::unique_ptr<std::FILE, int (&)(std::FILE*)> file(
        fdopen(mkstemp(&temp_file.front()), "w"), std::fclose);
    if (!file) {
      std::cerr << "Failed to make temp file! " << errno << " - "
                << std::strerror(errno) << "\n";
      return false;
    }
    if (std::fwrite(contents.data(), 1, contents.size(), file.get()) !=
        contents.size()) {
      std::cerr << "Failed to write to temp file! " << errno << " - "
                << std::strerror(errno) << "\n";
      return false;
    }
  }

  if (std::rename(temp_file.c_str(), file_name.c_str()) != 0) {
    std::cerr << "Failed to rename temp file to: " << file_name << " " << errno
              << " - " << std::strerror(errno) << "\n";
    std::remove(temp_file.c_str());
    return false;
  }
  return true;
}

bool GetContents(const std::string& file_name, std::string* contents) {
  std::filebuf fb;
  if (fb.open(file_name, std::ios::in | std::ios::binary) == nullptr) {
    return false;
  }
  contents->reserve(fb.pubseekoff(0, std::ios::end));
  fb.pubseekpos(0);
  contents->assign(std::istreambuf_iterator<char>(&fb), {});
  return true;
}

}  // namespace util
}  // namespace lab
}  // namespace deepmind
