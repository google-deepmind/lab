# Description:
#   Build rule for Lua 5.1.
#   Compiler and linker flags found with `pkg-config --cflags --libs lua5.1`.
#   The package name and the resulting flags may vary from platform to platform,
#   cf. 'How to build DeepMind Lab' in build.md.

cc_library(
    name = "lua",
    hdrs = glob(["include/lua5.1/*.h"]),
    includes = ["include/lua5.1"],
    linkopts = ["-llua5.1"],
    visibility = ["//visibility:public"],
)
