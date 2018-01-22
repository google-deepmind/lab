# Description:
#   Data structures and tools for building .map files.

licenses(["restricted"])  # GPLv2

cc_library(
    name = "map_builder",
    srcs = [
        "brush.cc",
        "builder.cc",
        "entity.cc",
    ],
    hdrs = [
        "brush.h",
        "builder.h",
        "entity.h",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//deepmind/support:stringprintf",
        "@com_google_absl//absl/strings",
        "@eigen_archive//:eigen",
    ],
)

cc_test(
    name = "map_builder_test",
    size = "small",
    srcs = ["map_builder_test.cc"],
    deps = [
        ":map_builder",
        "@com_google_absl//absl/strings",
        "@com_google_googletest//:gtest_main",
    ],
)
