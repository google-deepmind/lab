# Description:
#   Hooks and callbacks to enable gameplay modification.

licenses(["restricted"])

cc_library(
    name = "context",
    srcs = ["context.cc"],
    hdrs = ["context.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":context_actions",
        ":context_entities",
        ":context_events",
        ":context_game",
        ":context_observations",
        ":context_pickups",
        ":lua_image",
        ":lua_maze_generation",
        ":lua_random",
        ":lua_text_level_maker",
        "//:file_reader_types",
        "//:level_cache_types",
        "//deepmind/include:context_hdrs",
        "//deepmind/level_generation:compile_map",
        "//deepmind/lua",
        "//deepmind/lua:bind",
        "//deepmind/lua:call",
        "//deepmind/lua:class",
        "//deepmind/lua:n_results_or",
        "//deepmind/lua:push",
        "//deepmind/lua:push_script",
        "//deepmind/lua:read",
        "//deepmind/lua:stack_resetter",
        "//deepmind/lua:table_ref",
        "//deepmind/lua:vm",
        "//deepmind/model_generation:lua_model",
        "//deepmind/model_generation:lua_transform",
        "//deepmind/model_generation:model",
        "//deepmind/model_generation:model_getters",
        "//deepmind/model_generation:model_lua",
        "//deepmind/tensor:lua_tensor",
        "//deepmind/tensor:tensor_view",
        "//third_party/rl_api:env_c_api",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:span",
    ],
)

cc_library(
    name = "context_events",
    srcs = ["context_events.cc"],
    hdrs = ["context_events.h"],
    deps = [
        "//deepmind/lua",
        "//deepmind/lua:class",
        "//deepmind/lua:n_results_or",
        "//deepmind/lua:read",
        "//deepmind/tensor:lua_tensor",
        "//deepmind/tensor:tensor_view",
        "//third_party/rl_api:env_c_api",
        "@com_google_absl//absl/container:node_hash_map",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/meta:type_traits",
    ],
)

cc_library(
    name = "context_game",
    srcs = ["context_game.cc"],
    hdrs = ["context_game.h"],
    deps = [
        "//:file_reader_types",
        "//deepmind/include:context_hdrs",
        "//deepmind/lua",
        "//deepmind/lua:class",
        "//deepmind/lua:n_results_or",
        "//deepmind/lua:push",
        "//deepmind/lua:read",
        "//deepmind/lua:table_ref",
        "//deepmind/tensor:lua_tensor",
        "//deepmind/tensor:tensor_view",
        "//deepmind/util:default_read_only_file_system",
        "//deepmind/util:files",
        "//deepmind/util:smoother",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:span",
        "@eigen_archive//:eigen",
    ],
)

cc_library(
    name = "context_entities",
    srcs = ["context_entities.cc"],
    hdrs = ["context_entities.h"],
    deps = [
        "//deepmind/lua",
        "//deepmind/lua:class",
        "//deepmind/lua:n_results_or",
        "//deepmind/lua:push",
        "//deepmind/lua:read",
        "//deepmind/lua:table_ref",
    ],
)

