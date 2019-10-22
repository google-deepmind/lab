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
//
// Non-portable dynamic library loader for DeepMind Lab.

#include <dlfcn.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef __APPLE__
#  include <copyfile.h>
#else  // defined(__APPLE__)
#  include <sys/sendfile.h>
#endif  // defined(__APPLE__)

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <mutex>
#include <string>

#ifdef LEAK_SANITIZER
#include <sanitizer/lsan_interface.h>
#endif

#include "absl/container/flat_hash_map.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "public/dmlab.h"

namespace {

std::mutex connect_mutex;

struct InternalContext {
  void (*release_context)(void* context);
  void* dlhandle;
};

absl::flat_hash_map<void*, InternalContext>* context_map() {
  static absl::flat_hash_map<void*, InternalContext> internal_context;
  return &internal_context;
}

void close_handle(void* context) {
  std::lock_guard<std::mutex> connect_guard(connect_mutex);
  auto ctx_map = context_map();
  auto it = ctx_map->find(context);
  if (it != ctx_map->end()) {
    it->second.release_context(context);
#ifdef LEAK_SANITIZER
    // This function is usually called by LSAN at the end of the process.
    // Since dlclose is somewhat like an end of process as far as the DSO
    // is concerned, we call LSAN eagerly here. This prevents LSAN from
    // considering still-reachable DSO-global allocations as overall leaks.
    // This call effectively ends the use of LSAN in this process, since
    // future calls of this function are no-ops. We will therefore only
    // detect leaks that have happened up until now, but in typical uses,
    // there will be only one single dlclose near the end of the program.
    //
    // We have tried hard to minimize the amount of such leaks. It is worth
    // checking periodically (by disabling the following line) how much each
    // DSO load leaks, though, to make sure no large regressions sneak back
    // in. The only culprits at the moment are various OpenGL libraries.
    //
    // Note that it can be tricky to symbolize LSAN backtraces after the DSO
    // has been unloaded. You will at least want to make a note of the process
    // module maps just before the return from dmlab_connect below, e.g. via:
    // std::cerr << "Maps:\n" << std::ifstream("/proc/self/maps").rdbuf();

    __lsan_do_leak_check();
#endif
    dlclose(it->second.dlhandle);
    ctx_map->erase(it);
  }
}

// Copies the input file to the output file, where both files are given by open
// file descriptors. Returns 0 on success and a negative value on error. In the
// error case, the return value comes from an underlying library call, and errno
// may be set accordingly.
ssize_t copy_complete_file(int in_fd, int out_fd) {
#ifdef __APPLE__
  return fcopyfile(in_fd, out_fd, nullptr, COPYFILE_ALL);
#else  // defined(__APPLE__)
  off_t offset = 0;
  struct stat stat_in;

  if (fstat(in_fd, &stat_in) == -1) {
    std::cerr << "Failed to read source filesize\n";
    return -1;
  }

  for (ssize_t count = stat_in.st_size, bytes_read = 0; bytes_read < count;) {
    ssize_t res = sendfile(out_fd, in_fd, &offset, count - bytes_read);
    if (res < 0) {
      // An error occurred.
      if (errno == EINTR || errno == EAGAIN) {
        // e.g. intervening interrupt, just keep trying
        continue;
      } else {
        // unrecoverable error
        return res;
      }
    } else if (res == 0) {
      // The file was shorter than fstat originally reported, but that's OK.
      return 0;
    } else {
      // No error, res bytes were read.
      bytes_read += res;
    }
  }

  return 0;
#endif  // defined(__APPLE__)
}

// The first call to this function opens the original DSO, as specified by
// `so_path`. Subsequent calls create uniquely-named copies of the DSO and
// open the copy. This is because dlopen identiefies DSOs by file name and
// loads a distinct copy if and only if the file name is distinct.
//
// Returns the DSO handle on success, or null on error. Errors may be due
// either to dlopen or to file operations; in each case the error is logged.
void* open_unique_dso(const std::string& so_path) {
  static int global_counter;
  int current = global_counter++;
  void* dlhandle;

  if (current == 0) {
    dlhandle = dlopen(so_path.c_str(), RTLD_NOW | RTLD_LOCAL);
  } else {
    int source_fd = open(so_path.c_str(), O_RDONLY, 0);
    if (source_fd < 0) {
      std::cerr << "Failed to open library: \"" << so_path << "\"\n"
                << errno << " - " << std::strerror(errno) << "\n";
      return nullptr;
    }

    absl::string_view suffix = "_dmlab.so";
    std::string temp_path = absl::StrCat(P_tmpdir "/unique_dso_filename_",
                                         current, "_XXXXXX", suffix);
    int dest_fd = mkstemps(&temp_path.front(), suffix.size());

    if (dest_fd < 0) {
      std::cerr << "Failed to make library: \"" << temp_path << "\"\n"
                << errno << " - " << std::strerror(errno) << "\n";
      close(source_fd);
      return nullptr;
    }

    if (copy_complete_file(source_fd, dest_fd) < 0) {
      std::cerr << "Failed to copy file to destination \"" << temp_path
                << "\"\n" << errno << " - " << std::strerror(errno) << "\n";
      std::remove(temp_path.c_str());
      close(dest_fd);
      close(source_fd);
      return nullptr;
    }

    dlhandle = dlopen(temp_path.c_str(), RTLD_NOW | RTLD_LOCAL);
    std::remove(temp_path.c_str());
    close(dest_fd);
    close(source_fd);
  }

  if (dlhandle == nullptr) {
    std::cerr << "Failed to open library! - " << so_path << "\n"
              << dlerror() << "\n";
  }

  return dlhandle;
}

}  // namespace

