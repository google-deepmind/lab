# Description:
#   A collection of libraries for interacting with Lua.

licenses(["restricted"])  # GPLv2

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "lua",
    hdrs = ["lua.h"],
    deps = ["@lua_system//:lua"],
)

cc_library(
    name = "vm",
    srcs = ["vm.cc"],
    hdrs = ["vm.h"],
    deps = [":lua"],
)

cc_library(
    name = "vm_test_util",
    testonly = True,
    hdrs = ["vm_test_util.h"],
    deps = [
        ":vm",
        "//deepmind/support:logging",
        "@googletest//:gtest",
    ],
)

cc_test(
    name = "vm_test",
    size = "small",
    srcs = ["vm_test.cc"],
    data = ["vm_test_module.lua"],
    deps = [
        ":call",
        ":n_results_or_test_util",
        ":push_script",
        ":read",
        ":table_ref",
        ":vm",
        "//deepmind/support:logging",
        "//deepmind/support:test_srcdir",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "n_results_or",
    hdrs = ["n_results_or.h"],
)

cc_library(
    name = "n_results_or_test_util",
    testonly = True,
    hdrs = ["n_results_or_test_util.h"],
    deps = [
        ":n_results_or",
        "@googletest//:gtest",
    ],
)

cc_library(
    name = "bind",
    hdrs = ["bind.h"],
    deps = [
        ":lua",
        ":n_results_or",
    ],
)

cc_test(
    name = "bind_test",
    size = "small",
    srcs = ["bind_test.cc"],
    deps = [
        ":bind",
        ":push",
        ":read",
        ":vm_test_util",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "call",
    srcs = ["call.cc"],
    hdrs = ["call.h"],
    deps = [
        ":lua",
        ":n_results_or",
        ":push",
        ":read",
        "//deepmind/support:logging",
    ],
)

cc_test(
    name = "call_test",
    size = "small",
    srcs = ["call_test.cc"],
    deps = [
        ":bind",
        ":call",
        ":n_results_or_test_util",
        ":push",
        ":push_script",
        ":read",
        ":vm_test_util",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "push",
    hdrs = ["push.h"],
    deps = [":lua"],
)

cc_test(
    name = "push_test",
    size = "small",
    srcs = ["push_test.cc"],
    deps = [
        ":push",
        ":vm_test_util",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "push_script",
    srcs = ["push_script.cc"],
    hdrs = ["push_script.h"],
    deps = [
        ":lua",
        ":n_results_or",
        ":read",
    ],
)

cc_library(
    name = "read",
    hdrs = ["read.h"],
    deps = [":lua"],
)

cc_test(
    name = "read_test",
    size = "small",
    srcs = ["read_test.cc"],
    deps = [
        ":push",
        ":read",
        ":vm_test_util",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "table_ref",
    srcs = ["table_ref.cc"],
    hdrs = ["table_ref.h"],
    deps = [
        ":lua",
        ":push",
        ":read",
        "//deepmind/support:logging",
    ],
)

cc_test(
    name = "table_ref_test",
    size = "small",
    srcs = ["table_ref_test.cc"],
    deps = [
        ":call",
        ":class",
        ":n_results_or_test_util",
        ":push_script",
        ":read",
        ":table_ref",
        ":vm_test_util",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "class",
    hdrs = ["class.h"],
    deps = [
        ":lua",
        ":n_results_or",
        ":push",
        ":read",
        "//deepmind/support:logging",
    ],
)

cc_test(
    name = "class_test",
    size = "small",
    srcs = ["class_test.cc"],
    deps = [
        ":bind",
        ":call",
        ":class",
        ":push_script",
        ":read",
        ":table_ref",
        ":vm_test_util",
        "@googletest//:gtest_main",
    ],
)
