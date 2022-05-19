# Description:
#   Procedural modelling library.

licenses(["restricted"])

cc_library(
    name = "geometry_util",
    srcs = ["geometry_util.cc"],
    hdrs = ["geometry_util.h"],
    deps = [
        ":transform",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/strings",
        "@eigen_archive//:eigen",
    ],
)

cc_library(
    name = "geometry_cone",
    srcs = ["geometry_cone.cc"],
    hdrs = ["geometry_cone.h"],
    deps = [
        ":geometry_util",
        ":model",
        ":transform",
        "@com_google_absl//absl/log:check",
        "@eigen_archive//:eigen",
    ],
)

cc_library(
    name = "geometry_cube",
    srcs = ["geometry_cube.cc"],
    hdrs = ["geometry_cube.h"],
    deps = [
        ":geometry_util",
        ":model",
        ":transform",
        "@com_google_absl//absl/log:check",
        "@eigen_archive//:eigen",
    ],
)

cc_library(
    name = "geometry_cylinder",
    srcs = ["geometry_cylinder.cc"],
    hdrs = ["geometry_cylinder.h"],
    deps = [
        ":geometry_util",
        ":model",
        ":transform",
        "@com_google_absl//absl/log:check",
        "@eigen_archive//:eigen",
    ],
)

cc_library(
    name = "geometry_sphere",
    srcs = ["geometry_sphere.cc"],
    hdrs = ["geometry_sphere.h"],
    deps = [
        ":geometry_util",
        ":model",
        ":transform",
        "@com_google_absl//absl/log:check",
        "@eigen_archive//:eigen",
    ],
)

cc_test(
    name = "geometry_test",
    size = "small",
    srcs = ["geometry_test.cc"],
    deps = [
        ":geometry_cone",
        ":geometry_cube",
        ":geometry_cylinder",
        ":geometry_sphere",
        ":geometry_util",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "model",
    hdrs = ["model.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":transform",
        "@com_google_absl//absl/container:flat_hash_map",
    ],
)

cc_library(
    name = "model_lua",
    srcs = ["model_lua.cc"],
    hdrs = ["model_lua.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":model",
        ":transform",
        ":transform_lua",
        "//deepmind/lua",
        "//deepmind/lua:push",
        "//deepmind/lua:read",
        "//deepmind/lua:table_ref",
        "//deepmind/tensor:lua_tensor",
        "//deepmind/tensor:tensor_view",
        "@com_google_absl//absl/log",
    ],
)

cc_test(
    name = "model_lua_test",
    size = "small",
    srcs = ["model_lua_test.cc"],
    deps = [
        ":geometry_util",
        ":model_lua",
        "//deepmind/lua:call",
        "//deepmind/lua:n_results_or_test_util",
        "//deepmind/lua:push_script",
        "//deepmind/lua:vm",
        "//deepmind/support:test_srcdir",
        "//deepmind/tensor:lua_tensor",
        "//deepmind/util:default_read_only_file_system",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "model_getters",
    srcs = ["model_getters.cc"],
    hdrs = ["model_getters.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":model",
        ":transform",
        "//deepmind/include:context_hdrs",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/log:check",
    ],
)

cc_library(
    name = "model_setters",
    srcs = ["model_setters.cc"],
    hdrs = ["model_setters.h"],
    deps = [
        ":model",
        ":transform",
        "//deepmind/include:context_hdrs",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
    ],
)

cc_library(
    name = "model_util",
    srcs = ["model_util.cc"],
    hdrs = ["model_util.h"],
    deps = [
        ":model",
        "@eigen_archive//:eigen",
    ],
)

cc_library(
    name = "transform",
    hdrs = ["transform.h"],
    deps = ["@eigen_archive//:eigen"],
)

cc_library(
    name = "transform_lua",
    srcs = ["transform_lua.cc"],
    hdrs = ["transform_lua.h"],
    deps = [
        ":transform",
        "//deepmind/lua",
        "//deepmind/lua:read",
        "//deepmind/tensor:lua_tensor",
        "//deepmind/tensor:tensor_view",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
    ],
)

cc_test(
    name = "transform_lua_test",
    size = "small",
    srcs = ["transform_lua_test.cc"],
    deps = [
        ":geometry_util",
        ":transform_lua",
        "//deepmind/lua:call",
        "//deepmind/lua:n_results_or_test_util",
        "//deepmind/lua:push_script",
        "//deepmind/lua:vm",
        "//deepmind/support:test_srcdir",
        "//deepmind/tensor:lua_tensor",
        "@com_google_googletest//:gtest_main",
        "@eigen_archive//:eigen",
    ],
)

cc_library(
    name = "lua_model",
    srcs = ["lua_model.cc"],
    hdrs = ["lua_model.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":geometry_cone",
        ":geometry_cube",
        ":geometry_cylinder",
        ":geometry_sphere",
        ":geometry_util",
        ":model",
        ":model_getters",
        ":model_lua",
        ":model_setters",
        ":model_util",
        ":transform",
        ":transform_lua",
        "//deepmind/include:context_hdrs",
        "//deepmind/lua",
        "//deepmind/lua:class",
        "//deepmind/lua:n_results_or",
        "//deepmind/lua:push",
        "//deepmind/lua:read",
        "//deepmind/lua:table_ref",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/meta:type_traits",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "lua_model_test",
    size = "small",
    srcs = ["lua_model_test.cc"],
    deps = [
        ":geometry_util",
        ":lua_model",
        ":model_lua",
        "//deepmind/engine:callbacks",
        "//deepmind/include:context_hdrs",
        "//deepmind/lua:bind",
        "//deepmind/lua:call",
        "//deepmind/lua:n_results_or_test_util",
        "//deepmind/lua:push_script",
        "//deepmind/lua:vm",
        "//deepmind/support:test_srcdir",
        "//deepmind/tensor:lua_tensor",
        "@com_google_absl//absl/strings",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "lua_transform",
    srcs = ["lua_transform.cc"],
    hdrs = ["lua_transform.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":geometry_util",
        ":transform_lua",
        "//deepmind/lua",
        "//deepmind/lua:bind",
        "//deepmind/lua:n_results_or",
        "//deepmind/lua:push",
        "//deepmind/lua:read",
        "//deepmind/lua:table_ref",
    ],
)

cc_test(
    name = "lua_transform_test",
    size = "small",
    srcs = ["lua_transform_test.cc"],
    deps = [
        ":geometry_util",
        ":lua_transform",
        ":transform_lua",
        "//deepmind/lua:bind",
        "//deepmind/lua:call",
        "//deepmind/lua:n_results_or_test_util",
        "//deepmind/lua:push_script",
        "//deepmind/lua:vm",
        "//deepmind/support:test_srcdir",
        "//deepmind/tensor:lua_tensor",
        "@com_google_googletest//:gtest_main",
    ],
)
