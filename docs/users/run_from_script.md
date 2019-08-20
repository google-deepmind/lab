# How to Run *DeepMind Lab* from a Script

*DeepMind Lab* can be run,

 * from a [Python script](#running-with-python)

It can also be invoked,

* from a [C program](#running-with-c)


## Running with Python




For a complete example script, please see,
[examples/game_main.py](../../examples/game_main.py).

Depending on how the `deepmind_lab` module has been deployed, you may need to
set the module's runfiles path using `deepmind_lab.set_runfiles_path`; e.g. see
`game_main.py`'s main code near the end.

Construct the environment, step once, and retrieve an observation,

```python
import deepmind_lab

# Construct and start the environment.
env = deepmind_lab.Lab('seekavoid_arena_01', ['RGB_INTERLEAVED'])
env.reset()

# Create all-zeros vector for actions.
action = np.zeros([7], dtype=np.intc)

# Advance the environment 4 frames while executing the all-zeros action.
reward = env.step(action, num_steps=4)

# Retrieve the observations of the environment in its new state.
obs = env.observations()  # dict of Numpy arrays
rgb_i = obs['RGB_INTERLEAVED']
assert rgb_i.shape == (240, 320, 3)
```

Initialized environments can be asked for the list of available observations,

```python
import pprint
env = deepmind_lab.Lab('seekavoid_arena_01', [])
observation_spec = env.observation_spec()
pprint.pprint(observation_spec)
# Outputs:
# [{'dtype': <type 'numpy.uint8'>,
#   'name': 'RGB_INTERLEAVED',
#   'shape': (240, 320, 3)},
#  {'dtype': <type 'numpy.uint8'>,
#   'name': 'RGBD_INTERLEAVED',
#   'shape': (240, 320, 4)},
#  {'dtype': <type 'numpy.uint8'>, 'name': 'RGB', 'shape': (3, 240, 320)},
#  {'dtype': <type 'numpy.uint8'>, 'name': 'RGBD', 'shape': (4, 240, 320)},
#  {'dtype': <type 'numpy.float64'>, 'name': 'VEL.TRANS', 'shape': (3,)},
#  {'dtype': <type 'numpy.float64'>, 'name': 'VEL.ROT', 'shape': (3,)}]
```

Please see the
[Python Reference](/docs/users/python_api.md)
for complete listing of all environment functions.


## Running with C

For an example of doing the same thing with the C API, please see,
[examples/game_main.c](../../examples/game_main.c).