int dmlab_connect(const DeepMindLabLaunchParams* params, EnvCApi* env_c_api,
                  void** context) {
  std::lock_guard<std::mutex> connect_guard(connect_mutex);
  if (params == nullptr) {
    std::cerr << "Require params to be not null!\n";
    return 1;
  }
  std::string so_path;
  if (params->runfiles_path != nullptr && params->runfiles_path[0] != '\0') {
    so_path = params->runfiles_path;

    switch (params->renderer) {
      case DeepMindLabRenderer_Software:
        so_path += "/libdmlab_headless_sw.so";
        break;
      case DeepMindLabRenderer_Hardware:
        so_path += "/libdmlab_headless_hw.so";
        break;
      default:
        std::cerr << "Invalid renderer!\n";
        return 1;
    }
  } else {
    std::cerr << "Require runfiles_directory!\n";
    return 1;
  }

  void* dlhandle = open_unique_dso(so_path);

  if (dlhandle == nullptr) {
    return 1;
  }

  auto ctx_map = context_map();
  for (const auto& pair : *ctx_map) {
    if (pair.second.dlhandle == dlhandle) {
      std::cerr << "Failed to create new instance of library!\n";

      return 1;
    }
  }
  using EnvCApiConnect = int(const DeepMindLabLaunchParams* params,
                             EnvCApi* env_c_api, void** context);
  EnvCApiConnect* connect;
  *reinterpret_cast<void**>(&connect) = dlsym(dlhandle, "dmlab_connect");
  if (connect == nullptr) {
    std::cerr << "Failed to find function dmlab_connect in library!\n";
    return 1;
  }

  int connect_status = connect(params, env_c_api, context);
  if (connect_status != 0) {
    std::cerr << "Failed to call function dmlab_connect, return value was: "
              << connect_status << "\n";
    return 1;
  }

  // Monkey-patch release_context with a wrapper that also calls dlclose.
  void (*release_context)(void* context) = env_c_api->release_context;
  env_c_api->release_context = close_handle;

  ctx_map->emplace(*context, InternalContext{release_context, dlhandle});

  return 0;
}
