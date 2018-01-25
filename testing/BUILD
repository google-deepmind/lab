# Description:
#   End-to-end tests for DeepMind Lab.

licenses(["restricted"])  # GPLv2

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
    name = "lua_unit_test_lib",
    testonly = 1,
    srcs = ["lua_unit_test.cc"],
    data = [
        "//:assets_bots_pk3",
        "//:assets_oa_pk3",
        "//:assets_pk3",
        "//:game_script_assets",
        "//:map_assets",
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
        "//:game_script_assets",
        "//:libdmlab_headless_hw.so",
        "//:map_assets",
        "//:non_pk3_assets",
        "//:vm_pk3",
    ],
    visibility = ["//:__pkg__"],
    deps = [
        ":env_observation",
        "//:dmlablib",
        "//deepmind/support:logging",
        "//deepmind/support:test_srcdir",
        "//deepmind/util:files",
        "@com_google_googletest//:gtest",
    ],
)
