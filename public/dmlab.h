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
// The RL Environment API entry point.

#ifndef DML_PUBLIC_DMLAB_H_
#define DML_PUBLIC_DMLAB_H_

#include "third_party/rl_api/env_c_api.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DeepMindLabLaunchParams_s DeepMindLabLaunchParams;

struct DeepMindLabLaunchParams_s {
  // Path to where DeepMind Lab assets are stored.
  const char* runfiles_path;
};

// Starts an instance of DeepMind Lab and exports the single-player RL
// Environment API to *env_c_api. This function may be called at most once
// per process unless called via dmlab_so_loader. (DeepMind Lab is not
// modular and reusable.)
//
// 'params' must be dereferenceable; the DeepMindLabLaunchParams that it
// points to is owned by the caller.
//
// Returns 0 on success and non-zero otherwise.
int dmlab_connect(
    const DeepMindLabLaunchParams* params,
    EnvCApi* env_c_api,
    void** context);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // DML_PUBLIC_DMLAB_H_
