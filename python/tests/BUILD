# Description:
#   Integration tests via Python.

licenses(["restricted"])

py_binary(
    name = "python_benchmark",
    srcs = ["benchmark.py"],
    data = ["//:deepmind_lab.so"],
    imports = ["python"],
    main = "benchmark.py",
    deps = [
        "@com_google_absl_py//absl/testing:absltest",
        "@six_archive//:six",
    ],
)

py_test(
    name = "python_module_test",
    size = "large",
    srcs = ["dmlab_module_test.py"],
    data = ["//:deepmind_lab.so"],
    imports = ["python"],
    main = "dmlab_module_test.py",
    tags = ["noprecommit"],
    deps = [
        "@com_google_absl_py//absl/testing:absltest",
        "@six_archive//:six",
    ],
)

py_test(
    name = "python_random_agent_test",
    size = "large",  # for TSAN
    srcs = ["random_agent_test.py"],
    data = ["//:deepmind_lab.so"],
    imports = ["python"],
    main = "random_agent_test.py",
    deps = [
        "//:python_random_agent_lib",
        "@com_google_absl_py//absl/testing:absltest",
        "@six_archive//:six",
    ],
)

py_test(
    name = "raycast_test",
    size = "large",  # for TSAN
    srcs = ["raycast_test.py"],
    data = ["//:deepmind_lab.so"],
    imports = ["python"],
    deps = [
        "@com_google_absl_py//absl/testing:absltest",
        "@six_archive//:six",
    ],
)

py_test(
    name = "in_fov_test",
    size = "large",  # for TSAN
    srcs = ["in_fov_test.py"],
    data = ["//:deepmind_lab.so"],
    imports = ["python"],
    deps = [
        "@com_google_absl_py//absl/testing:absltest",
        "@six_archive//:six",
    ],
)

py_test(
    name = "lookat_test",
    srcs = ["lookat_test.py"],
    data = ["//:deepmind_lab.so"],
    imports = ["python"],
    deps = [
        "@com_google_absl_py//absl/testing:absltest",
        "@six_archive//:six",
    ],
)

py_test(
    name = "player_info_test",
    srcs = ["player_info_test.py"],
    data = ["//:deepmind_lab.so"],
    imports = ["python"],
    deps = [
        "//python/tests/utils:math_utils",
        "@com_google_absl_py//absl/testing:absltest",
        "@six_archive//:six",
    ],
)

py_test(
    name = "properties_test",
    srcs = ["properties_test.py"],
    data = ["//:deepmind_lab.so"],
    imports = ["python"],
    deps = [
        "@com_google_absl_py//absl/testing:absltest",
        "@six_archive//:six",
    ],
)

py_test(
    name = "extra_entities_test",
    srcs = ["extra_entities_test.py"],
    data = ["//:deepmind_lab.so"],
    imports = ["python"],
    deps = [
        "@com_google_absl_py//absl/testing:absltest",
        "@six_archive//:six",
    ],
)

py_test(
    name = "extra_entities_with_bots_test",
    srcs = ["extra_entities_with_bots_test.py"],
    data = ["//:deepmind_lab.so"],
    imports = ["python"],
    deps = [
        "@com_google_absl_py//absl/testing:absltest",
        "@six_archive//:six",
    ],
)

py_test(
    name = "episode_time_test",
    srcs = ["episode_time_test.py"],
    data = ["//:deepmind_lab.so"],
    imports = ["python"],
    deps = [
        "@com_google_absl_py//absl/testing:absltest",
        "@six_archive//:six",
    ],
)

py_test(
    name = "determinism_test",
    srcs = ["determinism_test.py"],
    data = ["//:deepmind_lab.so"],
    imports = ["python"],
    deps = [
        "@com_google_absl_py//absl/testing:absltest",
        "@six_archive//:six",
    ],
)

py_test(
    name = "spawn_inventory_test",
    srcs = ["spawn_inventory_test.py"],
    data = ["//:deepmind_lab.so"],
    imports = ["python"],
    deps = [
        "@com_google_absl_py//absl/testing:absltest",
        "@six_archive//:six",
    ],
)

py_test(
    name = "update_inventory_test",
    srcs = ["update_inventory_test.py"],
    data = ["//:deepmind_lab.so"],
    imports = ["python"],
    deps = [
        "@com_google_absl_py//absl/testing:absltest",
        "@six_archive//:six",
    ],
)

py_test(
    name = "entity_info_test",
    srcs = ["entity_info_test.py"],
    data = ["//:deepmind_lab.so"],
    imports = ["python"],
    deps = [
        "@com_google_absl_py//absl/testing:absltest",
        "@six_archive//:six",
    ],
)

py_test(
    name = "debug_observation_test",
    srcs = ["debug_observation_test.py"],
    data = ["//:deepmind_lab.so"],
    imports = ["python"],
    deps = [
        "@com_google_absl_py//absl/testing:absltest",
        "@six_archive//:six",
    ],
)

py_test(
    name = "custom_view_test",
    srcs = ["custom_view_test.py"],
    data = ["//:deepmind_lab.so"],
    imports = ["python"],
    deps = [
        "//python/tests/utils:math_utils",
        "@com_google_absl_py//absl/testing:absltest",
    ],
)

py_test(
    name = "teleporter_test",
    srcs = ["teleporter_test.py"],
    data = ["//:deepmind_lab.so"],
    imports = ["python"],
    deps = [
        "@com_google_absl_py//absl/testing:absltest",
        "@six_archive//:six",
    ],
)

py_test(
    name = "render_test",
    srcs = ["render_test.py"],
    data = ["//:deepmind_lab.so"],
    imports = ["python"],
    tags = ["manual"],
    deps = ["@com_google_absl_py//absl/testing:absltest"],
)

py_test(
    name = "maze_generation_test",
    size = "enormous",
    srcs = ["maze_generation_test.py"],
    data = ["//:deepmind_lab.so"],
    imports = ["python"],
    tags = ["manual"],
    deps = [
        "@com_google_absl_py//absl/testing:absltest",
        "@six_archive//:six",
    ],
)

py_test(
    name = "team_model_select_test",
    srcs = ["team_model_select_test.py"],
    data = ["//:deepmind_lab.so"],
    imports = ["python"],
    tags = ["manual"],
    deps = ["@six_archive//:six"],
)

py_test(
    name = "custom_actions_test",
    srcs = ["custom_actions_test.py"],
    data = ["//:deepmind_lab.so"],
    imports = ["python"],
    deps = ["@com_google_absl_py//absl/testing:absltest"],
)

py_test(
    name = "game_command_test",
    srcs = ["game_command_test.py"],
    data = ["//:deepmind_lab.so"],
    imports = ["python"],
    deps = [
        "@com_google_absl_py//absl/testing:absltest",
        "@six_archive//:six",
    ],
)

py_test(
    name = "final_reward_test",
    srcs = ["final_reward_test.py"],
    data = ["//:deepmind_lab.so"],
    imports = ["python"],
    deps = [
        "@com_google_absl_py//absl/testing:absltest",
        "@six_archive//:six",
    ],
)

py_test(
    name = "level_cache_test",
    srcs = ["level_cache_test.py"],
    data = ["//:deepmind_lab.so"],
    imports = ["python"],
    deps = [
        "@com_google_absl_py//absl/testing:absltest",
        "@six_archive//:six",
    ],
)

py_test(
    name = "dmenv_module_test",
    srcs = ["dmenv_module_test.py"],
    tags = ["noprecommit"],
    deps = [
        "//:dmenv_module",
        "@com_google_absl_py//absl/testing:absltest",
        "@dm_env_archive//:test_utils",
    ],
)
