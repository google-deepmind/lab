#include <stdio.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl3.h>

static CGLContextObj context;
//static GLuint framebuffer;
//static GLuint colorRenderbuffer;
//static GLuint depthRenderbuffer;

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

  if (CGLChoosePixelFormat((CGLPixelFormatAttribute *)attribs, &pix, &npix) != kCGLNoError) {
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

  /*
  glGenFramebuffers(1, &framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

  glGenRenderbuffers(1, &colorRenderbuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, glConfig.vidWidth, glConfig.vidHeight);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorRenderbuffer);

  glGenRenderbuffers(1, &depthRenderbuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, glConfig.vidWidth, glConfig.vidHeight);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    // Sys_Error("GLimp_Init - framebuffer not ready!");
    printf("GLimp_Init - framebuffer not ready!\n");
    exit(1);
  }
  */

  GLimp_CommonPostInit();
}

void GLimp_Shutdown(void) {
  /*
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  glDeleteRenderbuffers(1, &colorRenderbuffer);
  glDeleteRenderbuffers(1, &depthRenderbuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDeleteFramebuffers(1, &framebuffer);
  */
  CGLSetCurrentContext(NULL);
  CGLDestroyContext(context);
}
