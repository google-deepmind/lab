
cc_library(
    name = "libxml",
    hdrs = glob([
        # Homebrew
        "local/opt/libxml2/include/libxml2/libxml/*.h"
    ]),
    defines = ["_REENTRANT"],
    includes = [
        # Homebrew
        "local/opt/libxml2/include/libxml2"
    ],
    linkopts = ["-lxml2"],
    visibility = ["//visibility:public"],
)
