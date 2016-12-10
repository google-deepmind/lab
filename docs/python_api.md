(Switch to: [Lua](lua_api.md) &middot; Python &middot;
 [Level Generation](level_generation.md) &middot;
 [Tensor](tensor.md) &middot; [Text Levels](text_level.md) &middot;
 [Build](build.md) &middot;
 [Known Issues](issues.md))

# DeepMind Lab environment documentation: Python

## Environment usage in Python

Constructing the environment, doing one step and retrieving one observation:

```python
import deepmind_lab

# Construct and start the environment.
lab = deepmind_lab.Lab('seekavoid_arena_01', ['RGB_INTERLACED'])
lab.reset()

# Create all-zeros vector for actions.
action = np.zeros([7], dtype=np.intc)

# Advance the environment 4 frames while executing the all-zeros action.
reward = env.step(action, num_steps=4)

# Retrieve the observations of the environment in its new state.
obs = env.observations()  # dict of Numpy arrays
rgb_i = obs['RGB_INTERLACED']
assert rgb_i.shape == (240, 320, 3)
```

For an example of doing the same thing with the C API, take a look at
[examples/game_main.c](../examples/game_main.c).

Initialised environments can be asked for the list of available observations:

```python
import pprint
lab = deepmind_lab.Lab('seekavoid_arena_01', [])
observation_spec = lab.observation_spec()
pprint.pprint(observation_spec)
# Outputs:
# [{'dtype': <type 'numpy.uint8'>,
#   'name': 'RGB_INTERLACED',
#   'shape': (240, 320, 3)},
#  {'dtype': <type 'numpy.uint8'>,
#   'name': 'RGBD_INTERLACED',
#   'shape': (240, 320, 4)},
#  {'dtype': <type 'numpy.uint8'>, 'name': 'RGB', 'shape': (3, 240, 320)},
#  {'dtype': <type 'numpy.uint8'>, 'name': 'RGBD', 'shape': (4, 240, 320)},
#  {'dtype': <type 'numpy.float64'>, 'name': 'VEL.TRANS', 'shape': (3,)},
#  {'dtype': <type 'numpy.float64'>, 'name': 'VEL.ROT', 'shape': (3,)}]
```

For full documentation of the Python environment API, see below.

## Python environment API

The Python module `deepmind_lab` defines the `Lab` class. For example
usage, there is [python/dmlab_module_test.py](../python/dmlab_module_test.py)
and [python/random_agent.py](../python/random_agent.py).

### class `deepmind_lab.Lab`(*level*, *observations*, *config={}*)

Creates an environment object, loading the game script file *level*. The
environment's `observations`() method will return the observations specified by
name in the list *observations*.

The `config` dict specifies additional settings as key-value string pairs. The
following options are recognized:

Option         | Description                                     | Default value
-------------- |-------------------------------------------------| -------------:
`width`        | horizontal resolution of the observation frames |       `'320'`
`height`       | vertical resolution of the observation frames   |       `'240'`
`fps`          | frames per second                               |        `'60'`
`appendCommand` | commands for the internal Quake console, see also the [Lua map API](lua_api.md#commandlineold-commandline-string) | `''`

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

Returns a list specifying the available observations *DeepMind Lab* supports.
See above for an example.

### `fps`()

An advisory metric that correlates discrete environment steps
("frames") with real (wallclock) time: the number of frames per (real) second.

### `action_spec`()

Returns a dict specifying the shape of the actions expected by `step`():

```python
lab = deepmind_lab.Lab('tests/demo_map', [])
action_spec = lab.action_spec()
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
lab = deepmind_lab.Lab('tests/demo_map', ['RGBD'])
lab.reset()
obs = env.observations()
obs['RGBD'].dtype
# => dtype('int64')
```

### `close`()

Closes the environment and releases the underlying Quake III Arena
instance. The only method call allowed for closed environments is
`is_running`().