cc_test(
    name = "context_entities_test",
    srcs = ["context_entities_test.cc"],
    deps = [
        ":context_entities",
        "//deepmind/lua:bind",
        "//deepmind/lua:call",
        "//deepmind/lua:n_results_or_test_util",
        "//deepmind/lua:push",
        "//deepmind/lua:push_script",
        "//deepmind/lua:read",
        "//deepmind/lua:table_ref",
        "//deepmind/lua:vm_test_util",
        "@com_google_absl//absl/types:span",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "context_observations",
    srcs = ["context_observations.cc"],
    hdrs = ["context_observations.h"],
    deps = [
        "//deepmind/lua",
        "//deepmind/lua:call",
        "//deepmind/lua:n_results_or",
        "//deepmind/lua:push",
        "//deepmind/lua:read",
        "//deepmind/lua:table_ref",
        "//deepmind/tensor:lua_tensor",
        "//deepmind/tensor:tensor_view",
        "//third_party/rl_api:env_c_api",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
    ],
)

cc_library(
    name = "context_actions",
    srcs = ["context_actions.cc"],
    hdrs = ["context_actions.h"],
    deps = [
        "//deepmind/lua",
        "//deepmind/lua:call",
        "//deepmind/lua:n_results_or",
        "//deepmind/lua:push",
        "//deepmind/lua:read",
        "//deepmind/lua:stack_resetter",
        "//deepmind/lua:table_ref",
        "//deepmind/tensor:lua_tensor",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/types:span",
    ],
)

cc_library(
    name = "context_pickups",
    srcs = ["context_pickups.cc"],
    hdrs = ["context_pickups.h"],
    deps = [
        "//deepmind/include:context_hdrs",
        "//deepmind/lua:call",
        "//deepmind/lua:class",
        "//deepmind/lua:n_results_or",
        "//deepmind/lua:push",
        "//deepmind/lua:read",
        "//deepmind/lua:table_ref",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/log:check",
    ],
)

cc_library(
    name = "callbacks",
    srcs = ["callbacks.cc"],
    data = [
        "//:game_script_assets",
        "//:non_pk3_assets",
    ],
    visibility = ["//visibility:public"],
    deps = [
        ":context",
        ":lua_maze_generation",
        ":lua_random",
        ":lua_text_level_maker",
        "//:file_reader_types",
        "//deepmind/include:context_hdrs",
        "//deepmind/level_generation/text_level:lua_bindings",
        "//deepmind/lua:vm",
        "//deepmind/model_generation:lua_model",
        "//deepmind/tensor:lua_tensor",
    ],
    alwayslink = 1,  # provides symbols for a shared C and C++ header
)

cc_test(
    name = "callbacks_test",
    size = "small",
    srcs = ["callbacks_test.cc"],
    deps = [
        ":callbacks",
        "//deepmind/include:context_hdrs",
        "//deepmind/support:test_srcdir",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "lua_maze_generation",
    srcs = ["lua_maze_generation.cc"],
    hdrs = ["lua_maze_generation.h"],
    deps = [
        ":lua_random",
        "//deepmind/level_generation/text_level:char_grid",
        "//deepmind/level_generation/text_maze_generation:algorithm",
        "//deepmind/level_generation/text_maze_generation:flood_fill",
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
    size = "large",
    srcs = ["lua_maze_generation_test.cc"],
    tags = ["manual"],  # Different C++ library implementations may generate different results.
    deps = [
        ":lua_maze_generation",
        ":lua_random",
        "//deepmind/lua:bind",
        "//deepmind/lua:call",
        "//deepmind/lua:n_results_or_test_util",
        "//deepmind/lua:push_script",
        "//deepmind/lua:table_ref",
        "//deepmind/lua:vm_test_util",
        "@com_google_absl//absl/log",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "lua_random",
    srcs = ["lua_random.cc"],
    hdrs = ["lua_random.h"],
    visibility = ["//deepmind/tensor:__pkg__"],
    deps = [
        "//deepmind/lua",
        "//deepmind/lua:class",
        "//deepmind/lua:n_results_or",
        "//deepmind/lua:push",
        "//deepmind/lua:read",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "lua_random_test",
    size = "small",
    srcs = ["lua_random_test.cc"],
    deps = [
        ":lua_random",
        "//deepmind/lua:bind",
        "//deepmind/lua:call",
        "//deepmind/lua:n_results_or_test_util",
        "//deepmind/lua:push_script",
        "//deepmind/lua:vm_test_util",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "lua_text_level_maker",
    srcs = ["lua_text_level_maker.cc"],
    hdrs = ["lua_text_level_maker.h"],
    visibility = ["//deepmind:__subpackages__"],
    deps = [
        ":lua_random",
        "//:level_cache_types",
        "//deepmind/level_generation:compile_map",
        "//deepmind/level_generation/text_level:lua_bindings",
        "//deepmind/lua",
        "//deepmind/lua:call",
        "//deepmind/lua:class",
        "//deepmind/lua:n_results_or",
        "//deepmind/lua:push",
        "//deepmind/lua:read",
        "//deepmind/lua:table_ref",
        "//deepmind/util:files",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/strings",
    ],
)

cc_library(
    name = "lua_image",
    srcs = ["lua_image.cc"],
    hdrs = ["lua_image.h"],
    deps = [
        "//:file_reader_types",
        "//deepmind/lua",
        "//deepmind/lua:bind",
        "//deepmind/lua:n_results_or",
        "//deepmind/lua:push",
        "//deepmind/lua:read",
        "//deepmind/lua:table_ref",
        "//deepmind/tensor:lua_tensor",
        "//deepmind/tensor:tensor_view",
        "//deepmind/util:file_reader",
        "@com_google_absl//absl/strings",
        "@png_archive//:png",
    ],
)

cc_test(
    name = "lua_image_test",
    size = "small",
    srcs = ["lua_image_test.cc"],
    deps = [
        ":lua_image",
        "//deepmind/lua:call",
        "//deepmind/lua:n_results_or_test_util",
        "//deepmind/lua:push_script",
        "//deepmind/lua:vm_test_util",
        "//deepmind/tensor:lua_tensor",
        "//deepmind/util:default_read_only_file_system",
        "@com_google_googletest//:gtest_main",
    ],
)
