# Description:
#   Build rule for EGL dependencies.

licenses(["notice"])  # Khronos

exports_files(["LICENSE"])

cc_library(
    name = "EGL_headers",
    srcs = [
        "EGL/egl.h",
        "EGL/eglext.h",
        "EGL/eglplatform.h",
        "KHR/khrplatform.h",
    ],
    defines = ["MESA_EGL_NO_X11_HEADERS"],
    includes = ["."],
    visibility = ["//visibility:public"],
)
