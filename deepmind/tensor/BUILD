# Description:
#   Tiny tensor library.

licenses(["restricted"])  # GPLv2

cc_library(
    name = "tensor_view",
    srcs = ["tensor_view.cc"],
    hdrs = ["tensor_view.h"],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "tensor_view_test",
    size = "small",
    srcs = ["tensor_view_test.cc"],
    deps = [
        ":tensor_view",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "lua_tensor",
    srcs = ["lua_tensor.cc"],
    hdrs = ["lua_tensor.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":tensor_view",
        "//deepmind/lua",
        "//deepmind/lua:bind",
        "//deepmind/lua:call",
        "//deepmind/lua:class",
        "//deepmind/lua:push",
        "//deepmind/lua:read",
        "//deepmind/lua:table_ref",
    ],
)

cc_test(
    name = "lua_tensor_test",
    size = "small",
    srcs = ["lua_tensor_test.cc"],
    deps = [
        ":lua_tensor",
        "//deepmind/lua:call",
        "//deepmind/lua:n_results_or_test_util",
        "//deepmind/lua:push_script",
        "//deepmind/lua:read",
        "//deepmind/lua:table_ref",
        "//deepmind/lua:vm",
        "@googletest//:gtest_main",
    ],
)
