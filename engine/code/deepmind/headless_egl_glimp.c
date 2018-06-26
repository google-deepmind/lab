// Copyright (C) 2017 Google Inc.
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

#include <stdlib.h>
#include "../renderercommon/tr_common.h"
// Must include tr_common.h before opengl.
#include <EGL/egl.h>

#include "../sys/sys_local.h"
#include "glimp_common.h"
#include "third_party/GL/util/egl_util.h"

#define CHECK_EGL_SUCCESS(egl_expr)                                        \
  do {                                                                     \
    (egl_expr);                                                            \
    EGLint egl_error = eglGetError();                                      \
    if (egl_error != EGL_SUCCESS) {                                        \
      Sys_Error("EGL ERROR: 0x%x file:%s, line:%d\n", egl_error, __FILE__, \
                __LINE__);                                                 \
    }                                                                      \
  } while (0)

#define RETURN_IF_EGL_ERROR(egl_expr)                                      \
  do {                                                                     \
    (egl_expr);                                                            \
    EGLint egl_error = eglGetError();                                      \
    if (egl_error != EGL_SUCCESS) {                                        \
      Sys_Print(va("EGL ERROR: 0x%x file:%s, line:%d\n", egl_error,        \
                   __FILE__, __LINE__));                                   \
      return;                                                              \
    }                                                                      \
  } while (0)

static const EGLint kConfigAttribs[] = {
  EGL_RED_SIZE, 8,
  EGL_GREEN_SIZE, 8,
  EGL_BLUE_SIZE, 8,
  EGL_ALPHA_SIZE, 8,
  EGL_DEPTH_SIZE, 8,
  EGL_STENCIL_SIZE, 8,
  EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
  EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
  EGL_NONE
};

static EGLDisplay egl_display;
static EGLSurface egl_surface;
static EGLContext egl_context;

void GLimp_MakeCurrent(void) {
  CHECK_EGL_SUCCESS(eglMakeCurrent(egl_display, egl_surface, egl_surface,
                                   egl_context));
}

void GLimp_Init(qboolean coreContext) {
  GLimp_CommonPreInit();

  cvar_t* r_gpu_device_index =
      ri.Cvar_Get("r_gpuDeviceIndex", "0", CVAR_ARCHIVE | CVAR_LATCH);

  egl_display = CreateInitializedEGLDisplayAtIndex(r_gpu_device_index->integer);
  if (egl_display == EGL_NO_DISPLAY) {
    Sys_Error("Failed to create EGL display for device index %d!\n",
              r_gpu_device_index->integer);
    return;
  }

  EGLint num_configs;
  EGLConfig egl_config;
  CHECK_EGL_SUCCESS(eglChooseConfig(egl_display, kConfigAttribs, &egl_config, 1,
                                    &num_configs));

  EGLint pbuffer_attribs[] = {
      EGL_WIDTH,  glConfig.buffWidth,   //
      EGL_HEIGHT, glConfig.buffHeight,  //
      EGL_NONE,
  };

  CHECK_EGL_SUCCESS(egl_surface = eglCreatePbufferSurface(
                        egl_display, egl_config, pbuffer_attribs));

  CHECK_EGL_SUCCESS(eglBindAPI(EGL_OPENGL_API));

  CHECK_EGL_SUCCESS(egl_context = eglCreateContext(egl_display, egl_config,
                                                   EGL_NO_CONTEXT, NULL));

  GLimp_CommonPostInit();
}

void* GLimp_GetProcAddress(const char* func) { return eglGetProcAddress(func); }

void GLimp_Shutdown(void) {
  RETURN_IF_EGL_ERROR(eglMakeCurrent(egl_display, EGL_NO_SURFACE,
                                     EGL_NO_SURFACE, EGL_NO_CONTEXT));
  RETURN_IF_EGL_ERROR(eglDestroySurface(egl_display, egl_surface));
  RETURN_IF_EGL_ERROR(eglDestroyContext(egl_display, egl_context));
  RETURN_IF_EGL_ERROR(TerminateInitializedEGLDisplay(egl_display));
  ShutDownEGLSubsystem();
}
