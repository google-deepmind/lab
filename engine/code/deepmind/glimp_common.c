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

#include <stdlib.h>
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
  R_GetModeInfo(&glConfig.vidWidth, &glConfig.vidHeight, &glConfig.windowAspect,
                -1);
}

void GLimp_CommonPostInit(void) {
  GLimp_MakeCurrent();
  glConfig.colorBits = r_colorbits->value;
  glGetIntegerv(GL_DEPTH_BITS, &glConfig.depthBits);
  glGetIntegerv(GL_STENCIL_BITS, &glConfig.stencilBits);
}

void GLimp_Minimize(void) {}

void GLimp_EnableLogging(qboolean enable) {}

void GLimp_LogComment(char *comment) {}

void GLimp_SetGamma(unsigned char red[256], unsigned char green[256],
                    unsigned char blue[256]) {}
