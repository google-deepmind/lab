# Description:
#   Bazel BUILD rules for "dm_env" (https://github.com/deepmind/dm_env),
#   a Python API for reinforcement learning environments.

py_library(
    name = "dm_env",
    srcs = [
        "dm_env/__init__.py",
        "dm_env/_environment.py",
        "dm_env/_metadata.py",
        "dm_env/specs.py",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "@funcsigs_archive//:funcsigs",
        "@six_archive//:six",
    ],
)

py_library(
    name = "test_utils",
    srcs = [
        "dm_env/_abstract_test_mixin.py",
        "dm_env/test_utils.py",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":dm_env",
        "@com_google_absl_py//absl/testing:absltest",
        "@tree_archive//:tree",
    ],
)
