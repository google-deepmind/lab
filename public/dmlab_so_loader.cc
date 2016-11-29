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
// Non-portable dynamic library loader for DeepMind Lab.
// Requires the macro DMLAB_SO_LOCATION to be defined as the path to the
// shared object file.

#include "public/dmlab.h"

#include <dlfcn.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <mutex>
#include <string>
#include <unordered_map>

#ifndef DMLAB_SO_LOCATION
#error Must define DMLAB_SO_LOCATION dynamic library path.
#endif

namespace {

std::mutex connect_mutex;

constexpr const char kLibraryName[] = "/" DMLAB_SO_LOCATION;

struct InternalContext {
  void (*release_context)(void* context);
  void* dlhandle;
};

std::unordered_map<void*, InternalContext>* context_map() {
  static std::unordered_map<void*, InternalContext> internal_context;
  return &internal_context;
}

// The first call to this returns an empty string.
// Subsequent calls return a unique name that can be used for generating new
// files.
std::string make_unique_path() {
  static int global_counter;
  int current = global_counter++;
  if (current == 0) {
    return std::string();
  } else {
    static const std::string temp_name = std::tmpnam(nullptr);
    return temp_name + "_dmlab.so." + std::to_string(current);
  }
}

void close_handle(void* context) {
  std::lock_guard<std::mutex> connect_guard(connect_mutex);
  auto ctx_map = context_map();
  auto it = ctx_map->find(context);
  if (it != ctx_map->end()) {
    it->second.release_context(context);
    dlclose(it->second.dlhandle);
    ctx_map->erase(it);
  }
}

ssize_t send_complete_file(int out_fd, int in_fd, off_t offset, ssize_t count) {
  ssize_t bytes;
  ssize_t bytes_count = 0;
  do {
    bytes = sendfile(out_fd, in_fd, &offset, count - bytes_count);
    if (bytes <= 0) {
      if (errno == EINTR || errno == EAGAIN) {
        continue;
      }
      return bytes;
    }
    bytes_count += bytes;
  } while (bytes_count < count);
  return bytes_count;
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
    so_path += kLibraryName;
  } else {
    std::cerr << "Require runfiles_diectory!\n";
    return 1;
  }

  std::string so_path_original = so_path;
  std::string temp_path = make_unique_path();

  // If a temp_path was generated we need to copy with that name to convince
  // dlopen to create a new handle to the library.
  if (!temp_path.empty()) {
    int source = open(so_path.c_str(), O_RDONLY, 0);
    if (source < 0) {
      std::cerr << "Failed to open library: \"" << so_path << "\"\n"
                << errno << " - " << std::strerror(errno) << "\n";
      return 1;
    }
    int dest = open(temp_path.c_str(), O_WRONLY | O_CREAT, 0744);
    if (dest < 0) {
      close(source);
      std::cerr << "Failed to make library: \"" << temp_path << "\"\n"
                << errno << " - " << std::strerror(errno) << "\n";
      return 1;
    }

    struct stat stat_source;

    if (fstat(source, &stat_source) == -1) {
      close(source);
      close(dest);
      std::remove(temp_path.c_str());
      std::cerr << "Failed to read library size: \"" << so_path << "\"\n"
                << errno << " - " << std::strerror(errno) << "\n";
      return 1;
    }

    if (send_complete_file(dest, source, 0, stat_source.st_size) == -1) {
      std::cerr << "Failed to copy file to destination \"" << temp_path
                << "\"\n"
                << errno << " - " << std::strerror(errno) << "\n";
      close(source);
      close(dest);
      std::remove(temp_path.c_str());
      return 1;
    }

    close(source);
    close(dest);
    so_path = temp_path;
  }

  void* dlhandle = dlopen(so_path.c_str(), RTLD_NOW | RTLD_LOCAL);

  // If a new file was created, unlink it immediately.
  if (!temp_path.empty()) {
    std::remove(temp_path.c_str());
  }

  if (dlhandle == nullptr) {
    std::cerr << "Failed to open library! - " << so_path_original << "\n"
              << dlerror() << "\n";
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
  connect(params, env_c_api, context);

  // Monkey-patch release_context with a wrapper that also calls dlclose.
  void (*release_context)(void* context) = env_c_api->release_context;
  env_c_api->release_context = close_handle;

  ctx_map->emplace(*context, InternalContext{release_context, dlhandle});

  return 0;
}
