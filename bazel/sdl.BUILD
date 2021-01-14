# Description:
#   Build rule for SDL2.
#
#   On Linux, compiler and linker flags can be found
#   with `sdl2-config --libs --cflags`.

cc_library(
    name = "sdl2",
    srcs = select({
        "@//:is_linux": [],
        "@//:is_macos": ["local/Cellar/sdl2/2.0.12_1/lib/libSDL2-2.0.0.dylib"],
    }),
    hdrs = select({
        "@//:is_linux": glob(["include/SDL2/*.h"]),
        "@//:is_macos": glob(["local/Cellar/sdl2/2.0.12_1/include/SDL2/*.h"]),
    }),
    defines = ["_REENTRANT"],
    includes = select({
        "@//:is_linux": ["include/SDL2"],
        "@//:is_macos": ["local/Cellar/sdl2/2.0.12_1/include/SDL2"],
    }),
    linkopts = select({
        "@//:is_linux": ["-lSDL2"],
        "@//:is_macos": [],
    }),
    visibility = ["//visibility:public"],
)
