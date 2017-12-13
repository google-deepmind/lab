# Description:
#   Libraries for dynamic level generation.

licenses(["restricted"])  # GPLv2

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
    data = [":compile_map_sh"],
    visibility = ["//deepmind:__subpackages__"],
    deps = [
        "//deepmind/support:logging",
        "@com_google_absl//absl/strings",
    ],
)
