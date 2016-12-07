(Switch to: [Lua](lua_api.md) &middot; [Python](python_api.md) &middot;
 [Level Generation](level_generation.md) &middot;
 [Tensor](tensor.md) &middot; [Text Levels](text_level.md) &middot;
 Build &middot;
 [Known Issues](issues.md))

# How to build *DeepMind Lab*

*DeepMind Lab* uses [Bazel](https://www.bazel.io/) as its build system. Its main
`BUILD` file defines a number of *build targets* and their dependencies. The
build rules should work out of the box on Debian (Jessie or newer) and Ubuntu
(version 14.04 or newer), provided the required packages are installed.
*DeepMind Lab* also builds on other Linux systems, but some changes to the build
files might be required, see below.

*DeepMind Lab* is written in C99 and C++11, and you will need a sufficiently
modern compiler. GCC 4.8 should suffice.

### Step-by-step instructions for Debian or Ubuntu

Tested on Debian 8.6 (Jessie) and Ubuntu 14.04 (Trusty) and newer.

1. Install Bazel by adding a custom APT repository, as described
   [on the Bazel homepage](http://bazel.io/docs/install.html#ubuntu) or using
   an [installer](https://github.com/bazelbuild/bazel/releases).
   This should also install GCC and zip.

2. Install *DeepMind Lab*'s dependencies:

   ```shell
   $ sudo apt-get install lua5.1 liblua5.1-0-dev libffi-dev gettext \
       freeglut3-dev libsdl2-dev libosmesa6-dev python-dev python-numpy realpath
   ```

3. [Clone or download *DeepMind Lab*](https://github.com/deepmind/lab).

4. Build *DeepMind Lab* and run a random agent:

   ```shell
   $ cd lab
   # Build the Python interface to DeepMind Lab with OpenGL
   lab$ bazel build :deepmind_lab.so --define headless=glx
   # Build and run the tests for it
   lab$ bazel run :python_module_test --define headless=glx
   # Rebuild the Python interface in non-headless mode and run a random agent
   lab$ bazel run :random_agent --define headless=false
   ```

The Bazel target `:deepmind_lab.so` builds the Python module that interfaces
*DeepMind Lab*. It can be build in headless hardware rendering mode (`--define
headless=glx`), headless software rendering mode (`--define headless=osmesa`) or
non-headless mode (`--define headless=false`).

The random agent target `:random_agent` has a number of optional command line
arguments. Run

``` shell
lab$ bazel run :random_agent -- --help
```

to see those.

### Building on Red Hat Enterprise Linux Server

Tested on release 7.2 (Maipo).

1. Add the Extra Packages as described on
   [fedoraproject.org](http://fedoraproject.org/wiki/EPEL#How_can_I_use_these_extra_packages.3F)
2. Install Bazel's and DeepMind Lab's dependencies

   ```shell
   sudo yum -y install unzip java-1.8.0-openjdk lua lua-devel libffi-devel zip \
     java-1.8.0-openjdk-devel gcc gcc-c++ freeglut-devel SDL2 SDL2-devel \
     mesa-libOSMesa-devel python-devel numpy
   ```
3. Download and run
   a [Bazel binary installer](https://github.com/bazelbuild/bazel/releases),
   e.g.

   ```shell
   sudo yum -y install wget
   wget https://github.com/bazelbuild/bazel/releases/download/0.3.2/bazel-0.3.2-installer-linux-x86_64.sh
   sh bazel-0.3.2-installer-linux-x86_64.sh
   ```
4. [Clone or download *DeepMind Lab*](https://github.com/deepmind/lab).
5. Edit `lua.BUILD` to reflect how Lua is installed on your system:

   ```python
   cc_library(
       name = "lua",
       linkopts = ["-llua"],
       visibility = ["//visibility:public"],
   )
   ```
   The output of `pkg-config lua --libs --cflags` might be helpful to find the
   right include folders and linker options.
6. Build *DeepMind Lab* using Bazel as above.

### Building on SUSE Linux

Tested on SUSE Linux Enterprise Server 12.

1. Install Bazel's and DeepMind Lab's dependencies

   ```shell
   sudo zypper --non-interactive install java-1_8_0-openjdk \
     java-1_8_0-openjdk-devel gcc gcc-c++ lua lua-devel python-devel \
     python-numpy-devel libSDL-devel libOSMesa-devel freeglut-devel
   ```
2. Download and run
   a [Bazel binary installer](https://github.com/bazelbuild/bazel/releases),
   e.g.

   ```shell
   sudo yum -y install wget
   wget https://github.com/bazelbuild/bazel/releases/download/0.3.2/bazel-0.3.2-installer-linux-x86_64.sh
   sh bazel-0.3.2-installer-linux-x86_64.sh
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
