# Description:
#   DeepMind reinforcement learning APIs.

licenses(["notice"])  # Apache 2.0 (Google-authored)

cc_library(
    name = "env_c_api",
    hdrs = ["env_c_api.h"],
    visibility = ["//visibility:public"],
)

cc_binary(
    name = "libenv_c_api_example.so",
    testonly = 1,
    srcs = ["env_c_api_example.cc"],
    linkopts = [
        "-Wl,--version-script",
        ":env_c_api_example.lds",
    ],
    linkshared = 1,
    linkstatic = 1,
    visibility = ["//visibility:public"],
    deps = [
        ":env_c_api",
        ":env_c_api_example.lds",
    ],
)

cc_test(
    name = "env_c_api_example_test",
    size = "small",
    srcs = [
        "env_c_api_example.h",
        "env_c_api_example_test.cc",
        "libenv_c_api_example.so",
    ],
    deps = [
        ":env_c_api",
        "@googletest//:gtest_main",
    ],
)
