"""The DeepMind Lab native module and dm_env bindings.

The native module is loaded from the DSO deepmind_lab.so. It provides
complete access to the functionality of DeepMind Lab. The API of this
module and the "Lab" type is documented in docs/users/python_api.md.
Use it as follows:

  import deepmind_lab

  lab = deepmind_lab.Lab(level='lt_chasm', observations=['RGB'])
  # ...

The "dm_env bindings" module provides bindings to the "dm_env" API. The module
is exposed as "deepmind_lab.dmenv_module". It requires the "dm_env" module; the
PIP "dmenv_module" extra describes that dependency. Only a subset of features
is accessible this way. Use the module as follows:

  from deepmind_lab import dmenv_module as dm_env_lab

  lab = dm_env_lab.Lab(level='lt_chasm', observation_names=['RGB'], config={})
  # ...
"""

import imp
from deepmind_lab import dmenv_module
import pkg_resources

_deepmind_lab = imp.load_dynamic(
    __name__, pkg_resources.resource_filename(__name__, 'deepmind_lab.so'))

Lab = _deepmind_lab.Lab  # needed from within dmenv_module
