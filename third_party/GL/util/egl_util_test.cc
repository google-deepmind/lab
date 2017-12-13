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

#include <EGL/eglext.h>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <memory>

#include "gtest/gtest.h"

namespace {

// typedefs for other required EGL functions.
using EGLGetErrorFunc = EGLint (*)(void);
using EGLGetProcAddressFunc =
    __eglMustCastToProperFunctionPointerType (*)(const char* func_name);
using EGLInitializeFunc = EGLBoolean (*)(EGLDisplay display, EGLint* major,
                                         EGLint* minor);
using EGLTerminateFunc = EGLBoolean (*)(EGLDisplay display);

// Global function pointers that we can override at runtime.
EGLGetErrorFunc egl_get_error_func = nullptr;
EGLGetProcAddressFunc egl_get_proc_address_func = nullptr;
EGLInitializeFunc egl_initialize_func = nullptr;
EGLTerminateFunc egl_terminate_func = nullptr;

PFNEGLQUERYDEVICESEXTPROC egl_query_devices_ext_proc = nullptr;
PFNEGLGETPLATFORMDISPLAYEXTPROC egl_get_platform_display_ext_proc = nullptr;

// Implemention for EGL functions used in egl_util.cc. These forward to test
// functions defined at runtime that can be mocked in unit tests.
extern "C" {
EGLAPI EGLint EGLAPIENTRY eglGetError(void) {
  if (egl_get_error_func == nullptr) std::abort();
  return egl_get_error_func();
}

EGLAPI __eglMustCastToProperFunctionPointerType EGLAPIENTRY
eglGetProcAddress(const char* func_name) {
  if (egl_get_proc_address_func == nullptr) std::abort();
  return egl_get_proc_address_func(func_name);
}

EGLAPI EGLBoolean EGLAPIENTRY eglInitialize(EGLDisplay dpy, EGLint* major,
                                            EGLint* minor) {
  if (egl_initialize_func == nullptr) std::abort();
  return egl_initialize_func(dpy, major, minor);
}

EGLAPI EGLBoolean EGLAPIENTRY eglTerminate(EGLDisplay dpy) {
  if (egl_terminate_func == nullptr) std::abort();
  return egl_terminate_func(dpy);
}
}  // extern "C"

// Testing implementation of EGLDisplay. Stores ID which can be used to identify
// a given device.
struct EGLTestDisplay {
  int id;
};

// Testing implementation of EGLDevice. Stores ID which can be used to identify
// a given device.
struct EGLTestDevice {
  int id;
  EGLTestDisplay display;
};

// Device ID's set for each device. Used to check we initialized the correct
// device in tests.
constexpr int kTestDeviceId0 = 0;
constexpr int kTestDeviceId1 = 1;
constexpr int kTestDeviceId2 = 2;

// Default array of EGLDevices, used in EGL function implementations.
static EGLTestDevice egl_test_devices[] = {{kTestDeviceId0, {kTestDeviceId0}},
                                           {kTestDeviceId1, {kTestDeviceId1}},
                                           {kTestDeviceId2, {kTestDeviceId2}}};
constexpr std::size_t kNumDevices =
    sizeof(egl_test_devices) / sizeof(egl_test_devices[0]);

__eglMustCastToProperFunctionPointerType DefaultEGLGetProcAddress(
    const char* func_name) {
  if (std::strcmp(func_name, "eglQueryDevicesEXT") == 0) {
    return reinterpret_cast<__eglMustCastToProperFunctionPointerType>(
        egl_query_devices_ext_proc);
  } else if (std::strcmp(func_name, "eglGetPlatformDisplayEXT") == 0) {
    return reinterpret_cast<__eglMustCastToProperFunctionPointerType>(
        egl_get_platform_display_ext_proc);
  } else {
    return nullptr;
  }
}

EGLBoolean DefaultEGLInitialize(EGLDisplay display, EGLint* major,
                                EGLint* minor) {
  if (major) *major = 1;
  if (minor) *minor = 4;
  return std::find_if(std::begin(egl_test_devices), std::end(egl_test_devices),
                      [display](const EGLTestDevice& device) {
                        return &device.display == display;
                      }) != std::end(egl_test_devices);
}

EGLBoolean DefaultEGLTerminate(EGLDisplay display) {
  if (display == EGL_NO_DISPLAY) return EGL_FALSE;
  return std::find_if(std::begin(egl_test_devices), std::end(egl_test_devices),
                      [display](const EGLTestDevice& device) {
                        return &device.display == display;
                      }) != std::end(egl_test_devices);
}

EGLBoolean DefaultEGLQueryDevicesEXT(EGLint max_devices, EGLDeviceEXT* devices,
                                     EGLint* num_devices) {
  if (devices == nullptr) std::abort();
  if (num_devices == nullptr) std::abort();

  *num_devices = std::min<EGLint>(
      max_devices,
      std::distance(std::begin(egl_test_devices), std::end(egl_test_devices)));
  std::transform(egl_test_devices, egl_test_devices + *num_devices, devices,
                 [](EGLTestDevice& x) { return &x; });
  return EGL_TRUE;
}

EGLDisplay DefaultEGLGetPlatformDisplayEXT(EGLenum platform, void* device_ext,
                                           const EGLint* attrib_list) {
  if (platform == EGL_PLATFORM_DEVICE_EXT) {
    auto device_it =
        std::find_if(std::begin(egl_test_devices), std::end(egl_test_devices),
                     [device_ext](const EGLTestDevice& device) {
                       return &device == device_ext;
                     });
    if (device_it != std::end(egl_test_devices)) {
      return &device_it->display;
    }
  }
  return EGL_NO_DISPLAY;
}

// Fixture to reset EGL function pointers to default implementations for each
// test.
class EGLUtilTest : public ::testing::Test {
 protected:
  void SetUp() override {
    egl_get_error_func = []() -> EGLint { return EGL_SUCCESS; };
    egl_get_proc_address_func = DefaultEGLGetProcAddress;
    egl_initialize_func = DefaultEGLInitialize;
    egl_terminate_func = DefaultEGLTerminate;
    egl_query_devices_ext_proc = DefaultEGLQueryDevicesEXT;
    egl_get_platform_display_ext_proc = DefaultEGLGetPlatformDisplayEXT;
  }

