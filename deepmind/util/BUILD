# Description:
#   General utilities.

licenses(["restricted"])  # GPLv2

# Library for platform-specific file operations.
cc_library(
    name = "files",
    srcs = ["files.cc"],
    hdrs = ["files.h"],
    visibility = ["//visibility:public"],
    deps = ["@com_google_absl//absl/strings"],
)

cc_test(
    name = "files_test",
    srcs = ["files_test.cc"],
    deps = [
        ":files",
        "@com_google_googletest//:gtest_main",
    ],
)
