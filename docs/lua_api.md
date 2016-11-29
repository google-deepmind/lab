(Switch to: Lua &middot; [Python](python_api.md) &middot;
 [Level Generation](level_generation.md) &middot;
 [Tensor](tensor.md) &middot; [Text Levels](text_level.md) &middot;
 [Build](build.md) &middot;
 [Known Issues](issues.md))

# DeepMind Lab environment documentation: Lua

## Lua callbacks

*DeepMind Lab* provides a number of ways for Lua callback functions to hook into
various game events.

On construction of the environment, the `levelName` setting specifies a game
script file name. Game script files live in the `game_scripts` directory
and end in `.lua`. They consist of Lua code that should return an object
implementing some of the API functions below.

The environment will periodically attempt to call these functions in order to
determine its behaviour. However, in most cases there is some appropriate
default behaviour that takes place if the corresponding API function is not
defined.

In order to successfully load a map, at least `nextMap` should be provided by
the game script.

The calling code that calls into these API functions can be found in
[deepmind/engine/context.cc](deepmind/engine/context.cc).

### `addBots`() &rarr; array

Called at beginning of the level to populate the level with in-game
bots. Returns an array of tables, each of which has a `name` and `skill`
entry. Each `skill` should be a number between `1` and `5`.

### `canPickup`(*entity_id*) &rarr; boolean

The environment calls this function to decide whether the item with ID
`entity_id` is allowed to be picked up. Return `false` to disallow
pickup. Defaults to `true` if the callback function isn't implemented.

### `commandLine`(*old_commandline*) &rarr; string

The environment calls this function at the very beginning to determine the
initial engine console commands. The `old_commandline` parameter is the default
value, which you should return as-is if you do not want to customize the command
sequence.

Example:

```lua
function api:commandLine(old_command_line)
  return "+set r_fullscreen \"1\""
end
```

Lists of available commands in *Quake III Arena* can be found online.

### `createPickup`(*class_name*) &rarr; table

Returns a table with keys `name`, `class_name`, `model_name`, `quantity`,
`type`, and an optional `tag`.

## `customObservation`(*name*) &rarr; tensor.ByteTensor or tensor.DoubleTensor.

