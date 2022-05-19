# Description:
#   DeepMind reinforcement learning APIs.

licenses(["notice"])

exports_files(["LICENSE"])

cc_library(
    name = "env_c_api",
    hdrs = ["env_c_api.h"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "env_c_api_test_suite",
    testonly = 1,
    srcs = ["env_c_api_test_suite.cc"],
    hdrs = ["env_c_api_test_suite.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":env_c_api",
        "@com_google_absl//absl/log",
        "@com_google_googletest//:gtest",
    ],
    alwayslink = 1,  # contains test definitions
)

cc_library(
    name = "env_c_api_bind",
    hdrs = ["env_c_api_bind.h"],
    visibility = ["//visibility:public"],
    deps = [":env_c_api"],
)

cc_binary(
    name = "libenv_c_api_example.so",
    testonly = 1,
    srcs = ["env_c_api_example.cc"],
    linkopts = ["-Wl,--version-script=$(location :env_c_api_example.lds)"],
    linkshared = 1,
    linkstatic = 1,
    deps = [
        ":env_c_api",
        ":env_c_api_bind",
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
        "@com_google_googletest//:gtest_main",
    ],
)
