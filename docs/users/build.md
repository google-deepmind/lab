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

### Step-by-step instructions for Debian or Ubuntu

Tested on Debian 8.6 (Jessie) and Ubuntu 14.04 (Trusty) and newer.

1. Install Bazel (see above).

2. Install *DeepMind Lab*'s dependencies:

   ```shell
   $ sudo apt-get install lua5.1 liblua5.1-0-dev libffi-dev gettext \
       freeglut3-dev libsdl2-dev libosmesa6-dev python-dev python-numpy \
       python-pil realpath
   ```

3. [Clone or download *DeepMind Lab*](https://github.com/deepmind/lab).

4. Build *DeepMind Lab* and run a random agent:

   ```shell
   $ cd lab

   # Build the Python interface to DeepMind Lab
   lab$ bazel build :deepmind_lab.so

   # Build and run the tests for it
   lab$ bazel run :python_module_test

   # Run a random agent
   lab$ bazel run :python_random_agent
   ```

The Bazel target `:deepmind_lab.so` builds the Python module that interfaces
with *DeepMind Lab*.

The random agent target `:python_random_agent` has a number of optional command line
arguments. Run `bazel run :random_agent -- --help` to see those.

### Building on Red Hat Enterprise Linux Server

Tested on release 7.2 (Maipo). This should also work on Centos 7, and with some
modifications of the package installation commands on Centos 6.

1. Install Bazel (see above).

2. Install Bazel's and DeepMind Lab's dependencies

   ```shell
   sudo yum -y install unzip java-1.8.0-openjdk lua lua-devel libffi-devel zip \
     java-1.8.0-openjdk-devel gcc gcc-c++ freeglut-devel SDL2 SDL2-devel \
     mesa-libOSMesa-devel python-devel python-imaging numpy
   ```

3. [Clone or download *DeepMind Lab*](https://github.com/deepmind/lab).

4. Edit `lua.BUILD` to reflect how Lua is installed on your system:

   ```python
   cc_library(
       name = "lua",
       linkopts = ["-llua"],
       visibility = ["//visibility:public"],
   )
   ```
   The output of `pkg-config lua --libs --cflags` might be helpful to find the
   right include folders and linker options.

5. Build *DeepMind Lab* using Bazel as above.

### Building on SUSE Linux

Tested on SUSE Linux Enterprise Server 12.

1. Install Bazel (see above).

2. Install Bazel's and DeepMind Lab's dependencies

   ```shell
   sudo zypper --non-interactive install java-1_8_0-openjdk \
     java-1_8_0-openjdk-devel gcc gcc-c++ lua lua-devel python-devel \
     python-numpy-devel python-imaging libSDL-devel libOSMesa-devel freeglut-devel
   ```

3. [Clone or download *DeepMind Lab*](https://github.com/deepmind/lab).

4. Edit `lua.BUILD` to reflect how Lua is installed on your system:

   ```python
   cc_library(
       name = "lua",
       linkopts = ["-llua"],
       visibility = ["//visibility:public"],
   )
   ```
   The output of `pkg-config lua --libs --cflags` might be helpful to find the
   right include folders and linker options.

5. Edit `python.BUILD` to reflect how Python is installed on your system:

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
   helpful to find the rihgt include folders.

6. Build *DeepMind Lab* using Bazel as above.


