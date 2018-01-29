# Description:
#  A tool for building the Deepmind Lab pip package.

package(default_visibility = ["//visibility:private"])

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
