# Description:
#   Translation from text levels into .map format.
#   See deepmind/docs/developers/creating_levels/text_level.md.

licenses(["restricted"])

cc_library(
    name = "char_grid",
    srcs = ["char_grid.cc"],
    hdrs = ["char_grid.h"],
    visibility = ["//deepmind:__subpackages__"],
    deps = [
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "char_grid_test",
    size = "small",
    srcs = ["char_grid_test.cc"],
    deps = [
        ":char_grid",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "grid_maze",
    hdrs = ["grid_maze.h"],
)

cc_library(
    name = "text_level_exporter",
    srcs = ["text_level_exporter.cc"],
    hdrs = [
        "text_level_exporter.h",
        "text_level_settings.h",
    ],
    deps = [
        "//deepmind/level_generation/map_builder",
        "@com_google_absl//absl/strings",
        "@eigen_archive//:eigen",
    ],
)

cc_test(
    name = "text_level_exporter_test",
    size = "small",
    srcs = ["text_level_exporter_test.cc"],
    deps = [
        ":text_level_exporter",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "parse_text_level",
    srcs = ["parse_text_level.cc"],
    hdrs = ["parse_text_level.h"],
    deps = [
        ":char_grid",
        ":grid_maze",
    ],
)

cc_test(
    name = "parse_text_level_test",
    size = "small",
    srcs = ["parse_text_level_test.cc"],
    deps = [
        ":parse_text_level",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "translate_text_level",
    srcs = ["translate_text_level.cc"],
    hdrs = [
        "text_level_settings.h",
        "translate_text_level.h",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":grid_maze",
        ":parse_text_level",
        ":text_level_exporter",
        "//deepmind/level_generation/map_builder",
        "@com_google_absl//absl/strings",
        "@eigen_archive//:eigen",
    ],
)

cc_test(
    name = "translate_text_level_test",
    size = "small",
    srcs = ["translate_text_level_test.cc"],
    data = [
        "translate_text_level_test_libc++.golden_output",
        "translate_text_level_test_libstdc++.golden_output",
    ],
    deps = [
        ":translate_text_level",
        "//deepmind/support:test_srcdir",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/strings",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "lua_bindings",
    srcs = ["lua_bindings.cc"],
    hdrs = ["lua_bindings.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":translate_text_level",
        "//deepmind/lua",
        "//deepmind/lua:call",
        "//deepmind/lua:class",
        "//deepmind/lua:n_results_or",
        "//deepmind/lua:push",
        "//deepmind/lua:read",
        "//deepmind/lua:table_ref",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/log",
    ],
)

cc_test(
    name = "lua_bindings_test",
    size = "small",
    srcs = ["lua_bindings_test.cc"],
    deps = [
        ":lua_bindings",
        ":translate_text_level",
        "//deepmind/lua:call",
        "//deepmind/lua:push_script",
        "//deepmind/lua:vm",
        "@com_google_googletest//:gtest_main",
    ],
)
