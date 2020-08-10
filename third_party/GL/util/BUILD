# Description:
#   EGL Utility function required to correctly create and initialize an EGL
#   display on multi-GPU machines.

licenses(["notice"])

cc_library(
    name = "egl_util",
    srcs = ["egl_util.cc"],
    hdrs = ["egl_util.h"],
    visibility = ["//visibility:public"],
    deps = ["//third_party/GL:EGL_headers"],
)

cc_test(
    name = "egl_util_test",
    srcs = ["egl_util_test.cc"],
    deps = [
        ":egl_util",
        "//third_party/GL:EGL_headers",
        "@com_google_googletest//:gtest_main",
    ],
)
