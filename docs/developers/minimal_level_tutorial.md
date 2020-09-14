# Building a Minimal Level


Tasks written for DMLab normally span two Lua scripts: a *factory* which defines
a range of possible levels, and a *level* which names a particular set of
parameters. For example, `game_scripts/levels/emstm_eat.lua` parameterizes
`game_scripts/factories/em_eat_factory.lua`. Several levels can use the same
factory, whether they're exploring different parts of the parameter space or
explicitly tracking versions.

Our level script, `game_scripts/levels/demos/simple_demo.lua`, is tiny:

```lua
local factory = require 'factories.simple_demo_factory'

return factory.createLevelApi{
    episodeLengthSeconds = 12,
}
```

There's some convenient boilerplate to write for
`game_scripts/factories/simple_demo_factory.lua`:

```lua
local custom_observations = require 'decorators.custom_observations'
local game = require 'dmlab.system.game'

local factory = {}

function factory.createLevelApi(kwargs)
  assert(kwargs.episodeLengthSeconds)
  local api = {}

  custom_observations.decorate(api)
  return api
end

return factory
```

`kwargs` here (short for 'keyword arguments') are the default settings specified
in your `simple_demo.lua` script.


This creates a factory, but the level will fail to run, because you can't start
an episode without a map. The map should be set in `api:nextMap()`, but we'll
often use one map throughout an entire episode, creating it in `api:start()`:

```lua
local make_map = require 'common.make_map'
local map_maker = require 'dmlab.system.map_maker'
local random = require 'common.random'
local randomMap = random(map_maker:randomGen())
```

```lua
  function api:nextMap()
    return self._map
  end

  function api:start(episode, seed)
    random:seed(seed)
    randomMap:seed(random:mapGenerationSeed())

    self._map = make_map.makeMap{
        mapName = 'WideOpenLevel',
        mapEntityLayer =
            '***********\n' ..
            '*        P*\n' ..
            '*         *\n' ..
            '*         *\n' ..
            '*         *\n' ..
            '*         *\n' ..
            '*         *\n' ..
            '*         *\n' ..
            '*         *\n' ..
            '*         *\n' ..
            '***********\n',
    }
  end
```

NOTE: `api:start()` is called once per episode; `api:nextMap()` may be called
multiple times, for levels which "restart" the agent multiple times within a
single episode.

And with that, we can run around a wide open empty room forever!

## Timeouts

"Wait," you say, "you said `episodeLengthSeconds` is mandatory!" Yes, it is, but
we haven't yet wired it up to the timeout decorator. The preferred way to do so
is to use `setting_overrides`, which handles other kwargs manipulations as well.
Making that change:

```lua
local setting_overrides = require 'decorators.setting_overrides'
```

```lua
  custom_observations.decorate(api)
  setting_overrides.decorate{
      api = api,
      apiParams = kwargs,
      decorateWithTimeout = true
  }
  return api
end
```

Now your life as a human agent player is short, if not nasty or brutal; you may
want to increase `episodeLengthSeconds` in simple_demo.lua.

## Restarts

Restarting can take a while to reload data; if you specify an empty string as
the map, DMLab calls that a 'quick restart' and doesn't reload anything, keeping
the current state in memory and just respawning the player and entities as
necessary. This is frequently controlled by a boolean option:

```lua
  function api:nextMap()
    if kwargs.quickRestart then
      self._map = ''
    end
    return self._map
  end
```

How do we trigger that option? One way would be to update our `simple_demo.lua`;
arguments can also be overridden on the command line. Our `createLevelApi()`
function does *not* see these overrides; they're processed in the
`setting_overrides` decorator, and seen during or after `api:init()`.

BEST PRACTICE: Any work done by `createLevelApi()` only takes into account the
per-level defaults. If you want to respect command-line overrides you'll need to
defer work to `api:init()`.

## Maps

Let's make the map more interesting:

```lua
        mapName = 'FormerlyWideOpenLevel',
        mapEntityLayer =
            '***********\n' ..
            '*** IGGGI *\n' ..
            '***G*PPP* *\n' ..
            '*GP IPGP* *\n' ..
            '*PGG*H*** *\n' ..
            '*   IPGP* *\n' ..
            '*P P*PGG* *\n' ..
            '*GP *PGP* *\n' ..
            '***H***H* *\n' ..
            '***       *\n' ..
            '***********\n'
```

Here

*   '\*' is a wall
*   'G' is a spawn point for goal objects
*   'H' is a north-south door
*   'I' is an east-west door
*   'P' is a spawn point for the player

