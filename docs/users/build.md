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

     Tested on Debian 8.6 (Jessie) and Ubuntu 14.04 (Trusty) and newer.

     ```shell
     $ sudo apt-get install libffi-dev gettext freeglut3-dev libsdl2-dev \
           zip libosmesa6-dev python-dev python-numpy python-pil
     ```

   * On Red Hat Enterprise Linux Server:

     Tested on release 7.2 (Maipo). This should also work on Centos 7, and with
     some modifications of the package installation commands on Centos 6.

     ```shell
     sudo yum -y install unzip java-1.8.0-openjdk libffi-devel gcc gcc-c++ \
         java-1.8.0-openjdk-devel freeglut-devel python-devel python-imaging \
         SDL2 SDL2-devel mesa-libOSMesa-devel zip numpy
     ```

   * On SUSE Linux:

     Tested on SUSE Linux Enterprise Server 12.

     ```shell
     sudo zypper --non-interactive install gcc gcc-c++ java-1_8_0-openjdk \
         java-1_8_0-openjdk-devel libOSMesa-devel freeglut-devel libSDL-devel \
         python-devel python-numpy-devel python-imaging
     ```

For Python3, replace the various `python-*` packages with their corresponding
`python3-*` versions.

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
Python is not included, but instead must already be installed on your
system. This means that depending on the details of where that library is
installed, you may need to adjust the Bazel build rules in
[`python.BUILD`](../../python.BUILD) to locate it correctly.

The default build rules should work for Debian and Ubuntu. Note that paths in
the build rules are relative to the root path specified in the
[`WORKSPACE`](../../WORKSPACE) file (which is `"/usr"` by default).

Python requires two separate dependencies: The CPython extension API, and NumPy.
If, say, NumPy is installed in a custom location, like it is on SUSE Linux, you
need to add the files from that location and set an include search path
accordingly. For example:

```python
cc_library(
    name = "python",
    hdrs = glob([
        "include/python2.7/*.h",
        "lib64/python2.7/site-packages/numpy/core/include/**/*.h",
    ]),
    includes = [
        "include/python2.7",
        "lib64/python2.7/site-packages/numpy/core/include",
    ],
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

You will also need to change the Python paths if you want to use Python 3. For
example:

```python
cc_library(
    name = "python",
    hdrs = glob([
        "include/python3.5/*.h",
        "lib/python3/dist-packages/numpy/core/include/numpy/*.h",
    ]),
    includes = [
        "include/python3.5",
        "lib/python3/dist-packages/numpy/core/include",
    ],
    visibility = ["//visibility:public"],
)
```

(In that case you may also want to pass `--python_path=/usr/bin/python3` to
Bazel. For building PIP packages, you may need to run the PIP packaging script
with `PYTHON_BIN_PATH="/usr/bin/python3"
bazel-bin/python/pip_package/build_pip_package /your/outputdir` and then use the
`pip3` command. As before, the Python binary needs to match the Python and NumPy
libraries that you linked against, which may need some care when a user's local
installation differs from the system-wide one.)


