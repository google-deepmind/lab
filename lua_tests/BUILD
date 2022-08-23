# Description:
#   Unit tests for DeepMind Lab.

licenses(["restricted"])

TEST_SCRIPTS = [
    test_script[:-len(".lua")]
    for test_script in glob(["*.lua"])
]

# Tests that need custom data.
SKIPPED_TESTS = [
    "image_test",
    "read_file_test",
]

test_suite(
    name = "lua_unit_test_suite",
    tests = [test_script for test_script in TEST_SCRIPTS if test_script not in SKIPPED_TESTS],
)

[
    cc_test(
        name = test_script,
        args = ["lua_tests/" + test_script + ".lua"],
        data = [test_script + ".lua"],
        deps = ["//testing:lua_unit_test_lib"],
    )
    for test_script in TEST_SCRIPTS
    if test_script not in SKIPPED_TESTS
]

cc_test(
    name = "image_test",
    size = "small",
    args = ["lua_tests/image_test.lua"],
    data = [
        "data/testL.png",
        "data/testRGB.png",
        "data/testRGBA.png",
        ":image_test.lua",
    ],
    deps = ["//testing:lua_unit_test_lib"],
)

cc_test(
    name = "read_file_test",
    size = "small",
    args = ["lua_tests/read_file_test.lua"],
    data = [
        "data/empty_test_file",
        "data/testL.png",
        ":read_file_test.lua",
    ],
    deps = ["//testing:lua_unit_test_lib"],
)
