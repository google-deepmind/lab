# Description:
#   A system for generating maps from "text levels".
#   See docs/text_level.md for details.

licenses(["restricted"])  # GPLv2

cc_library(
    name = "char_grid",
    srcs = ["char_grid.cc"],
    hdrs = ["char_grid.h"],
    visibility = ["//deepmind:__subpackages__"],
    deps = [
        "//deepmind/support:logging",
        "//deepmind/support:str_split",
        "//deepmind/support:string_view",
    ],
)

cc_test(
    name = "char_grid_test",
    size = "small",
    srcs = ["char_grid_test.cc"],
    deps = [
        ":char_grid",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "grid_maze",
    hdrs = ["grid_maze.h"],
)

cc_library(
    name = "text_maze_exporter",
    srcs = ["text_maze_exporter.cc"],
    hdrs = ["text_maze_exporter.h"],
    deps = [
        "//deepmind/level_generation/map_builder",
        "//deepmind/support:logging",
        "@eigen_archive//:eigen",
    ],
)

cc_test(
    name = "text_maze_exporter_test",
    size = "small",
    srcs = ["text_maze_exporter_test.cc"],
    deps = [
        ":text_maze_exporter",
        "@googletest//:gtest_main",
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
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "translate_text_level",
    srcs = ["translate_text_level.cc"],
    hdrs = ["translate_text_level.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":grid_maze",
        ":parse_text_level",
        ":text_maze_exporter",
        "//deepmind/level_generation/map_builder",
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
        "@googletest//:gtest_main",
    ],
)
