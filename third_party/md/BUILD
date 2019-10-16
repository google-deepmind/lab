# Description:
#   Reference implementations of MD4 and MD5
#   Taken from:
#     http://www.ietf.org/rfc/rfc1320.txt (MD4)
#     http://www.ietf.org/rfc/rfc1321.txt (MD5)

licenses(["notice"])  # RSA MD4, MD5 Licenses (see LICENSE.{md4,md5})

cc_library(
    name = "md_global",
    hdrs = ["global.h"],
)

cc_library(
    name = "md4",
    srcs = ["md4c.c"],
    hdrs = ["md4.h"],
    visibility = ["//visibility:public"],
    deps = [":md_global"],
)

cc_binary(
    name = "md4driver",
    testonly = 1,
    srcs = ["mddriver.c"],
    copts = ["-DMD=4"],
    deps = [":md4"],
)

cc_library(
    name = "md5",
    srcs = ["md5c.c"],
    hdrs = ["md5.h"],
    visibility = ["//visibility:public"],
    deps = [":md_global"],
)

cc_binary(
    name = "md5driver",
    testonly = 1,
    srcs = ["mddriver.c"],
    copts = ["-DMD=5"],
    deps = [":md5"],
)

sh_test(
    name = "md_test",
    size = "small",
    srcs = ["md_test.sh"],
    data = [
        "golden.md4",
        "golden.md5",
        ":md4driver",
        ":md5driver",
    ],
)

# The fuzzers require Clang, see https://llvm.org/docs/LibFuzzer.html.

FUZZOPTS = "-fsanitize=fuzzer,address"

cc_binary(
    name = "md4_fuzzer",
    srcs = ["md4_fuzzer.c"],
    copts = [FUZZOPTS],
    linkopts = [FUZZOPTS],
    tags = ["manual"],
    deps = [":md4"],
)

cc_binary(
    name = "md5_fuzzer",
    srcs = ["md5_fuzzer.c"],
    copts = [FUZZOPTS],
    linkopts = [FUZZOPTS],
    tags = ["manual"],
    deps = [":md5"],
)
