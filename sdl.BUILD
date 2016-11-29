# Description:
#   Build rule for SDL2.
#   Compiler and linker flags found with `sdl2-config --libs --cflags`.

cc_library(
    name = "sdl2",
    hdrs = glob(["include/SDL2/*.h"]),
    defines = ["_REENTRANT"],
    includes = ["include/SDL2"],
    linkopts = ["-lSDL2"],
    visibility = ["//visibility:public"],
)
