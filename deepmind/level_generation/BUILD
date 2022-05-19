# Description:
#   Libraries for dynamic level generation.

licenses(["restricted"])

sh_binary(
    name = "compile_map_sh",
    srcs = ["compile_map.sh"],
    data = [
        "//:assets_oa_pk3",
        "//:assets_pk3",
        "//:bspc",
        "//q3map2",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "compile_map",
    srcs = ["compile_map.cc"],
    hdrs = ["compile_map.h"],
    copts = [
        "-DMD5_DIGEST_LENGTH=16",
        "-DMD5_Final=MD5Final",
        "-DMD5_Init=MD5Init",
        "-DMD5_Update=MD5Update",
    ],
    data = [":compile_map_sh"],
    visibility = ["//deepmind:__subpackages__"],
    deps = [
        "//:level_cache_types",
        "//deepmind/util:files",
        "//deepmind/util:run_executable",
        "//third_party/md:md5",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "compile_map_test",
    srcs = ["compile_map_test.cc"],
    deps = [
        ":compile_map",
        "@com_google_absl//absl/strings",
        "@com_google_googletest//:gtest_main",
    ],
)
