# Description:
#  A tool for building the Deepmind Lab pip package.

load("@python_system//:defs.bzl", "py_tags")
load("@//:wheel_defs.bzl", "py_wheel")

py_wheel(
    name = "deepmind_lab_wheel",
    srcs = [":__init__.py"],
    abi = select({
        "@bazel_tools//tools/python:PY2": py_tags("py2").abi,
        "@bazel_tools//tools/python:PY3": py_tags("py3").abi,
    }),
    description_file = ":pip_description.txt",
    distribution = "deepmind-lab",
    platform = select({
        "@bazel_tools//tools/python:PY2": py_tags("py2").platform,
        "@bazel_tools//tools/python:PY3": py_tags("py3").platform,
    }),
    python_tag = select({
        "@bazel_tools//tools/python:PY2": py_tags("py2").interpreter,
        "@bazel_tools//tools/python:PY3": py_tags("py3").interpreter,
    }),
    version = "1.0",
    deps = [
        "//:deepmind_lab.so",
        "//:dmenv_module",
    ],
)

package(default_visibility = ["//visibility:private"])

exports_files(["__init__.py"])

sh_binary(
    name = "build_pip_package",
    srcs = ["build_pip_package.sh"],
    data = [
        "README.md",
        "__init__.py",
        "setup.py",
        "//:deepmind_lab.so",
    ],
)
