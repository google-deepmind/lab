# Description:
#   Build rule for Lua 5.1.

cc_library(
    name = "lua",
    srcs = glob(
        include = [
            "*.c",
            "*.h",
        ],
        exclude = [
            "lauxlib.h",
            "lua.c",
            "lua.h",
            "luac.c",
            "lualib.h",
            "print.c",
        ],
    ),
    hdrs = [
        "lauxlib.h",
        "lua.h",
        "lualib.h",
    ],
    visibility = ["//visibility:public"],
)
