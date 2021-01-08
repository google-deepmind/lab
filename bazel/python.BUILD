# Description:
#   Build rule for Python and Numpy.
#   This rule works for Debian and Ubuntu, and for MacOS. Other platforms might
#   keep the headers in different places.

config_setting(
    name = "is_linux",
    constraint_values = ["@platforms//os:linux"],
)

config_setting(
    name = "is_macos",
    constraint_values = ["@platforms//os:macos"],
)

cc_library(
    name = "python_headers_linux",
    hdrs = select(
        {
            "@bazel_tools//tools/python:PY2": glob(["usr/include/python2.7/*.h"]),
            "@bazel_tools//tools/python:PY3": glob(["usr/include/python3.8/**/*.h"]),
        },
        no_match_error = "Internal error, Python version should be one of PY2 or PY3",
    ),
    includes = select(
        {
            "@bazel_tools//tools/python:PY2": ["usr/include/python2.7"],
            "@bazel_tools//tools/python:PY3": ["usr/include/python3.8"],
        },
        no_match_error = "Internal error, Python version should be one of PY2 or PY3",
    ),
)

cc_library(
    name = "python_headers_macos",
    hdrs = select(
        {
            "@bazel_tools//tools/python:PY2": glob(["Library/Frameworks/Python.framework/Versions/2.7/Headers/*.h"]),
            "@bazel_tools//tools/python:PY3": glob(["Library/Frameworks/Python.framework/Versions/3.9/Headers/**/*.h"]),
        },
        no_match_error = "Internal error, Python version should be one of PY2 or PY3",
    ),
    includes = select(
        {
            "@bazel_tools//tools/python:PY2": ["Library/Frameworks/Python.framework/Versions/2.7/Headers"],
            "@bazel_tools//tools/python:PY3": ["Library/Frameworks/Python.framework/Versions/3.9/Headers"],
        },
        no_match_error = "Internal error, Python version should be one of PY2 or PY3",
    ),
)

alias(
    name = "python_headers",
    actual = select(
        {
            ":is_linux": ":python_headers_linux",
            ":is_macos": ":python_headers_macos",
        },
        no_match_error = "Unsupported platform; only Linux and MacOS are supported.",
    ),
    visibility = ["//visibility:public"],
)

alias(
    name = "python",
    actual = ":python_headers",
    visibility = ["//visibility:public"],
)
