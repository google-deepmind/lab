# Description:
#   Hooks and callbacks to enable gameplay modification.

licenses(["restricted"])  # GPLv2

cc_library(
    name = "context",
    srcs = ["context.cc"],
    hdrs = ["context.h"],
    visibility = ["//:__pkg__"],
    deps = [
        ":lua_maze_generation",
        ":lua_random",
        ":lua_text_level_maker",
        "//deepmind/include:context_headers",
        "//deepmind/lua",
        "//deepmind/lua:bind",
        "//deepmind/lua:call",
        "//deepmind/lua:class",
        "//deepmind/lua:n_results_or",
        "//deepmind/lua:push_script",
        "//deepmind/lua:read",
        "//deepmind/lua:table_ref",
        "//deepmind/lua:vm",
        "//deepmind/tensor:lua_tensor",
    ],
)

cc_library(
    name = "callbacks",
    srcs = ["callbacks.cc"],
    data = ["//:non_pk3_assets"],
    visibility = ["//:__pkg__"],
    deps = [
        ":context",
        ":lua_maze_generation",
        ":lua_random",
        ":lua_text_level_maker",
        "//deepmind/level_generation/text_level:lua_bindings",
        "//deepmind/lua:vm",
        "//deepmind/tensor:lua_tensor",
    ],
)

cc_test(
    name = "callbacks_test",
    size = "small",
    srcs = ["callbacks_test.cc"],
    deps = [
        ":callbacks",
        "//deepmind/include:context_headers",
        "//deepmind/support:test_srcdir",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "lua_maze_generation",
    srcs = ["lua_maze_generation.cc"],
    hdrs = ["lua_maze_generation.h"],
    deps = [
        "//deepmind/level_generation/text_level:char_grid",
        "//deepmind/level_generation/text_maze_generation:algorithm",
        "//deepmind/level_generation/text_maze_generation:text_maze",
        "//deepmind/lua",
        "//deepmind/lua:bind",
        "//deepmind/lua:call",
        "//deepmind/lua:class",
        "//deepmind/lua:n_results_or",
        "//deepmind/lua:push",
        "//deepmind/lua:read",
        "//deepmind/lua:table_ref",
    ],
)

cc_test(
    name = "lua_maze_generation_test",
    size = "small",
    srcs = ["lua_maze_generation_test.cc"],
    deps = [
        ":lua_maze_generation",
        "//deepmind/lua:call",
        "//deepmind/lua:n_results_or_test_util",
        "//deepmind/lua:push_script",
        "//deepmind/lua:vm_test_util",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "lua_random",
    srcs = ["lua_random.cc"],
    hdrs = ["lua_random.h"],
    deps = [
        "//deepmind/lua",
        "//deepmind/lua:class",
        "//deepmind/lua:n_results_or",
        "//deepmind/lua:push",
        "//deepmind/lua:read",
    ],
)

cc_test(
    name = "lua_random_test",
    size = "small",
    srcs = ["lua_random_test.cc"],
    deps = [
        ":lua_random",
        "//deepmind/lua:call",
        "//deepmind/lua:n_results_or_test_util",
        "//deepmind/lua:push_script",
        "//deepmind/lua:vm_test_util",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "lua_text_level_maker",
    srcs = ["lua_text_level_maker.cc"],
    hdrs = ["lua_text_level_maker.h"],
    visibility = ["//deepmind:__subpackages__"],
    deps = [
        ":lua_random",
        "//deepmind/level_generation:compile_map",
        "//deepmind/level_generation/text_level:lua_bindings",
        "//deepmind/lua",
        "//deepmind/lua:class",
        "//deepmind/lua:n_results_or",
        "//deepmind/lua:push",
        "//deepmind/lua:read",
        "//deepmind/lua:table_ref",
    ],
)
