// Copyright 2017 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
////////////////////////////////////////////////////////////////////////////////

#include "third_party/GL/util/egl_util.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <ios>
#include <iostream>
#include <mutex>
#include <thread>
#include <unordered_map>

namespace {

// Maximum number of EGL devices to query. Currently the maximum number of
// devices a machine could have is 16, but we allow space for 32.
constexpr int kMaxDevices = 32;

// Helper function for loading EGL extensions.
template <typename T>
T LoadEGLFunction(const char* func_name) {
  if (T func = reinterpret_cast<T>(eglGetProcAddress(func_name))) {
    return func;
  } else {
    std::cerr << "Failed to load EGL function " << func_name << "\n";
    return nullptr;
  }
}

// Mutex used to lock the display_reference_map and eglInitialize/egTerminate
// calls.
std::mutex* get_display_mutex() {
  static std::mutex* display_reference_mutex = new std::mutex();
  return display_reference_mutex;
}

std::unordered_map<EGLDisplay, int>* get_display_reference_map() {
  static std::unordered_map<EGLDisplay, int>* display_reference_map =
      new std::unordered_map<EGLDisplay, int>();
  return display_reference_map;
}

void IncrementDisplayRefCount(EGLDisplay display) {
  auto* display_map = get_display_reference_map();
  auto iter_inserted = display_map->emplace(display, 0).first;
  ++iter_inserted->second;
}

// Helper to decrement reference count for provided EGLDisplay. Returns the
// reference count after decrementing the provided counter. If the EGLDisplay is
// not found, return -1.
int DecrementDisplayRefCount(EGLDisplay display) {
  auto* display_map = get_display_reference_map();
  auto it = display_map->find(display);
  if (it != display_map->end()) {
    int ref_count = --it->second;
    if (ref_count == 0) display_map->erase(it);
    return ref_count;
  } else {
    return -1;
  }
}

EGLBoolean TerminateInitializedEGLDisplayNoLock(EGLDisplay display) {
  if (display == EGL_NO_DISPLAY) {
    return eglTerminate(display);
  }
  int ref_count = DecrementDisplayRefCount(display);
  if (ref_count == 0) {
    return eglTerminate(display);
  } else if (ref_count > 0) {
    return EGL_TRUE;
  } else {
    std::cerr << "Could not find EGLDisplay Reference count! Either we didn't "
                 "create EGLDisplay with CreateInitializedEGLDisplay() or we "
                 "have already terminated the display.\n";
    return EGL_FALSE;
  }
}

}  // namespace

extern "C" EGLDisplay CreateInitializedEGLDisplayAtIndex(int device_index) {
  // Load EGL extension functions for querying EGL devices manually. This
  // extension isn't officially supported in EGL 1.4, so try and manually
  // load them using eglGetProcAddress.
  auto eglQueryDevicesEXT =
      LoadEGLFunction<PFNEGLQUERYDEVICESEXTPROC>("eglQueryDevicesEXT");
  if (eglQueryDevicesEXT == nullptr) return EGL_NO_DISPLAY;

  auto eglGetPlatformDisplayEXT =
      LoadEGLFunction<PFNEGLGETPLATFORMDISPLAYEXTPROC>(
          "eglGetPlatformDisplayEXT");
  if (eglGetPlatformDisplayEXT == nullptr) return EGL_NO_DISPLAY;

  EGLDeviceEXT egl_devices[kMaxDevices];
  EGLint num_devices = 0;
  auto egl_error = eglGetError();
  if (!eglQueryDevicesEXT(kMaxDevices, egl_devices, &num_devices) ||
      egl_error != EGL_SUCCESS) {
    std::cerr << "eglQueryDevicesEXT Failed. EGL error " << std::hex
               << eglGetError() << "\n";
    return EGL_NO_DISPLAY;
  }

  // Go through each device and try to initialize the display.
  for (EGLint i = 0; i < num_devices; ++i) {
    // First try and get a valid EGL display.
    auto display = eglGetPlatformDisplayEXT(EGL_PLATFORM_DEVICE_EXT,
                                            egl_devices[i], nullptr);
    if (eglGetError() == EGL_SUCCESS && display != EGL_NO_DISPLAY) {
      // Aquire lock before calling eglInitialize() and incrementing ref count.
      std::lock_guard<std::mutex> display_guard(*get_display_mutex());

      // Now try to initialize the display. This can fail when we don't have
      // access to the device.
      int major, minor;
      EGLBoolean initialized = eglInitialize(display, &major, &minor);
      if (eglGetError() == EGL_SUCCESS && initialized == EGL_TRUE) {
        IncrementDisplayRefCount(display);
        if (--device_index < 0) {
          return display;
        } else {
          TerminateInitializedEGLDisplayNoLock(display);
        }
      }
    }
  }

  std::cerr << "Failed to create and initialize a valid EGL display! "
             << "Devices tried: " << num_devices << "\n";
  return EGL_NO_DISPLAY;
}

extern "C" EGLDisplay CreateInitializedEGLDisplay() {
  return CreateInitializedEGLDisplayAtIndex(0);
}

extern "C" EGLBoolean TerminateInitializedEGLDisplay(EGLDisplay display) {
  // Acquire lock before terminating and decrementing display ref count.
  std::lock_guard<std::mutex> display_guard(*get_display_mutex());
  return TerminateInitializedEGLDisplayNoLock(display);
}

extern "C" void ShutDownEGLSubsystem() {
  delete get_display_reference_map();
  delete get_display_mutex();
}
