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
// Must include tr_common.h before opengl.
#include <GL/osmesa.h>
#include "../sys/sys_local.h"
#include "glimp_common.h"

static OSMesaContext osmesa_ctx;
static GLubyte* osmesa_frame_buffer;

void GLimp_MakeCurrent(void) {
  if (!OSMesaMakeCurrent(osmesa_ctx, osmesa_frame_buffer, GL_UNSIGNED_BYTE,
                         glConfig.buffWidth, glConfig.buffHeight)) {
    Sys_Error("GLimp_MakeCurrent - Failed!");
  }
}

void GLimp_Init(qboolean coreContext) {
  r_colorbits->value = 16;
  r_depthbits->value = 24;
  GLimp_CommonPreInit();
  /* Create an RGBA-mode context */
  osmesa_ctx =
      OSMesaCreateContextExt(OSMESA_RGBA, r_depthbits->value, 0, 0, NULL);
  if (!osmesa_ctx) {
    Sys_Error("OSMesaCreateContext failed!");
  }

  /* Allocate the image buffer */
  osmesa_frame_buffer =
      calloc(glConfig.buffWidth * glConfig.buffHeight * 4, sizeof(GLubyte));
  if (!osmesa_frame_buffer) {
    Sys_Error("Alloc image buffer failed!");
  }

  // Force draw buffer to GL_FRONT, as OSMesa doesn't have a back buffer to
  // render to.
  ri.Cvar_Set("r_drawBuffer", "GL_FRONT");

  GLimp_CommonPostInit();
}

void* GLimp_GetProcAddress(const char* func) {
  return OSMesaGetProcAddress(func);
}

void GLimp_Shutdown(void) {
  OSMesaDestroyContext(osmesa_ctx);
  free(osmesa_frame_buffer);
  osmesa_frame_buffer = NULL;
}
