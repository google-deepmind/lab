# Description:
#   End-to-end tests for DeepMind Lab.

licenses(["restricted"])

cc_test(
    name = "recording_test",
    srcs = ["recording_test.cc"],
    data = [
        "//:assets_bots_pk3",
        "//:assets_oa_pk3",
        "//:assets_pk3",
        "//:built_maps",
        "//:game_script_assets",
        "//:libdmlab_headless_hw.so",
        "//:non_pk3_assets",
        "//:vm_pk3",
    ],
    deps = [
        ":env_observation",
        ":env_observation_util",
        "//:dmlablib",
        "//deepmind/support:test_srcdir",
        "//deepmind/util:files",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "env_observation",
    srcs = ["env_observation.cc"],
    hdrs = ["env_observation.h"],
    deps = [
        "//:dmlablib",
        "//third_party/rl_api:env_c_api",
    ],
)

cc_library(
    name = "env_observation_util",
    srcs = ["env_observation_util.cc"],
    hdrs = ["env_observation_util.h"],
    deps = [
        ":env_observation",
        "@jpeg_archive//:jpeg",
    ],
)

cc_library(
    name = "lua_unit_test_lib",
    testonly = 1,
    srcs = ["lua_unit_test.cc"],
    data = [
        "//:assets_bots_pk3",
        "//:assets_oa_pk3",
        "//:assets_pk3",
        "//:built_maps",
        "//:game_script_assets",
        "//:non_pk3_assets",
        "//:vm_pk3",
    ],
    visibility = ["//lua_tests:__pkg__"],
    deps = [
        "//:dmlablib",
        "//deepmind/support:test_srcdir",
        "@com_google_googletest//:gtest",
    ],
)

cc_library(
    name = "load_level_test_lib",
    testonly = 1,
    srcs = ["load_level_test.cc"],
    data = [
        "//:assets_bots_pk3",
        "//:assets_oa_pk3",
        "//:assets_pk3",
        "//:built_maps",
        "//:game_script_assets",
        "//:libdmlab_headless_hw.so",
        "//:non_pk3_assets",
        "//:vm_pk3",
    ],
    visibility = ["//:__pkg__"],
    deps = [
        ":env_observation",
        "//:dmlablib",
        "//deepmind/support:test_srcdir",
        "//deepmind/util:files",
        "//third_party/rl_api:env_c_api",
        "@com_google_absl//absl/log:check",
        "@com_google_googletest//:gtest",
    ],
)

cc_library(
    name = "seed_level_test_lib",
    testonly = 1,
    srcs = ["seed_level_test.cc"],
    data = [
        "//:assets_bots_pk3",
        "//:assets_oa_pk3",
        "//:assets_pk3",
        "//:built_maps",
        "//:game_script_assets",
        "//:libdmlab_headless_hw.so",
        "//:non_pk3_assets",
        "//:vm_pk3",
    ],
    visibility = ["//:__pkg__"],
    deps = [
        "//:dmlablib",
        "//deepmind/support:test_srcdir",
        "//deepmind/util:files",
        "//third_party/rl_api:env_c_api",
        "@com_google_absl//absl/log:check",
        "@com_google_googletest//:gtest",
    ],
)
