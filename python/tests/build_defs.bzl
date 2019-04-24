"""A macro for simultaneous testing of Py2 and Py3."""

def py2and3_test(name, **kwargs):
    """A drop-in replacement for py_test that runs tests under both Py2 and Py3.

    An invocation `py2and3_test(name = "foo_test", ...)` creates two actual test
    targets foo.py2 and foo.py3, which set python_version respectively to "PY2"
    and "PY3".

    Args:
      name: the rule name, which becomes the name of a test suite
      **kwargs: forwarded to each py_test
    """

    py2 = name + ".py2"
    py3 = name + ".py3"

    main = kwargs.pop("main", name + ".py")
    tags = kwargs.pop("tags", [])

    native.py_test(name = py2, main = main, tags = tags, python_version = "PY2", **kwargs)
    native.py_test(name = py3, main = main, tags = tags, python_version = "PY3", **kwargs)
    native.test_suite(name = name, tags = tags, tests = [py2, py3])
