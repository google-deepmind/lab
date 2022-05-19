# Description:
#   A collection of libraries for interacting with Lua.

package(default_visibility = ["//visibility:public"])

licenses(["restricted"])

cc_library(
    name = "lua",
    hdrs = ["lua.h"],
    deps = ["@lua_archive//:lua"],
)

cc_library(
    name = "vm",
    srcs = ["vm.cc"],
    hdrs = ["vm.h"],
    deps = [
        ":lua",
        "@com_google_absl//absl/container:flat_hash_map",
    ],
)

cc_library(
    name = "vm_test_util",
    testonly = True,
    hdrs = ["vm_test_util.h"],
    deps = [
        ":vm",
        "@com_google_absl//absl/log:check",
        "@com_google_googletest//:gtest",
    ],
)

cc_test(
    name = "vm_test",
    size = "small",
    srcs = ["vm_test.cc"],
    data = ["vm_test_module.lua"],
    deps = [
        ":call",
        ":n_results_or",
        ":n_results_or_test_util",
        ":push_script",
        ":read",
        ":table_ref",
        ":vm",
        "//deepmind/support:test_srcdir",
        "@com_google_googletest//:gtest_main",
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
        "@com_google_googletest//:gtest",
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
        "@com_google_googletest//:gtest_main",
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
        "@com_google_absl//absl/log:check",
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
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "push",
    hdrs = ["push.h"],
    deps = [
        ":lua",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:span",
        "@com_google_absl//absl/types:variant",
    ],
)

cc_test(
    name = "push_test",
    size = "small",
    srcs = ["push_test.cc"],
    deps = [
        ":push",
        ":vm_test_util",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:span",
        "@com_google_googletest//:gtest_main",
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
        "@com_google_absl//absl/strings",
    ],
)

cc_library(
    name = "read",
    hdrs = ["read.h"],
    deps = [
        ":lua",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:span",
        "@com_google_absl//absl/types:variant",
    ],
)

cc_test(
    name = "read_test",
    size = "small",
    srcs = ["read_test.cc"],
    deps = [
        ":push",
        ":read",
        ":vm_test_util",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:span",
        "@com_google_absl//absl/types:variant",
        "@com_google_googletest//:gtest_main",
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
        "@com_google_absl//absl/log:check",
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
        "@com_google_googletest//:gtest_main",
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
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/strings",
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
        ":n_results_or_test_util",
        ":push_script",
        ":read",
        ":table_ref",
        ":vm_test_util",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "stack_resetter",
    hdrs = ["stack_resetter.h"],
    deps = [":lua"],
)

cc_test(
    name = "stack_resetter_test",
    size = "small",
    srcs = ["stack_resetter_test.cc"],
    deps = [
        ":call",
        ":n_results_or",
        ":n_results_or_test_util",
        ":push",
        ":push_script",
        ":read",
        ":stack_resetter",
        ":vm_test_util",
        "@com_google_googletest//:gtest_main",
    ],
)
