#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

#include <OpenGL/OpenGL.h>
#include <OpenGL/gl3.h>

#include "glimp_common.h"

static CGLContextObj context;

void GLimp_MakeCurrent(void) {
}

void GLimp_Init(void) {
  CGLPixelFormatObj pix;
  GLint npix;
  int attribs[] = {
    kCGLPFAAccelerated,     // no software rendering
    kCGLPFAOpenGLProfile,
    kCGLOGLPVersion_Legacy,
    0
  };

  GLimp_CommonPreInit();

  // NOTE: in headless mode there is no GUI, hence output to console instead of message boxes

  if (CGLChoosePixelFormat((CGLPixelFormatAttribute*)attribs, &pix, &npix) != kCGLNoError) {
    // Sys_Error("GLimp_Init - choose pixel format error!\n");
    printf("GLimp_Init - choose pixel format error!\n");
    exit(1);
  }
  if (CGLCreateContext(pix, NULL, &context) != kCGLNoError) {
    // Sys_Error("GLimp_Init - create context error!\n");
    printf("GLimp_Init - create context error!\n");
    exit(1);
  }
  if (CGLSetCurrentContext(context) != kCGLNoError) {
    // Sys_Error("GLimp_Init - set current context error!");
    printf("GLimp_Init - set current context error!\n");
    exit(1);
  }
  CGLDestroyPixelFormat(pix);

  printf("Renderer: %s\nVersion: %s\n", glGetString(GL_RENDERER), glGetString(GL_VERSION));

  GLimp_CommonPostInit();
}

void* GLimp_GetProcAddress(const char* func) {
  return dlsym(RTLD_SELF, func);
}

void GLimp_Shutdown(void) {
  CGLSetCurrentContext(NULL);
  CGLDestroyContext(context);
}
