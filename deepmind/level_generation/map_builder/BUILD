# Description:
#   Data structures and tools for building .map files.

licenses(["restricted"])

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
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
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
