# Description:
#   End-to-end tests for DeepMind Lab.

licenses(["restricted"])  # GPLv2

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
