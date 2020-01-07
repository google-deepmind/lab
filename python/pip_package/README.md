# DeepMind Lab Python Module

The DeepMind Lab Python module is the recommended way to use DeepMind Lab
outside of a Bazel project. You can create and run environments using the same
Python API that you would use with a Bazel project.

## Build and Install

The build and install process has the following steps:

- Install project dependencies.
- Build project assets and binaries with Bazel.
- Bundle project assets and binaries into a Python package.
- Install the package.

Here's the short version if you have already set up the dependencies.

```sh
git clone https://github.com/deepmind/lab.git && cd lab
bazel build -c opt --python_version=PY2 //python/pip_package:build_pip_package
./bazel-bin/python/pip_package/build_pip_package /tmp/dmlab_pkg
pip install /tmp/dmlab_pkg/deepmind_lab-1.0-py2-none-any.whl --force-reinstall
```

#### Dependencies

First, see [External dependencies, prerequisites and porting
notes](../../README.md#external-dependencies-prerequisites-and-porting-notes)
and install any requirements you might be missing. In addition to the
requirements listed there, you may also need:

- [pip](https://pip.pypa.io/en/stable/installing/), wheel and setuptools
- [virtualenv](https://virtualenv.pypa.io/en/stable/installation/) (optional)

If you use virtualenv, you may need to install NumPy again in the hermetic
environment (using pip). Instructions for doing so are provided below. Do beware
that the NumPy version should be compatible with and at least as new as the one
used to build the Python module itself. If necessary, change the `WORKSPACE` and
`python.BUILD` files to use the NumPy version from your PIP package. See the
[build documentation](../../docs/users/build.md#lua-and-python-dependencies) for
details.

#### Python 2 vs Python 3

To select the Python API against which the DeepMind Lab module is built, build
the PIP packaging script with the
[`--python_version`](https://docs.bazel.build/versions/master/command-line-reference.html#flag--python_version)
Bazel flag:

* For Python 2: `bazel build -c opt --python_version=PY2 //python/pip_package:build_pip_package`
* For Python 3: `bazel build -c opt --python_version=PY3 //python/pip_package:build_pip_package`

For Python 3, you will then probably want to invoke virtualenv with `virtualenv
--python=python3`, and you will want to set the Python binary path (e.g. via
`export PYTHON_BIN_PATH="/usr/bin/python3"`) when running the packaging script.
The resulting `.whl` file should contain the string `-py2-` or `-py3-` for a
Python 2 or Python 3 build, respectively.

As of Bazel 0.27, the default Python version is `PY3`.

#### Build assets/binaries

Following this are more detailed instructions on how to build and install if you
haven't followed the short version of the instructions above. To begin, if you
haven't already, clone DeepMind Lab.

```sh
$ git clone https://github.com/deepmind/lab.git
```

To build the prerequisite assets and binaries for the Python package, change to
the `lab` directory and run the Bazel command to build the pip package script:

```sh
$ cd lab
$ bazel build -c opt //python/pip_package:build_pip_package
```

If the build command fails, make sure you've grabbed the latest version and
double-check that you've installed all of the dependencies for DeepMind Lab,
then [file a bug](https://github.com/deepmind/lab/issues/new).

Keep in mind that for most changes you make to DeepMind Lab, including adding
new game scripts, models, textures, and code, that you will need to rebuild the
package script and perform the following installation instructions again.

#### Package assets/binaries

To create a Python package for DeepMind Lab run the following:

```sh
$ ./bazel-bin/python/pip_package/build_pip_package /tmp/dmlab_pkg
```
This script copies all of the relevant files for the package to a temporary
directory and bundles them up into the distribution file saved in the directory
specified. (It needs to be run from the root of the DeepMind Lab source
directory since it expects to find `bazel-bin` in the directory from which it is
called.)

#### Install

The recommended way to use the DeepMind Lab Python Module is with Virtualenv, a
tool that allows you to keep the dependencies for projects separate from one
another. Instructions provided are for building and installing with Virtualenv,
but you can skip these steps to install the package with your system-wide
packages.

First, create a virtual environment in your project directory:

```sh
$ cd ~/my_agent
$ virtualenv agentenv
```

You can also use `virtualenv --system-site-package agentenv` to allow the
environment to use your system-wide Python packages (e.g. NumPy), in which
case you do not need to install NumPy again below.

Once you've created your virtualenv, you can activate it using:

```sh
$ source agentenv/bin/activate
```

Once your virtualenv is activated, install any remaining dependencies (if you
have not used `--system-site-package` above):

```sh
(agentenv)$ pip install numpy
```

The package generation step will have created a `.whl` file in `/tmp/dmlab_pkg`.
This is the binary distribution file for DeepMind Lab. Install it using:

```sh
(agentenv)$ pip install /tmp/dmlab_pkg/deepmind_lab-1.0-py2-none-any.whl
```

After a successful install you're now ready to start using DeepMind Lab as a
standalone module.

Finally, when you're done using your virtualenv you can deactivate it by
running:

```sh
(agentenv)$ deactivate
```

#### Testing the Installation

Create a new file `agent.py` and add the following:

```python
import deepmind_lab
import numpy as np

# Create a new environment object.
lab = deepmind_lab.Lab("demos/extra_entities", ['RGB_INTERLEAVED'],
                       {'fps': '30', 'width': '80', 'height': '60'})
lab.reset(seed=1)

# Execute 100 walk-forward steps and sum the returned rewards from each step.
print(sum(
    [lab.step(np.array([0,0,0,1,0,0,0], dtype=np.intc)) for i in range(0, 100)]))
```

Run `agent.py`:

```sh
(agentenv)$ python agent.py
```

DeepMind Lab prints debugging/diagnostic info to the console, but at the end it
should print out a number showing the reward. For the map in the example, the
reward should be 4.0 (since there are four apples in front of the spawn).

#### Uninstall

If you make changes to any files that get bundled in the package then you can
either uninstall the old version of DeepMind Lab before installing the new
version, or invoke the above `pip install` command with the flag
`--force-reinstall`.

If you really want to say goodbye, just run:

```sh
$ pip uninstall deepmind_lab
```

If you've installed this to your virtualenv and to your system-wide packages you
will need to run this command once for each.

## DeepMind Lab Python API

You can use the same API for the standalone Python module as you do when
building with the Bazel project. See:
[DeepMind Lab environment documentation: Python](../../docs/users/python_api.md).

## Bindings for the DeepMind "dm_env" API

DeepMind has released a general API for reinforcement learning, called "dmenv":
https://github.com/deepmind/dm_env

We provide an adapter module, [`dmenv_module.py`](../dmenv_module.py),
that exposes DeepMind Lab through that API. The adaptor module is also included
in the above PIP package; if installed that way, the module can be imported with
`from deepmind_lab import dmenv_module`. The additional dependencies for that
module can be installed by requesting the PIP extra called `dmenv_module`, for
example:

```sh
pip install /tmp/dmlab_pkg/deepmind_lab-1.0-py2-none-any.whl[dmenv_module]
```

Note that not all DeepMind Lab features may be exposed through the dm_env API.
For example, at the time of writing, observation specs in dm_env do not allow
dynamic shapes, whereas DeepMind Lab's native API (the
[EnvCApi](../../third_party/rl_api/env_c_api.h)) does.
