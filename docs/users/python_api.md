# Python environment API



The Python module `deepmind_lab` defines the `Lab` class. For example
usage, there is
[python/tests/dmlab_module_test.py](../../python/tests/dmlab_module_test.py),
[python/random_agent.py](../../python/random_agent.py), and
[python/random_agent_simple.py](../../python/random_agent_simple.py).

### module function `deepmind_lab.version`()

Prints the current version of *DeepMind Lab*. This information is not useful,
since we never update it.

### module functions `deepmind_lab.runfiles_path`(), `deepmind_lab.set_runfiles_path`(*path*)

These functions manipulate the module's "runfiles path", which is the path where
runtime dependencies of the module are located (such as the native code
libraries). Depending on how the module is deployed, you may need to set this
path before creating an environment.

### class `deepmind_lab.Lab`(*level*, *observations*, *config={}*, *renderer='software'*, *level_cache=None*)

Creates an environment object, loading the game script file *level*. The
environment's `observations`() method will return the observations specified by
name in the list *observations*.

The `config` dict specifies additional settings as key-value string pairs. The
following options are recognized:

| Option           | Description                                                                                    | Default |
| ---------------- | ---------------------------------------------------------------------------------------------- | ------: |
| `width`          | horizontal resolution of the observation frames                                                | `'320'` |
| `height`         | vertical resolution of the observation frames                                                  | `'240'` |
| `fps`            | frames per second                                                                              | `'60'`  |
| `levelDirectory` | optional path to level directory (relative paths are relative to game_scripts/levels)          | `''`    |
| `appendCommand`  | Commands for the internal Quake console\*                                                      | `''`    |
| `mixerSeed`      | value combined with each of the seeds fed to the environment to define unique subsets of seeds | `'0'`   |

