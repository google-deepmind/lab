
# Build *DeepMind Lab* for macOS

For more information refer to [How to build DeepMind Lab](build.md).

### Prepare build environment

The following steps has to be done only once per system.

1. Make sure _Xcode Command Tools_ are installed:

   ```shell
   $ xcode-select --install
   ```

2. Install [Homebrew](https://brew.sh/):

   ```shell
   $ /usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
   ```

3. Install required packages via _Homebrew_:

   ```shell
   $ brew install bazel coreutils sdl2 lua glib libxml2 numpy
   ```

4. The `ln`, `cp` and `realpath` tools from Homebrew `coreutils` package must be made accessible by Bazel using one of the following approaches:

   1. Copy symlinks removing `g` prefix in `/usr/local/bin`:

      ```shell
      $ sudo cp /usr/local/bin/gln /usr/local/bin/ln
      $ sudo cp /usr/local/bin/gcp /usr/local/bin/cp
      $ sudo cp /usr/local/bin/grealpath /usr/local/bin/realpath
      ```

      Make sure that sure that `/usr/local/bin` is in `PATH` environment variable and is listed before `/bin`, i.e.,
      `$ which ln` should output `/usr/local/bin/ln`. If it does not, you may need to update your `~/.profile`:

      ```shell
      $ echo 'export PATH=/usr/local/bin:$PATH' >> ~/.profile
      ```

      and re-open your terminal window.

   2. Export `PATH` variable for each build and run session:

      ```shell
      export PATH=/usr/local/opt/coreutils/libexec/gnubin:$PATH
      ```

      **NOTE:** if you choose this approach, remember to execute the above command once per each terminal session used to build and run this project
      or you may add this line at the end of your `~/.profile` file for a permanent solution,
      however, in this case all tools from `coreutils` package will override the native system tools, which is not recommended.

   **NOTE:** because of a _Bazel_ [bug](https://github.com/bazelbuild/bazel/issues/4008) the `--action_env PATH=...` argument will not work for this.

5. For convenience you may want to make _macOS_ as the default target platform,
   you can do this by putting following lines in your `~/.bazelrc` or `tools/bazel.rc` at the project level:

   ```
   build --apple_platform_type=macos
   run --apple_platform_type=macos
   ```

   **NOTE:** if you decide not to put these lines in the `~/.bazelrc` or `tools/bazel.rc`,
   you will need to add `--apple_platform_type=macos` argument
   to each `run` and `build` invocation of _Bazel_ for this project.

   Additionally you could add `--cpu=darwin` or `--cpu=darwin_x86_64` argument,
   but it should be autodetected when building under _macOS_.

6. Optionally, for headless software rendering support you need to install _[OSMesa](https://www.mesa3d.org/)_,
   unfortunately there are no binary packages available, therefore you will need to build it from source.
   To facilitate building process of _OSMesa_, you can try the installation script from
   [devernay/osmesa-install](https://github.com/devernay/osmesa-install), but first you need to prepare target directories:

   ```shell
   $ sudo mkdir /opt/osmesa /opt/llvm
   $ sudo chown $USER:staff /opt/osmesa /opt/llvm
   ```

   Then run the script to build and install _OSMesa_:

   ```shell
   $ curl https://raw.githubusercontent.com/devernay/osmesa-install/master/osmesa-install.sh \
       | sed -e 's/mangled=1/mangled=0/' osmesa-install.sh | LLVM_BUILD=1 bash
   ```

### Build and run _DeepMind Lab_ for macOS

1. [Clone or download *DeepMind Lab*](https://github.com/deepmind/lab).

2. Build *DeepMind Lab* and run a random agent:

   ```shell
   $ cd lab
   # Build the Python interface to DeepMind Lab with OpenGL
   lab$ bazel build :deepmind_lab.so --define headless=macos
   # Rebuild the Python interface in non-headless mode and run a random agent
   lab$ bazel run :random_agent --define headless=false
   ```

   The Bazel target `:deepmind_lab.so` builds the Python module that interfaces
   *DeepMind Lab*. It can be build in headless hardware rendering mode (`--define
   headless=macos` on macOS), headless software rendering mode (`--define
   headless=osmesa`) or non-headless mode (`--define headless=false`).

   For more information refer to [How to build DeepMind Lab](build.md).

### Troubleshooting

* If got error message about missing `Python.h` header, then probably
  the _Xcode Command Line Tools_ are not installed, refer to step 1 of [Prepare build environment](#prepare-build-environment).

* If got error message about missing `Carbon.h` header,
  then you have to add `--apple_platform_type=macos` argument to invocation of _Bazel_ commands,
  for convenience you can put it in the `~/.bazelrc` or `tools/bazel.rc`,
  refer to step 5 of [Prepare build environment](#prepare-build-environment) for details.

* If got error messages like `ln: illegal option -- L`, `cp: illegal option -- t` or `/bin/bash: realpath: command not found`,
  then _Bazel_ cannot find the _GNU_ `ln`, `cp` or `realpath` tools,
  refer to step 4 of [Prepare build environment](#prepare-build-environment) for details.

* If got error message `ld: library not found for -lOSMesa`, then you probably did not specify `headless` parameter,
  depending on what you need, you have to specify one of the following: `--define headless=false`, `--define headless=macos` or
  `--define headless=osmesa`, refer to [Build and Run _DeepMind Lab_ for macOS](#build-and-run-deepmind-lab-for-macos) for details.
