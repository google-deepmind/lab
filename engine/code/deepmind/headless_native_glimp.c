// Copyright (C) 2016-2018 Google Inc.
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
#include <GL/glext.h>
#include <GL/glx.h>
#include "../sys/sys_local.h"
#include "glimp_common.h"

typedef GLXContext glXCreateContextAttribsARBProc(Display*, GLXFBConfig,
                                                  GLXContext, Bool, const int*);

static GLXPbuffer glx_pbuffer;
static GLXContext glx_context;
static Display* glx_display;

void GLimp_MakeCurrent(void) {
  if (!glXMakeCurrent(glx_display, glx_pbuffer, glx_context)) {
    Sys_Error("GLimp_MakeCurrent - Failed!");
  }
}

void GLimp_Init(qboolean coreContext) {
  static const int visual_attribs[] = {
      GLX_RED_SIZE,   8, GLX_GREEN_SIZE,   8,    GLX_BLUE_SIZE, 8,
      GLX_ALPHA_SIZE, 8, GLX_DOUBLEBUFFER, True, None};

  static const int context_attribs[] = {None};

  GLimp_CommonPreInit();

  const int glx_pbuffer_attribs[] = {GLX_PBUFFER_WIDTH, glConfig.buffWidth,
                                     GLX_PBUFFER_HEIGHT, glConfig.buffHeight,
                                     None};

  glXCreateContextAttribsARBProc* glXCreateContextAttribsARB =
      (glXCreateContextAttribsARBProc*)glXGetProcAddressARB(
          (const GLubyte*)"glXCreateContextAttribsARB");

  if (glXCreateContextAttribsARB == NULL) {
    Sys_Error("Failed to find glXCreateContextAttribsARB!");
  }

  glx_display = XOpenDisplay(NULL);

  if (glx_display == NULL) {
    Sys_Error("XOpenDisplay failed!");
  }

  int number_of_fb_config;
  GLXFBConfig* fb_configs =
      glXChooseFBConfig(glx_display, DefaultScreen(glx_display), visual_attribs,
                        &number_of_fb_config);

  if (fb_configs == NULL || number_of_fb_config == 0) {
    Sys_Error("glXChooseFBConfig failed!");
  }

  glx_context = glXCreateContextAttribsARB(glx_display, fb_configs[0], 0, True,
                                           context_attribs);

  if (glx_context == NULL) {
    Sys_Error("glXCreateContextAttribsARB failed!");
  }

  glx_pbuffer =
      glXCreatePbuffer(glx_display, fb_configs[0], glx_pbuffer_attribs);

  if (glx_pbuffer == 0) {
    Sys_Error("glXCreatePbuffer failed!");
  }

  // clean up:
  XFree(fb_configs);
  XSync(glx_display, False);

  GLimp_CommonPostInit();
}

void* GLimp_GetProcAddress(const char* func) {
  return glXGetProcAddress((const GLubyte*)func);
}

void GLimp_Shutdown(void) {
  glXMakeCurrent(glx_display, 0, NULL);
  glXDestroyPbuffer(glx_display, glx_pbuffer);
  glXDestroyContext(glx_display, glx_context);
  XCloseDisplay(glx_display);
}
