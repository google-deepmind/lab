# Description:
#   Infrastructure support.

package(default_visibility = ["//visibility:public"])

licenses(["restricted"])

cc_library(
    name = "test_srcdir",
    testonly = True,
    srcs = ["test_srcdir.cc"],
    hdrs = ["test_srcdir.h"],
    defines = ["SUPPRESS_COMMANDLINE_FLAGS"],
    deps = ["@com_google_absl//absl/strings"],
)

cc_binary(
    name = "realpath",
    srcs = ["realpath.c"],
    copts = [
        "-std=c99",
        "-D_DEFAULT_SOURCE",
        "-D_POSIX_C_SOURCE",
        "-D_DARWIN_BETTER_REALPATH",
    ],
)