  void TearDown() override {
    egl_get_error_func = nullptr;
    egl_get_proc_address_func = nullptr;
    egl_initialize_func = nullptr;
    egl_query_devices_ext_proc = nullptr;
    egl_get_platform_display_ext_proc = nullptr;
  }
};

TEST_F(EGLUtilTest, CheckCreateEGLDisplay) {
  auto display = CreateInitializedEGLDisplay();
  ASSERT_NE(EGL_NO_DISPLAY, display);

  // Expect to get first display.
  EXPECT_EQ(kTestDeviceId0, static_cast<EGLTestDisplay*>(display)->id);
  EXPECT_EQ(EGL_TRUE, TerminateInitializedEGLDisplay(display));
}

TEST_F(EGLUtilTest, CheckNoDevices) {
  egl_query_devices_ext_proc = [](EGLint max_devices, EGLDeviceEXT* devices,
                                  EGLint* num_devices) -> EGLBoolean {
    if (num_devices == nullptr) std::abort();
    *num_devices = 0;
    return EGL_TRUE;
  };

  EXPECT_EQ(EGL_NO_DISPLAY, CreateInitializedEGLDisplay());

  egl_query_devices_ext_proc = [](EGLint, EGLDeviceEXT*,
                                  EGLint*) -> EGLBoolean { return EGL_FALSE; };

  EXPECT_EQ(EGL_NO_DISPLAY, CreateInitializedEGLDisplay());
}

TEST_F(EGLUtilTest, CheckFailExtensions) {
  egl_get_proc_address_func =
      [](const char*) -> __eglMustCastToProperFunctionPointerType {
    return nullptr;
  };
  EXPECT_EQ(EGL_NO_DISPLAY, CreateInitializedEGLDisplay());

  // Only fail for eglGetPlatformDisplayEXT
  egl_get_proc_address_func =
      [](const char* func_name) -> __eglMustCastToProperFunctionPointerType {
    if (std::strcmp(func_name, "eglGetPlatformDisplayEXT") == 0) {
      return nullptr;
    } else {
      return DefaultEGLGetProcAddress(func_name);
    }
  };
  EXPECT_EQ(EGL_NO_DISPLAY, CreateInitializedEGLDisplay());
}

TEST_F(EGLUtilTest, CheckFailInitialize) {
  egl_initialize_func = [](EGLDisplay, EGLint*, EGLint*) -> EGLBoolean {
    return EGL_FALSE;
  };

  EXPECT_EQ(EGL_NO_DISPLAY, CreateInitializedEGLDisplay());
}

TEST_F(EGLUtilTest, CheckUnavailableDevices) {
  // Only return EGLDisplay for device1 and fail others.
  egl_get_platform_display_ext_proc = [](
      EGLenum platform, void* device_ext,
      const EGLint* attrib_list) -> EGLDisplay {
    auto* test_device = static_cast<EGLTestDevice*>(device_ext);
    if (platform == EGL_PLATFORM_DEVICE_EXT &&
        test_device->id == kTestDeviceId1) {
      return &test_device->display;
    }
    return EGL_NO_DISPLAY;
  };
  auto display = CreateInitializedEGLDisplay();
  ASSERT_NE(EGL_NO_DISPLAY, display);
  EXPECT_EQ(kTestDeviceId1, static_cast<EGLTestDisplay*>(display)->id);

  // Now test EGLInitialize only succeeds for device2. This is the usual way to
  // determine if an EGLDisplay is actually usable, as eglGetPlatformDisplayEXT
  // usually succeeds, even if we can't actually use the device.
  egl_get_platform_display_ext_proc = DefaultEGLGetPlatformDisplayEXT;
  egl_initialize_func = [](EGLDisplay display, EGLint* major,
                           EGLint* minor) -> EGLBoolean {
    auto* test_display = static_cast<EGLTestDisplay*>(display);
    if (test_display && test_display->id == kTestDeviceId2) {
      return DefaultEGLInitialize(display, major, minor);
    }
    return EGL_FALSE;
  };
  display = CreateInitializedEGLDisplay();
  ASSERT_NE(EGL_NO_DISPLAY, display);
  EXPECT_EQ(kTestDeviceId2, static_cast<EGLTestDisplay*>(display)->id);
  EXPECT_EQ(EGL_TRUE, TerminateInitializedEGLDisplay(display));
}

TEST_F(EGLUtilTest, CheckTerminateRefCounting) {
  static int call_count = 0;
  egl_terminate_func = [](EGLDisplay display) -> EGLBoolean {
    if (display == EGL_NO_DISPLAY) return EGL_FALSE;
    call_count++;
    return EGL_TRUE;
  };
  auto d1 = CreateInitializedEGLDisplay();
  ASSERT_NE(EGL_NO_DISPLAY, d1);

  auto d2 = CreateInitializedEGLDisplay();
  EXPECT_EQ(d1, d2);

  EXPECT_EQ(EGL_TRUE, TerminateInitializedEGLDisplay(d1));
  EXPECT_EQ(EGL_TRUE, TerminateInitializedEGLDisplay(d2));
  EXPECT_EQ(1, call_count);

  // Try and terminate again. Should fail.
  EXPECT_EQ(EGL_FALSE, TerminateInitializedEGLDisplay(d1));
}

TEST_F(EGLUtilTest, CheckInitializeDisplayAtIndex) {
  auto d0 = CreateInitializedEGLDisplayAtIndex(0);
  ASSERT_NE(EGL_NO_DISPLAY, d0);

  auto d1 = CreateInitializedEGLDisplayAtIndex(1);
  EXPECT_NE(d0, d1);

  auto invalid_device = CreateInitializedEGLDisplayAtIndex(kNumDevices+1);
  EXPECT_EQ(EGL_NO_DISPLAY, invalid_device);

  EXPECT_EQ(EGL_TRUE, TerminateInitializedEGLDisplay(d0));
  EXPECT_EQ(EGL_TRUE, TerminateInitializedEGLDisplay(d1));
}

}  // namespace
