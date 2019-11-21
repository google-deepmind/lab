# How to build *DeepMind Lab*




*DeepMind Lab* uses [Bazel](https://bazel.build/) as its build system. Its main
`BUILD` file defines a number of *build targets* and their dependencies. The
build rules should work out of the box on Debian (Jessie or newer) and Ubuntu
(version 14.04 or newer), provided the required packages are installed.
*DeepMind Lab* also builds on other Linux systems, but some changes to the build
files might be required, see below.

*DeepMind Lab* is written in C99 and C++11, and you will need a sufficiently
modern compiler. GCC 4.8 should suffice.

Instructions for installing Bazel can be found in the [Bazel install
guide](https://docs.bazel.build/versions/master/install.html).

You may need to deal with some details concerning Python dependencies. Those
are documented in a [separate section](#python-dependencies) below.

## Step-by-step instructions for building and running

1. Install Bazel (see above).

2. Install *DeepMind Lab*'s dependencies:

   * On Debian or Ubuntu:

     Tested on Debian 9 (Strectch) and Ubuntu 16.04 (Xenial) and newer.
     Tested with Python 2 only on Debian 8.6 (Jesse) and Ubuntu 14.04 (Trusty).

     ```shell
     $ sudo apt-get install libffi-dev gettext freeglut3-dev libsdl2-dev \
           zip libosmesa6-dev python-dev python-numpy python-pil python3-dev \
           python3-numpy python3-pil
     ```

   * On Red Hat Enterprise Linux Server:

     Tested on release 7.6 (Maipo). This should also work on Centos 7, and with
     some modifications of the package installation commands on Centos 6. Tested
     with Python 2 only on release 7.2.

     ```shell
     sudo yum -y install unzip java-1.8.0-openjdk libffi-devel gcc gcc-c++ \
         java-1.8.0-openjdk-devel freeglut-devel python-devel python-imaging \
         numpy python36-numpy python36-pillow python36-devel SDL2 SDL2-devel \
         mesa-libOSMesa-devel zip
     ```

   * On SUSE Linux:

     Tested on SUSE Linux Enterprise Server 12.

     ```shell
     sudo zypper --non-interactive install gcc gcc-c++ java-1_8_0-openjdk \
         java-1_8_0-openjdk-devel libOSMesa-devel freeglut-devel libSDL-devel \
         python-devel python-numpy-devel python-imaging
     ```

If Python 3 support is not required, omit the packages that mention `python3`.

3. [Clone or download *DeepMind Lab*](https://github.com/deepmind/lab).

4. If necessary, edit `python.BUILD` according to the [Python
   instructions](#python-dependencies) below.

5. Build *DeepMind Lab* and run a random agent. (Use the `-c opt` flag to enable
   optimizations.)

   ```shell
   $ cd lab

   # Build the Python interface to DeepMind Lab
   lab$ bazel build -c opt //:deepmind_lab.so

   # Build and run the tests for it
   lab$ bazel test -c opt //python/tests:python_module_test

   # Run a random agent
   lab$ bazel run -c opt //:python_random_agent
   ```

The Bazel target `:deepmind_lab.so` builds the Python module that interfaces
with *DeepMind Lab*.

The random agent target `:python_random_agent` has a number of optional command line
arguments. Run `bazel run :random_agent -- --help` to see those.

## Python dependencies

*DeepMind Lab* does not include every dependency hermetically. In particular,
Python is not included, but instead it must already be installed on your system.
This means that depending on the details of where that library is installed, you
may need to adjust the Bazel build rules in
[`python.BUILD`](../../bazel/python.BUILD) to locate it correctly.

Bazel can build Python code using either Python 2 or Python 3. The default is
Python 3, but each individual `py_binary` and `py_test` target can specify the
desired version using the
[`python_version`](https://docs.bazel.build/versions/master/be/python.html#py_test.python_version)
argument. The build rules need to make the local installation path of correct
version of Python available.

If you only intend to use one of the two versions (e.g. on an older system where
Python 3 with NumPy is not available), you only need to provide paths for that
version; however, the codebase includes tests that run under both Python 2 and
Python 3.

The default build rules should work for Debian and Ubuntu. They use Bazel's
[configurable attributes](https://docs.bazel.build/versions/master/be/common-definitions.html#configurable-attributes)
to provide paths for Python 2 and Python 3, respectively, based on which version
is required during a particular build. Note that paths in the build rules are
relative to the root path specified in the [`WORKSPACE`](../../WORKSPACE) file
(which is `"/usr"` by default).

Python requires two separate dependencies: The CPython extension API, and NumPy.
If, say, NumPy is installed in a custom location, like it is on SUSE Linux and
RedHat Linux, you need to add the files from that location and set an include
search path accordingly. For example:

```python
cc_library(
    name = "python",
    hdrs = select(
        {
            "@bazel_tools//tools/python:PY2": glob([
                "include/python2.7/*.h",
                "lib64/python2.7/site-packages/numpy/core/include/**/*.h",
            ]),
            "@bazel_tools//tools/python:PY3": glob([
                "include/python3.6m/*.h",
                "lib64/python3.6/site-packages/numpy/core/include/**/*.h",
            ]),
        },
        no_match_error = "Internal error, Python version should be one of PY2 or PY3",
    ),
    includes = select(
        {
            "@bazel_tools//tools/python:PY2": [
                "include/python2.7",
                "lib64/python2.7/site-packages/numpy/core/include",
            ],
            "@bazel_tools//tools/python:PY3": [
                "include/python3.6m",
                "lib64/python3.6/site-packages/numpy/core/include",
            ],
        },
        no_match_error = "Internal error, Python version should be one of PY2 or PY3",
    ),
    visibility = ["//visibility:public"],
)
```
The outputs of `rpm -ql python` and `rpm -ql python-numpy-devel` might be
helpful to find the right include directories on Red-Hat-like systems.

If you have installed NumPy locally via PIP and would like to use the *DeepMind
Lab* PIP module, then you should build the module against the version of NumPy
that you will be using at runtime. You can discover the include path of that
version by running the following code in your desired environment:

```python
import numpy as np
print(np.get_include())
```

For [building PIP packages](../../python/pip_package/README.md), you may need to
run the PIP packaging script with `PYTHON_BIN_PATH="/usr/bin/python3"
bazel-bin/python/pip_package/build_pip_package /your/outputdir` and then use the
`pip3` command. As before, the Python binary needs to match the Python and NumPy
libraries that you linked against, which may need some care when a user's local
installation differs from the system-wide one.


