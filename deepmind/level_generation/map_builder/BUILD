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
        "//deepmind/support:str_cat",
        "//deepmind/support:str_join",
        "//deepmind/support:str_split",
        "//deepmind/support:string_view",
        "//deepmind/support:stringprintf",
        "@eigen_archive//:eigen",
    ],
)

cc_test(
    name = "map_builder_test",
    size = "small",
    srcs = ["map_builder_test.cc"],
    deps = [
        ":map_builder",
        "//deepmind/support:stringprintf",
        "@googletest//:gtest_main",
    ],
)
