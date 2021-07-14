# Description:
#   Tree provides utilities for working with nested data structures.
#   The tree package used to support Bazel, but no longer does so.
#   This BUILD file is adapted from the last official version.

cc_binary(
    name = "tree/_tree.so",
    srcs = [
        "tree/tree.cc",
        "tree/tree.h",
    ],
    linkshared = 1,
    linkstatic = 1,
    deps = [
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
        "@pybind11_archive//:pybind11",
        "@python_system//:python_headers",
    ],
)

py_library(
    name = "tree",
    srcs = ["tree/__init__.py"],
    data = [":tree/_tree.so"],
    srcs_version = "PY2AND3",
    visibility = ["@dm_env_archive//:__pkg__"],
    deps = [":sequence"],
)

py_library(
    name = "sequence",
    srcs = ["tree/sequence.py"],
    data = [":tree/_tree.so"],
    srcs_version = "PY3",
)
