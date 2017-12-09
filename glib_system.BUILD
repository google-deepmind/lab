
cc_library(
    name = "glib",
    hdrs = glob([
        # "local/include/glib-2.0/*.h",
        # "local/include/glib-2.0/*/*.h",
        # "local/include/glib-2.0/*/*/*.h",
        # "local/lib/glib-2.0/include/*.h",
        # Homebrew
        "local/opt/glib/include/glib-2.0/**/*.h",
        "local/opt/glib/lib/glib-2.0/include/**/*.h",
    ]),
    defines = ["_REENTRANT"],
    includes = [
        # "local/include/glib-2.0",
        # "local/lib/glib-2.0/include",
        # Homebrew
        "local/opt/glib/include/glib-2.0",
        "local/opt/glib/lib/glib-2.0/include",
    ],
    linkopts = [
        "-lglib-2.0",
        # "-L/usr/local/lib",
        # Homebrew
        "-L/usr/local/opt/glib/lib",
    ],
    visibility = ["//visibility:public"],
)
