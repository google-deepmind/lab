# Description:
#   Integration tests via Python.

licenses(["restricted"])  # GPLv2

py_binary(
    name = "python_benchmark",
    srcs = ["benchmark.py"],
    data = ["//:deepmind_lab.so"],
    imports = ["python"],
    main = "benchmark.py",
)

py_test(
    name = "python_module_test",
    size = "large",
    srcs = ["dmlab_module_test.py"],
    data = ["//:deepmind_lab.so"],
    imports = ["python"],
    main = "dmlab_module_test.py",
)

py_test(
    name = "python_random_agent_test",
    size = "large",  # for TSAN
    srcs = ["random_agent_test.py"],
    data = ["//:deepmind_lab.so"],
    imports = ["python"],
    main = "random_agent_test.py",
    deps = ["//:python_random_agent"],
)