\* See also [Lua map API](/docs/developers/reference/lua_api.md#commandlineold_commandline--string).

Unrecognized options are passed down to the level's init function. In Lua,
this is `kwargs.opts` in `api:init`.

For example, you can run a modified version of the lt_chasm level with these
calls,

```python
import deepmind_lab

observations = ['RGBD']
env = deepmind_lab.Lab('lt_chasm', observations,
                       config={'width': '640',    # screen size, in pixels
                               'height': '480',   # screen size, in pixels
                               'botCount': '2'},  # lt_chasm option.
                       renderer='hardware')       # select renderer.
env.reset()
```

Building with `--define graphics=<option>` sets which graphics implementation
is used.

`--define graphics=osmesa_or_egl`.

If no define is set then the build uses this config_setting at the default.

*   If `renderer` is set to `'software'` then osmesa is used for rendering.
*   If `renderer` is set to `'hardware'` then EGL is used for rendering.

`--define graphics=osmesa_or_glx`.

*   If `renderer` is set to `'software'` then osmesa is used for rendering.
*   If `renderer` is set to `'hardware'` then GLX is used for rendering.

`--define graphics=sdl`.

This will render the game to the native window. One of the observation starting
with 'RGB' must be in the `observations` for the game to render correctly.

#### Level Cache

A level cache can optionally be passed to `Lab(..., level_cache=...)` which
allows the reuse of levels that have already been seen without recompilation. It
can significantly improve performance for levels where a new map is generated
for each episode, such as explore_goal_locations_small.

The level cache object must have have the following two methods:

##### `bool fetch(self, key, pk3_path)`

Must return `True` if `key` was found in the level cache, otherwise `False`.
If `True`, then the cached level must be copied to `pk3_path`.

##### `write(self, key, pk3_path)`

The level that is to be cached for key `key` is located at `pk3_path`.

Example of a file-based level cache:

```python
import os.path
import shutil

class LevelCache(object):

  def __init__(self, cache_dir):
    self._cache_dir = cache_dir

  def fetch(self, key, pk3_path):
    path = os.path.join(self._cache_dir, key)

    if os.path.isfile(path):
      # Copy the cached file to the path expected by DeepMind Lab.
      shutil.copyfile(path, pk3_path)
      return True

    return False

  def write(self, key, pk3_path):
    path = os.path.join(self._cache_dir, key)

    if not os.path.isfile(path):
      # Copy the cached file DeepMind Lab has written to the cache directory.
      shutil.copyfile(pk3_path, path)
```

DeepMind Lab environment objects have the following methods:

### `reset`(*episode=-1*, *seed=None*)

Resets the environment to its initialization state. This method needs
to be called to start a new episode after the last episode ended
(which puts the environment into `is_running() == False` state).

The optional integer argument `episode` can be supplied to load the level in a
specific episode. If it isn't supplied or negative, episodes are loaded in
numerical order.

The optional integer argument `seed` can be supplied to seed the environment's
random number generator. If `seed` is omitted or `None`, a random number is
used.

The optional integer argument `mixerSeed` provided with the environment is
combined with every seed passed to this function. The resulting seeds span a
unique subset of the integers in \[0, 2^64\) for each different `mixerSeed`
value. However, the sequences produced by the environment's random number
generator are not necessarily disjoint.

### `num_steps`()

Number of frames since the last `reset`() call

### `is_running`()

Returns `True` if the environment is in running status, `False` otherwise.

### `step`(*action*, *num_steps=1*)

Advance the environment a number *num_steps* frames, executing the action
defined by *action* during every frame.

The Numpy array *action* should have dtype `np.intc` and should adhere to the
specification given in `action_spec`(), otherwise the behaviour is undefined.

### `observation_spec`()

Returns a list specifying the available observations *DeepMind Lab* supports,
including level specific custom observations.

```python
env = deepmind_lab.Lab('tests/empty_room_test', [])
observation_spec = env.observation_spec()
pprint.pprint(observation_spec)
# Outputs:
[{'dtype': <type 'numpy.uint8'>, 'name': 'RGB_INTERLEAVED', 'shape': (180, 320, 3)},
 {'dtype': <type 'numpy.uint8'>, 'name': 'RGBD_INTERLEAVED', 'shape': (180, 320, 4)},
 {'dtype': <type 'numpy.uint8'>, 'name': 'RGB', 'shape': (3, 180, 320)},
 {'dtype': <type 'numpy.uint8'>, 'name': 'RGBD', 'shape': (4, 180, 320)},
 {'dtype': <type 'numpy.uint8'>, 'name': 'BGR_INTERLEAVED', 'shape': (180, 320, 3)},
 {'dtype': <type 'numpy.uint8'>, 'name': 'BGRD_INTERLEAVED', 'shape': (180, 320, 4)},
 {'dtype': <type 'numpy.float64'>, 'name': 'MAP_FRAME_NUMBER', 'shape': (1,)},
 {'dtype': <type 'numpy.float64'>, 'name': 'VEL.TRANS', 'shape': (3,)},
 {'dtype': <type 'numpy.float64'>, 'name': 'VEL.ROT', 'shape': (3,)},
 {'dtype': <type 'str'>, 'name': 'INSTR', 'shape': ()},
 {'dtype': <type 'numpy.float64'>, 'name': 'DEBUG.POS.TRANS', 'shape': (3,)},
 {'dtype': <type 'numpy.float64'>, 'name': 'DEBUG.POS.ROT', 'shape': (3,)},
 {'dtype': <type 'numpy.float64'>, 'name': 'DEBUG.PLAYER_ID', 'shape': (1,)},
# etc...
```

The `observation_spec` returns the name, type and shape of the tensor or string
that will be returned if that spec name is specified in the observation list.

Example:

```python
{
    'name': 'RGB_INTERLEAVED',      ## Name of observation.
    'dtype': <type 'numpy.uint8'>,  ## Data type array.
    'shape': (180, 320, 3)          ## Shape of array. (Height, Width, Colors)
}
```

If the 'dtype' is `<type 'str'>` then a string is returned instead. If the a
dimension of any rank is dynamic or unknown until runtime then 0 is returned. If
the rank is unknown then the shape is an empty tuple.

### `events`()

Returns a list of events that has occurred since the last call to `reset`() or
`step`(). Each event is a tuple of a name, and a list of observations.

### `fps`()

An advisory metric that correlates discrete environment steps
("frames") with real (wallclock) time: the number of frames per (real) second.

### `action_spec`()

Returns a dict specifying the shape of the actions expected by `step`():

```python
env = deepmind_lab.Lab('tests/empty_room_test', [])
action_spec = env.action_spec()
pprint.pprint(action_spec)
# Outputs:
# [{'max': 512, 'min': -512, 'name': 'LOOK_LEFT_RIGHT_PIXELS_PER_FRAME'},
#  {'max': 512, 'min': -512, 'name': 'LOOK_DOWN_UP_PIXELS_PER_FRAME'},
#  {'max': 1, 'min': -1, 'name': 'STRAFE_LEFT_RIGHT'},
#  {'max': 1, 'min': -1, 'name': 'MOVE_BACK_FORWARD'},
#  {'max': 1, 'min': 0, 'name': 'FIRE'},
#  {'max': 1, 'min': 0, 'name': 'JUMP'},
#  {'max': 1, 'min': 0, 'name': 'CROUCH'}]
```

### `observations`()

Returns a dict, with every observation type passed at initialization
as a Numpy array:

```python
env = deepmind_lab.Lab('tests/empty_room_test', ['RGBD'])
env.reset()
obs = env.observations()
obs['RGBD'].dtype
# => dtype('int64')
```

### `close`()

Closes the environment and releases the underlying Quake III Arena
instance. The only method call allowed for closed environments is
`is_running`().


