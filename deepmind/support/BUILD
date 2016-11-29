# Description:
#   Infrastructure support.

licenses(["restricted"])  # GPLv2

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "logging",
    hdrs = ["logging.h"],
)

cc_library(
    name = "str_join",
    hdrs = ["str_join.h"],
)

cc_library(
    name = "str_split",
    hdrs = ["str_split.h"],
)

cc_library(
    name = "str_cat",
    hdrs = ["str_cat.h"],
    deps = [":string_view"],
)

cc_library(
    name = "string_view",
    hdrs = [
        "string_view.h",
        "string_view_utils.h",
    ],
)

cc_library(
    name = "stringprintf",
    srcs = ["stringprintf.cc"],
    hdrs = ["stringprintf.h"],
)

cc_library(
    name = "test_srcdir",
    testonly = True,
    srcs = ["test_srcdir.cc"],
    hdrs = ["test_srcdir.h"],
    defines = ["SUPPRESS_COMMANDLINE_FLAGS"],
)
