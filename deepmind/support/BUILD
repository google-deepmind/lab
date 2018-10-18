# Description:
#   Infrastructure support.

licenses(["restricted"])  # GPLv2

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "logging",
    hdrs = ["logging.h"],
)

cc_library(
    name = "stringprintf",
    srcs = ["stringprintf.cc"],
    hdrs = ["stringprintf.h"],
)

cc_library(
    name = "test_srcdir",
    testonly = True,
    srcs = ["test_srcdir.cc"],
    hdrs = ["test_srcdir.h"],
    defines = ["SUPPRESS_COMMANDLINE_FLAGS"],
)

cc_binary(
    name = "realpath",
    srcs = ["realpath.c"],
    copts = [
        "-std=c99",
        "-D_DEFAULT_SOURCE",
        "-D_POSIX_C_SOURCE",
    ],
)
