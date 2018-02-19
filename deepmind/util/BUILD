# Description:
#   General utilities.

licenses(["restricted"])  # GPLv2

# Library for running external executables.
cc_library(
    name = "run_executable",
    srcs = ["run_executable.cc"],
    hdrs = ["run_executable.h"],
    visibility = ["//deepmind:__subpackages__"],
    deps = [
        "//deepmind/support:logging",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "run_executable_test",
    srcs = ["run_executable_test.cc"],
    deps = [
        ":run_executable",
        "@com_google_googletest//:gtest_main",
    ],
)

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

# Generic smoothing routines.
cc_library(
    name = "smoother",
    hdrs = ["smoother.h"],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "smoother_test",
    srcs = ["smoother_test.cc"],
    deps = [
        ":smoother",
        "@com_google_googletest//:gtest_main",
        "@eigen_archive//:eigen",
    ],
)
