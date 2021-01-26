# Description:
#   libpng is the official PNG reference library.

licenses(["notice"])  # BSD/MIT-like license

genrule(
    name = "pnglibconf",
    srcs = ["scripts/pnglibconf.h.prebuilt"],
    outs = ["pnglibconf.h"],
    cmd = "cp $(location scripts/pnglibconf.h.prebuilt) $(location pnglibconf.h)",
)

config_setting(
    name = "is_arm64",
    constraint_values = ["@platforms//cpu:arm64"],
)

cc_library(
    name = "png",
    srcs = [
        "png.c",
        "pngdebug.h",
        "pngerror.c",
        "pngget.c",
        "pnginfo.h",
        "pnglibconf.h",
        "pngmem.c",
        "pngpread.c",
        "pngpriv.h",
        "pngread.c",
        "pngrio.c",
        "pngrtran.c",
        "pngrutil.c",
        "pngset.c",
        "pngstruct.h",
        "pngtrans.c",
        "pngwio.c",
        "pngwrite.c",
        "pngwtran.c",
        "pngwutil.c",
    ] + select({
        ":is_arm64": [
            "arm/arm_init.c",
            "arm/filter_neon_intrinsics.c",
            "arm/palette_neon_intrinsics.c",
        ],
        "//conditions:default": [],
    }),
    hdrs = [
        "png.h",
        "pngconf.h",
    ],
    includes = ["."],
    linkopts = ["-lm"],
    visibility = ["//visibility:public"],
    deps = ["@zlib_archive//:zlib"],
)
