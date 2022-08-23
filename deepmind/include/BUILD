# Description:
#   Internal C API for hooks and callbacks.

licenses(["restricted"])

cc_library(
    name = "context_hdrs",
    hdrs = [
        "deepmind_calls.h",
        "deepmind_context.h",
        "deepmind_hooks.h",
        "deepmind_model_getters.h",
        "deepmind_model_setters.h",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//:file_reader_types",
        "//:level_cache_types",
        "//third_party/rl_api:env_c_api",
    ],
)