Run the game again, and you will see a more complicated map, with doors which
open as you pass through them. However, 'G' sites haven't spawned any goals.

## Pickups

This whole time DMLab has been *trying* to spawn goals, but you didn't tell it
how. Do that (minimally) now:

```lua
local pickups = require 'common.pickups'
```

```lua
  function api:createPickup(className)
    return pickups.defaults[className]
  end
```

If you look at `common/pickups.lua`, you can see several different options
defined there, but cruising around this version of the level all you'll see are
blobby orange objects, touching any one of which ends the episode and starts
another with the agent at a new spawn position.

Let's address the issue of object variety. 'H', 'I', and 'P' are hardwired, but
the other objects are controlled by a level-specific callback. If your level
doesn't specify one, the default callback in `make_map.makeMap()` is used, which
defines 'A' for apple_reward and 'G' for goal.

You can override the list of simple definitions by specifying `kwargs.pickups`,
or write your own function as `kwargs.callback`. We'll keep it simple, and
replace some of the initial 'G' instances to get a much more varied diet:

```lua
  function api:start(episode, seed)
    random:seed(seed)
    randomMap:seed(random:mapGenerationSeed())

    self._map = make_map.makeMap{
        mapName = 'WideOpenLevel',
        mapEntityLayer =
            '***********\n' ..
            '*** IAFGI *\n' ..
            '***L*PPP* *\n' ..
            '*MP IPSP* *\n' ..
            '*PWA*H*** *\n' ..
            '*   IPFP* *\n' ..
            '*P P*PGL* *\n' ..
            '*MP *PSP* *\n' ..
            '***H***H* *\n' ..
            '***       *\n' ..
            '***********\n',
        pickups = {
            A = 'apple_reward',
            F = 'fungi_reward',
            G = 'goal',
            L = 'lemon_reward',
            M = 'mango_goal',
            S = 'strawberry_reward',
            W = 'watermelon_goal',
        }
    }
  end
```

## Maps II

To dynamically generate maps, we can require dmlab.system.maze_generation. Most
levels that use it call `randomMazeGeneration()` to get a different maze on
every episode (or every restart!), but we'll do something a bit different:

```lua
local maze_gen = require 'dmlab.system.maze_generation'
```

```lua
    self._maze = maze_gen.mazeGeneration{
        entity =
            '***********\n' ..
            '*** IAFGI *\n' ..
            '***L*PPP* *\n' ..
            '*MP IPSP* *\n' ..
            '*PWA*H*** *\n' ..
            '*   IPFP* *\n' ..
            '*P P*PGL* *\n' ..
            '*MP *PSP* *\n' ..
            '***H***H* *\n' ..
            '***       *\n' ..
            '***********\n',
    }
    self._map = make_map.makeMap{
        mapName = 'WideOpenLevel',
        mapEntityLayer = self._maze:entityLayer(),
        pickups = {
            A = 'apple_reward',
            F = 'fungi_reward',
            G = 'goal',
            L = 'lemon_reward',
            M = 'mango_goal',
            S = 'strawberry_reward',
            W = 'watermelon_goal',
        }
    }
```

Here we aren't making use of any of maze_generation's random generation
features, but we *are* hooking into debug support in the C++ code. This will
allow us to use a top-down overhead camera to monitor what our agent does while
attempting the task, if we add one more decorator.

```lua
local debug_observations = require 'decorators.debug_observations'
```

```lua
    debug_observations.setMaze(self._maze)
```


## Pickups II

For more complicated pickups, we can replace our `makeMap{pickups={...}}` table
with a callback. There is support for a wide range of human recognisable
objects; `demos/hrp_demo` is a simple example, `scavenger_hunt` or the
`language/` levels more complex. `demos/hrp_gallery` encapsulates the basic
workflow: a callback is passed to `map_maker.makeMap()` which calls
`common.human_recognisable_pickups.create{}`.
`decorators.human_recognisable_pickups` is added to the api to make sure all
necessary models and textures are loaded.


## Alternatives

Many of the levels provided with DMLab use a more complex approach of layered
factories. For example, `game_scripts/factories/explore` has five separate
factories built on top of `factory.lua`, which use a complex set of arguments
and a simplified API to create search tasks in procedural mazes:

*   `level:createPickup()`
*   `level:restart()`
*   `level.spawnVarsUpdater()`
*   `level:start()`

A more unusual example is `game_scripts/factories/language`. Levels can make use
of that `factory.lua` together with the utilities in `game_scripts/language` to
declaratively specify linguistic tasks through a set of constraints.
