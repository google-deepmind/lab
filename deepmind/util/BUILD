# Description:
#   General utilities.

licenses(["restricted"])

# Library for running external executables.
cc_library(
    name = "run_executable",
    srcs = ["run_executable.cc"],
    hdrs = ["run_executable.h"],
    visibility = ["//deepmind:__subpackages__"],
    deps = [
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
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

# Default readonly file system.
cc_library(
    name = "default_read_only_file_system",
    srcs = ["default_read_only_file_system.cc"],
    hdrs = ["default_read_only_file_system.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//:file_reader_types",
        "@com_google_absl//absl/strings",
    ],
)

# Library for read-only file operations.
cc_library(
    name = "file_reader",
    srcs = ["file_reader.cc"],
    hdrs = ["file_reader.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//:file_reader_types",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "file_reader_test",
    srcs = ["file_reader_test.cc"],
    deps = [
        ":default_read_only_file_system",
        ":file_reader",
        ":files",
        "//:file_reader_types",
        "@com_google_absl//absl/log:check",
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
