# How to build *DeepMind Lab*




*DeepMind Lab* uses [Bazel](https://bazel.build/) as its build system. Its main
`BUILD` file defines a number of *build targets* and their dependencies. The
build rules should work out of the box on Debian (Jessie or newer) and Ubuntu
(version 14.04 or newer), provided the required packages are installed.
*DeepMind Lab* also builds on other Linux systems, but some changes to the build
files might be required, see below.

*DeepMind Lab* is written in C99 and C++17, and you will need a sufficiently
modern compiler.

Instructions for installing Bazel can be found in the [Bazel install
guide](https://docs.bazel.build/versions/master/install.html).

You may need to deal with some details concerning Python dependencies. Those
are documented in a [separate section](#python-dependencies) below.

## Step-by-step instructions for building and running

These instructions were checked for the initial release of *DeepMind Lab*, which
only required C++11. Since 2022 it requires C++17, and more recent versions of
the platforms shown below are needed. However, the general set of dependencies
should continue to remain largely accurate.

1. Install Bazel (see above).

2. Install *DeepMind Lab*'s dependencies:

   * On Debian or Ubuntu:

     Tested on Debian 9 (Strectch) and Ubuntu 16.04 (Xenial) and newer.
     Tested with Python 2 only on Debian 8.6 (Jesse) and Ubuntu 14.04 (Trusty).

     ```shell
     $ sudo apt-get install libffi-dev gettext freeglut3-dev libsdl2-dev \
           zip libosmesa6-dev python-dev python-numpy python-pil python-enum34 \
          python3-dev python3-numpy python3-pil
     ```

     To build a [PIP package](../../python/pip_package/README.md), also install
     `python3-setuptools python-setuptools python3-wheel python-wheel`. To use
     it, install `python3-pip python-pip`, and also `python3-virtualenv
     python-virtualenv` to use virtualenv.

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

3. [Clone or download *DeepMind Lab*](https://github.com/deepmind/lab).

4. Build *DeepMind Lab* and run a random agent. (Use the `-c opt` flag to enable
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
Our Bazel workspace includes a mechanism to discover the location of the
system's Python paths automatically by running the `python2` and `python3`
interpreters. Additionally, NumPy must be available on your system, too.

Bazel can build Python code using either Python 2 or Python 3. The default is
Python 3, but each individual `py_binary` and `py_test` target can specify the
desired version using the
[`python_version`](https://docs.bazel.build/versions/master/be/python.html#py_test.python_version)
argument. The build rules need to make the local installation path of correct
version of Python available.

The default build rules should work for Debian and Ubuntu. They use Bazel's
[configurable attributes](https://docs.bazel.build/versions/master/be/common-definitions.html#configurable-attributes)
to provide paths for Python 2 and Python 3, respectively, based on which version
is required during a particular build.

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


