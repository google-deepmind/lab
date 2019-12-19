// Copyright (C) 2016-2017 Google Inc.
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
#include <string.h>

#include "glimp_common.h"
#include "../renderercommon/tr_common.h"
#include "../sys/sys_local.h"

cvar_t *r_tvMode;
cvar_t *r_tvConsoleMode;
cvar_t *r_tvModeAspect;
cvar_t *r_motionblur;
cvar_t *r_allowResize;
cvar_t *r_centerWindow;
cvar_t *r_sdlDriver;

void (*qglMultiTexCoord2fARB)(GLenum texture, float s, float t);
void (*qglActiveTextureARB)(GLenum texture);
void (*qglClientActiveTextureARB)(GLenum texture);
void (*qglLockArraysEXT)(int, int);
void (*qglUnlockArraysEXT)(void);

#define GLE(ret, name, ...) name##proc *qgl##name;
QGL_1_1_PROCS;
QGL_1_1_FIXED_FUNCTION_PROCS;
QGL_DESKTOP_1_1_PROCS;
QGL_DESKTOP_1_1_FIXED_FUNCTION_PROCS;
QGL_3_0_PROCS;
#undef GLE

int qglMajorVersion, qglMinorVersion;

void GLimp_EndFrame(void) {}

void GLimp_CommonPreInit(void) {
  r_sdlDriver = ri.Cvar_Get("r_sdlDriver", "", CVAR_ROM);
  r_allowResize = ri.Cvar_Get("r_allowResize", "0", CVAR_ARCHIVE | CVAR_LATCH);
  r_centerWindow =
      ri.Cvar_Get("r_centerWindow", "0", CVAR_ARCHIVE | CVAR_LATCH);
  r_tvMode = ri.Cvar_Get("r_tvMode", "-1", CVAR_LATCH | CVAR_ARCHIVE);
  r_tvModeAspect =
      ri.Cvar_Get("r_tvModeAspect", "0", CVAR_LATCH | CVAR_ARCHIVE);
  r_tvConsoleMode =
      ri.Cvar_Get("r_tvConsoleMode", "0", CVAR_LATCH | CVAR_ARCHIVE);
  r_motionblur = ri.Cvar_Get("r_motionblur", "0", CVAR_ARCHIVE | CVAR_LATCH);
  // Ignore r_tvMode.
  R_GetModeInfo(&glConfig.vidWidth, &glConfig.vidHeight, &glConfig.buffWidth,
                &glConfig.buffHeight, &glConfig.windowAspect, -1);
}

void GLimp_CommonPostInit(void) {
#define GLE(ret, name, ...)                                     \
  qgl##name = (name##proc *)GLimp_GetProcAddress("gl" #name);

  GLimp_MakeCurrent();

  // OpenGL 1.0
  GLE(const GLubyte *, GetString, GLenum name)

  const char *version = (const char *)qglGetString(GL_VERSION);

  if (!version) {
    Com_Error(ERR_FATAL, "Failed to get GL_VERSION string.\n");
  } else if (sscanf(version, "%d.%d", &qglMajorVersion, &qglMinorVersion) <
             2) {
    Com_Error(ERR_FATAL, "Failed to read GL Version: %s\n", version);
  }

  if (QGL_VERSION_ATLEAST(1, 1)) {
    QGL_1_1_PROCS;
    QGL_1_1_FIXED_FUNCTION_PROCS;
    QGL_DESKTOP_1_1_PROCS;
    QGL_DESKTOP_1_1_FIXED_FUNCTION_PROCS;
  } else {
    Com_Error(ERR_FATAL, "Unsupported OpenGL Version: %s\n", version);
  }

  if (QGL_VERSION_ATLEAST(3, 0)) {
    QGL_3_0_PROCS;
  }

  glConfig.colorBits = r_colorbits->value;
  qglGetIntegerv(GL_DEPTH_BITS, &glConfig.depthBits);
  qglGetIntegerv(GL_STENCIL_BITS, &glConfig.stencilBits);

  Q_strncpyz(
      glConfig.vendor_string, (const char *)qglGetString(GL_VENDOR),
      sizeof(glConfig.vendor_string));
  Q_strncpyz(
      glConfig.version_string, version, sizeof(glConfig.version_string));
  Q_strncpyz(
      glConfig.renderer_string, (const char *)qglGetString(GL_RENDERER),
      sizeof(glConfig.renderer_string));

  if (glConfig.renderer_string[0] != '\0') {
    size_t n = strlen(glConfig.renderer_string);
    if (glConfig.renderer_string[n - 1] == '\n') {
      glConfig.renderer_string[n - 1] = 0;
    }
  }

#undef GLE
}

void GLimp_Minimize(void) {}

void GLimp_EnableLogging(qboolean enable) {}

void GLimp_LogComment(char *comment) {}

void GLimp_SetGamma(unsigned char red[256], unsigned char green[256],
                    unsigned char blue[256]) {}
