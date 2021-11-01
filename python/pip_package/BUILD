# Description:
#  A tool for building the Deepmind Lab pip package.

load("@python_system//:defs.bzl", "PY_TAGS")
load("@//:wheel_defs.bzl", "py_wheel")

py_wheel(
    name = "deepmind_lab_wheel",
    srcs = [":__init__.py"],
    abi = PY_TAGS.abi,
    description_file = ":pip_description.txt",
    distribution = "deepmind-lab",
    platform = PY_TAGS.platform,
    python_tag = PY_TAGS.interpreter,
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