(For information on *DeepMind Lab*'s tensor library, see [Tensor](tensor.md).)

When called it must return a tensor of a matching shape and type as specified in
`customObservationSpec`.

Example:

```lua
local order = 'Find Apples!'
local observationTable = {
    LOOK_PITCH = tensor.Tensor{0},
    ORDER = tensor.ByteTensor(order:byte(1, -1)),
    LOCATION = tensor.Tensor{0, 0, 0},
}

function api:customObservation(name)
  return observationTable[name]
end
```

## `customObservationSpec`() &rarr; array

Called after `init`, it returns an additional array of extra
observation types supplied by the script.

Each entry must contain a `name`, `type` and `shape`.

*   `name` Name of the observation reported by the environment.
*   `type` Type of tensor returned by the environment. May only be "bytes" or
    "doubles".
*   `shape` An array of integers denoting the shape of the tensor. If the size
    of any dimension can change between calls, it must be set to zero here.

```lua
-- See customObservation how to implement these.
function api:customObservationSpec()
  return {
    {name = 'LOCATION', type = 'doubles', shape = {3}},
    {name = 'ORDER', type = 'bytes', shape = {0}},
    {name = 'LOOK_PITCH', type = 'doubles', shape = {1}},
  }
end
```

When this function is specified the environment may call `observation` with any
`name` specified in the spec.

### `init`(*settings*) &rarr; nil

Called at game startup with the table *settings* representing a key-value store
for all settings passed at the start of the environment which weren't consumed
by the engine itself. See [python_api.md](python_api.md#python-environment-api) for
settings consumed by the engine.

Example:

```lua
function api:init(settings)
  print("Script Settings")
  for k, v in pairs(settings) do
    print(k .. " = " .. v)
  end
  io.flush()
end
```

### `hasEpisodeFinished`(*time_in_seconds*) &rarr; boolean

Called at the end of every frame with the elapsed time as an argument
to determine if the episode has finished and the game evaluation
should continue with the next episode.

The default implementation returns `false` until 5 minutes have passed.

### `nextMap`() &rarr; string

Called whenever a map needs to be loaded. Must return the name of a map
discoverable by the engine.

For more information on maps, see [Maps](#maps) below.

### `pickup`(*spawn_id*) &rarr; number

Event handler when the item with id *spawn_id* is picked up. Returns the respawn
time.

### `start`(*episode*, *seed*) &rarr; nil

The environment calls this function at the start of each episode, with:

*   A number *episode*, starting from 0.
*   A number *seed*, the random seed of the game engine. Supplying the same seed
    is intended to result in reproducible behaviour.

Example:

```lua
function api:start(episode, seed)
  print("Entering episode no. " .. episode .. " with seed " .. seed)
  io.flush()
end
```
### `updateSpawnVars`(*spawn_vars*) &rarr; table

Called once per `spawn_vars` entry when a new map is loaded. The `spawn_vars`
argument is a Lua table with the internal Quake III Arena `spawnVars` key-value
setting. Quake III Arena [maps](level_generation.md) define a list of entities
(light sources, player start points, items, ...). Implementing this function
allows overriding the settings for each map entity.

The default implementation returns `spawn_vars` unchanged.

## Lua common helper objects

A number of helper functions can be found in `game_scripts/common`. They
can be used in game scripts via Lua's `require` statement, for example:

```lua
local make_map = require 'common.make_map'

function api:nextMap()
  return make_map.makeMap("G I A P", "my_map")
end
```

### `common.make_map`

Returns a Lua object offering a [`makeMap`](#makemapmap_text-map_name)
and a [`commandLine`](#commandlineold_commandline--string) function.

### `common.pickups`

Returns a Lua object offering a `type` and `defaults` table for
defining pickup objects.

## The game module

Game scripts can interact with *DeepMind Lab* using the `dmlab.system.game`
module, which can be loaded using `local game = require 'dmlab.system.game'`.
The module provides the following functions.

### `addScore`(*player_id*, *score*)

Adds a *score* to the total score of the player with *player_id*.

### `finishMap`()

Finishes the current map.

## Factory functions

*DeepMind Lab* provides a number of Lua files with auxiliary helper functions in
the `game_scripts/helpers` directory. Among them are *factories*, which are of
the form

``` lua
local factory = {}

function factory.createLevelApi(kwargs)
  local api = {}
  -- ...
  return api
end

return factory
```

For examples, see any `game_scripts/helpers/*_factory.lua` file. Factory
functions can be used as a higher-level API for creating game scripts. As an
example, the `lt_chasm.lua` game script is implemented via

``` lua
local factory = require 'helpers.lt_factory'
return factory.createLevelApi{mapName = "lt_chasm"}
```

## Maps

*DeepMind Lab* maps are identified by strings representing a file name. This
file must contain a Quake III Arena map, see
[Level Generation](level_generation.md) for a description of
those. *DeepMind Lab* also defines a [text level](text_level.md)
format which provides a simple text format for creating levels. The Lua module
`game_scripts/common/make_map.lua` provides a convenient interface for making
maps from ASCII text level strings on the fly, namely

### `makeMap`(*map_text*, *map_name*)

Generates a map called *map_name* from *map_text*. The map name acts as a file
name and should be unique. For a description of the text level format that
*map_text* uses, see [Text Levels](text_level.md).

An example usage of `makeMap` is

``` lua
local make_map = require 'common.make_map'
...
function api:nextMap()
  map_text = "G I A P"
  api._count = api._count + 1
  return make_map.makeMap(map_text, "luaMap" .. api._count)
end
```

where `api._count` would have been set up in `start`. The `tests/demo_map.lua`
game script file provides a self-contained example.

## Rendering

### `screenMessages`(*args*)

Called to enable the script to render text on the screen at arbitrary locations.

`args` is a table containing:

*   `width` - Virtual screen width. (Always 640.)
*   `height` - Virtual screen height. (Always 480.)
*   `line_height` - Distance to move vertically between lines. (Always 20.)
*   `max_string_length` - Maximum length of string per message (Always 79.)

The user must return an array of messages. Each message shall contain:

*   `message` - String for the screen to render.
*   `x` - X location to render the string. (0 is left edge, 640 is right.)
*   `y` - Y location to render the string. (0 is top edge, 480 is bottom.)
*   `alignment` - 0 left aligned, 1 right aligned, 2 center aligned.

Helpers for rendering some message types are in `common.screen_message`:

```lua
local screen_message = require 'common.screen_message'

function api:screenMessages(args)
  local message_order = {
      message = 'Find an apple!',
      x = args.width / 2,
      y = (args.height - args.line_height) / 2,
      alignment = screen_message.ALIGN_CENTER,
  }
  return { message_order }
end
```
