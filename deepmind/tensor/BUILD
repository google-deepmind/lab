# Description:
#   Tiny tensor library.

licenses(["restricted"])

cc_library(
    name = "tensor_view",
    srcs = ["tensor_view.cc"],
    hdrs = ["tensor_view.h"],
    visibility = ["//visibility:public"],
    deps = ["@eigen_archive//:eigen"],
)

cc_test(
    name = "tensor_view_test",
    size = "small",
    srcs = ["tensor_view_test.cc"],
    deps = [
        ":tensor_view",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "lua_tensor",
    srcs = ["lua_tensor.cc"],
    hdrs = ["lua_tensor.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":tensor_view",
        "//:file_reader_types",
        "//deepmind/engine:lua_random",
        "//deepmind/lua",
        "//deepmind/lua:bind",
        "//deepmind/lua:call",
        "//deepmind/lua:class",
        "//deepmind/lua:push",
        "//deepmind/lua:read",
        "//deepmind/lua:table_ref",
        "//deepmind/util:file_reader",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "lua_tensor_test",
    size = "small",
    srcs = ["lua_tensor_test.cc"],
    deps = [
        ":lua_tensor",
        "//deepmind/lua:bind",
        "//deepmind/lua:call",
        "//deepmind/lua:n_results_or_test_util",
        "//deepmind/lua:push_script",
        "//deepmind/lua:read",
        "//deepmind/lua:table_ref",
        "//deepmind/lua:vm",
        "//deepmind/util:default_read_only_file_system",
        "@com_google_googletest//:gtest_main",
    ],
)
