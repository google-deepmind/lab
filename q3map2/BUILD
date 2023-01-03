# Description:
#   The open source, cross platform level editor for idtech games.

package(default_visibility = ["//visibility:private"])

licenses(["restricted"])  # GPL v2 or later, portions BSD

cc_library(
    name = "ddslib",
    srcs = glob(["libs/ddslib/*.c"]) + ["libs/ddslib.h"],
    copts = ["-fno-strict-aliasing"],
    includes = ["libs"],
)

cc_library(
    name = "mathlib",
    srcs = glob(["libs/mathlib/*.c"]) + ["libs/bytebool.h"],
    hdrs = ["libs/mathlib.h"],
    includes = ["libs"],
)

cc_library(
    name = "l_net",
    srcs = [
        "libs/l_net/l_net.c",
        "libs/l_net/l_net_berkeley.c",
    ],
    hdrs = ["libs/l_net/l_net.h"],
    copts = ["-fno-strict-aliasing"],
    includes = ["libs"],
    textual_hdrs = ["libs/l_net/l_net_wins.h"],
)

cc_library(
    name = "picomodel",
    srcs = glob([
        "libs/picomodel/*.c",
        "libs/picomodel/lwo/*.c",
    ]) + [
        "libs/picomodel/picointernal.h",
        "libs/picomodel/pm_fm.h",
    ],
    copts = [
        "-Iq3map2/libs",
        "-Wno-error",
        "-w",
    ],
    textual_hdrs = [
        "libs/picomodel.h",
        "libs/picomodel/lwo/lwo2.h",
    ],
)

cc_binary(
    name = "q3map2",
    srcs = glob(
        include = [
            "common/*.c",
            "common/*.h",
            "q3map2/*.c",
            "q3map2/*.h",
            "include/*.h",
        ],
        exclude = [
            "common/bspfile.*",
            "common/l3dslib.c",
            "q3map2/light_shadows.c",
            "q3map2/lightmaps.c",
        ],
    ),
    copts = [
        "-fno-strict-aliasing",
        "-Iq3map2/common",
        "-Iq3map2/include",
        "-Iq3map2/q3map2",
        "-Iexternal/glib_archive/glib",
        "-w",
        "-DMD4_Final=MD4Final",
        "-DMD4_Init=MD4Init",
        "-DMD4_Update=MD4Update",
        "-DMD5(data,len,out)=\"{ MD5_CTX ctx; MD5Init(&ctx); MD5Update(&ctx, data, len); MD5Final(out, &ctx); }\"",
    ],
    visibility = [
        "//:__pkg__",
        "//deepmind/level_generation:__pkg__",
    ],
    deps = [
        ":ddslib",
        ":l_net",
        ":mathlib",
        ":picomodel",
        "//third_party/md:md4",
        "//third_party/md:md5",
        "@glib_archive//:glib",
        "@jpeg_archive//:jpeg",
        "@libxml_archive//:libxml",
        "@png_archive//:png",
    ],
)
