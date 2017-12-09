# Description:
#   Build rule for Lua 5.1.
#   Compiler and linker flags found with `pkg-config --cflags --libs lua5.1`.
#   The package name and the resulting flags may vary from platform to platform,
#   cf. 'How to build DeepMind Lab' in build.md.

cc_library(
    name = "lua",
    hdrs = glob([
        "include/lua5.1/*.h",
        # "local/include/lua5.2/*.h",
        # Homebrew
        "local/opt/lua/include/*.h",
    ]),
    includes = [
        "include/lua5.1",
        # "local/include/lua5.2",
        # Homebrew
        "local/opt/lua/include",
    ],
    linkopts = select({
        "@//:darwin": [
            "-llua.5.2",
            # Homebrew
            "-L/usr/local/opt/lua/lib",
        ],
        "@//:darwin_x86_64": [
            "-llua.5.2",
            # Homebrew
            "-L/usr/local/opt/lua/lib",
        ],
        "//conditions:default": [
            "-llua5.1",
        ],
    }),
    visibility = ["//visibility:public"],
)
