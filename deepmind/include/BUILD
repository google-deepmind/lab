# Description:
#   Internal C API for hooks and callbacks.

licenses(["restricted"])  # GPLv2

cc_library(
    name = "context_headers",
    hdrs = [
        "deepmind_calls.h",
        "deepmind_context.h",
        "deepmind_hooks.h",
    ],
    visibility = [
        "//:__pkg__",
        "//deepmind/engine:__pkg__",
    ],
    deps = ["//third_party/rl_api:env_c_api"],
)
