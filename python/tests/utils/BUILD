# Description:
#   Utility libraries for DM Lab tests.

package(default_visibility = ["//python/tests:__subpackages__"])

licenses(["restricted"])  # GPLv2

PY_VERSION = "PY2"

py_library(
    name = "math_utils",
    testonly = 1,
    srcs = ["math_utils.py"],
    data = ["//:deepmind_lab.so"],
    imports = ["python"],
)

py_test(
    name = "math_utils_test",
    srcs = ["math_utils_test.py"],
    data = ["//:deepmind_lab.so"],
    imports = ["python"],
    python_version = PY_VERSION,
    deps = [":math_utils"],
)

py_library(
    name = "test_environment_decorator",
    testonly = 1,
    srcs = ["test_environment_decorator.py"],
    data = ["//:deepmind_lab.so"],
    imports = ["python"],
)

py_test(
    name = "test_environment_decorator_test",
    srcs = ["test_environment_decorator_test.py"],
    data = ["//:deepmind_lab.so"],
    imports = ["python"],
    python_version = PY_VERSION,
    deps = [
        ":test_environment_decorator",
        "@six_archive//:six",
    ],
)

py_library(
    name = "game_controller",
    testonly = 1,
    srcs = ["game_controller.py"],
    data = ["//:deepmind_lab.so"],
    imports = ["python"],
    deps = [
        ":math_utils",
        "@six_archive//:six",
    ],
)

py_test(
    name = "game_controller_test",
    srcs = ["game_controller_test.py"],
    data = ["//:deepmind_lab.so"],
    imports = ["python"],
    python_version = PY_VERSION,
    deps = [
        ":game_controller",
        ":math_utils",
        ":test_environment_decorator",
        "@six_archive//:six",
    ],
)

py_library(
    name = "maze_game_controller",
    testonly = 1,
    srcs = ["maze_game_controller.py"],
    data = ["//:deepmind_lab.so"],
    imports = ["python"],
    deps = [":game_controller"],
)

py_test(
    name = "maze_game_controller_test",
    srcs = ["maze_game_controller_test.py"],
    data = ["//:deepmind_lab.so"],
    imports = ["python"],
    python_version = PY_VERSION,
    deps = [
        ":maze_game_controller",
        ":test_environment_decorator",
        "@six_archive//:six",
    ],
)
