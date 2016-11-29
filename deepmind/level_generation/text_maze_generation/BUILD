# Description:
#   Tools for generation of text levels that can be used by the text level converter.

licenses(["restricted"])  # GPLv2

cc_library(
    name = "text_maze",
    srcs = ["text_maze.cc"],
    hdrs = ["text_maze.h"],
    visibility = ["//deepmind/engine:__pkg__"],
)

cc_test(
    name = "text_maze_test",
    size = "small",
    srcs = ["text_maze_test.cc"],
    deps = [
        ":text_maze",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "algorithm",
    srcs = ["algorithm.cc"],
    hdrs = ["algorithm.h"],
    visibility = ["//deepmind/engine:__pkg__"],
    deps = [
        ":flood_fill",
        ":text_maze",
        "//deepmind/level_generation/text_level:char_grid",
    ],
)

cc_test(
    name = "algorithm_test",
    size = "small",
    srcs = ["algorithm_test.cc"],
    deps = [
        ":algorithm",
        ":text_maze",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "flood_fill",
    srcs = ["flood_fill.cc"],
    hdrs = ["flood_fill.h"],
    visibility = ["//deepmind/engine:__pkg__"],
    deps = [":text_maze"],
)
