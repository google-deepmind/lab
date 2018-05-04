# DeepMind Lab environment documentation: Lua

## Lua callbacks

*DeepMind Lab* provides a number of ways for Lua callback functions to hook into
various game events.

On construction of the environment, the `levelName` setting specifies a game
script file name. Game script files live in the `game_scripts/levels` directory
and end in `.lua`. They consist of Lua code that should return an object
implementing some of the API functions below.

The environment will periodically attempt to call these functions in order to
determine its behaviour. However, in most cases there is some appropriate
default behaviour that takes place if the corresponding API function is not
defined.

In order to successfully load a map, at least `nextMap` should be provided by
the game script.

The calling code that calls into these API functions can be found in:

*   [deepmind/engine/context.cc](../../../deepmind/engine/context.cc).
*   [deepmind/engine/context_actions.cc](../../../deepmind/engine/context_actions.cc).
*   [deepmind/engine/context_entities.cc](../../../deepmind/engine/context_entities.cc).
*   [deepmind/engine/context_events.cc](../../../deepmind/engine/context_events.cc).
*   [deepmind/engine/context_game.cc](../../../deepmind/engine/context_game.cc).
*   [deepmind/engine/context_observations.cc](../../../deepmind/engine/context_observations.cc).
*   [deepmind/engine/context_pickups.cc](../../../deepmind/engine/context_pickups.cc).

## Lua api callbacks order

All callback functions are invoked by the environment in a specific order:
![lua_api_callbacks_flowchart](images/lua_api_flowchart.png "Callbacks order")


### `addBots`() &rarr; array

Called at beginning of the level to populate the level with in-game bots.
Returns an array of tables, each of which has a `name` and `skill` entry. Each
`skill` should be a number between `1` and `5`.

### `gameType`() &rarr; int

Called before loading a map and sets the game type being played these can be
accessed from `common.game_types`. If not implemented FREE_FOR_ALL is returned.

```lua
local game_types = require 'common.game_types'

function api:gameType()
  return game_types.CAPTURE_THE_FLAG
end
```

If the game type is team based the team a player belongs to can be set via
`team`().

### `team`(*playerId*, *playerName*) &rarr; string

Called for each player joining the game. The team can be selected by returning
one of the following characters. A team must be selected if game mode is a team
game.

Character | Team
:-------: | ------------------
'p'       | Any Team (Default)
'r'       | Red Team
'b'       | Blue Team
's'       | Spectator

```lua
local TEAMS = {'r', 'b'}

function api:team(playerId, playerName)
  return TEAMS[playerId % 2 + 1]
end
```

### `playerModel`(*playerId*, *playerName*) &rarr; string

Called for each player joining the game. Each model/team combination has a
textures that can be overridden.

Model                 | team | Textures
:-------------------: | :--: | ---------------------------------------------
`'crash'`             | free | `'models/players/crash/skin1.tga'`
`'crash'`             | red  | `'models/players/crash/redskin.tga'`
`'crash'`             | blue | `'models/players/crash/blueskin.tga'`
`'crash_color'`       | all  | `'models/players/crash_color/skin_base.tga'`
`'crash_color/skin1'` | all  | `'models/players/crash_color/skin_base1.tga'`
`'crash_color/skin2'` | all  | `'models/players/crash_color/skin_base2.tga'`
`'crash_color/skin3'` | all  | `'models/players/crash_color/skin_base3.tga'`
`'crash_color/skin4'` | all  | `'models/players/crash_color/skin_base4.tga'`
`'crash_color/skin5'` | all  | `'models/players/crash_color/skin_base5.tga'`
`'crash_color/skin6'` | all  | `'models/players/crash_color/skin_base6.tga'`
`'crash_color/skin7'` | all  | `'models/players/crash_color/skin_base7.tga'`

```lua
local redTeamSkin = 'crash_color/skin1'
local blueTeamSkin = 'crash_color/skin2'
function api:playerModel(playerId, playerName)
  return playerId % 2 == 0 and redTeamSkin or blueTeamSkin
end

function api:modifyTexture(name, texture)
  if name == 'models/players/crash_color/skin_base1.tga' then
    texture:add{255, 0, 0, 0}
    return true
  end
  if name == 'models/players/crash_color/skin_base2.tga' then
    texture:add{0, 0, 255, 0}
    return true
  end
  return false
end
```

### `canPickup`(*spawnId*, *playerId*) &rarr; boolean

The environment calls this function to decide whether the item with ID `spawnId`
is allowed to be picked up. Return `false` to disallow pickup. Defaults to
`true` if the callback function isn't implemented.

Will not be called if *spawnId* is not a positive integer; this is normally
ensured by `updateSpawnVars()`.

### `commandLine`(*old_commandline*) &rarr; string

The environment calls this function at the very beginning to determine the
initial engine console commands. The `old_commandline` parameter is the default
value, which you should return as-is if you do not want to customize the command
sequence.

Example:

```lua
function api:commandLine(old_command_line)
  return '+set r_fullscreen "1"'
end
```

Lists of available commands in *Quake III Arena* can be found online.

### `createPickup`(*class_name*) &rarr; table

Returns a table with keys `name`, `classname`, `model`, `quantity`, `type`, and
an optional `typeTag` and `moveType`. (`tag` deprecated.)

Pickups are rendered bobbing and spinning in the air. If you require them to be
static, use `pickups.move_type.STATIC` as `moveType`.

DMLab defines two item types not found in *Quake III Arena*:

*   `pickups.type.REWARD`: an item which when picked up changes the agent's
    score, positive or negative.
*   `pickups.type.GOAL`: an item which when picked up changes the agent's score
    *and* causes the current level to restart.

See game_scripts/common/pickups.lua for examples.

Whenever the runtime cannot load the asset referenced by `model`, it will
invoke `createModel` with the contents of that string.

### `createModel`(*model_name*) &rarr; table

Returns a table with the geometry of the model referenced by `model_name`. The
format of such table is described in [Model Generation](
../creating_levels/model_generation.md).

### `customDiscreteActionSpec`() &rarr; array
### `customDiscreteActions`(*actions*)

`customDiscreteActionSpec` is called after `init`, it returns an additional
array of extra actions supplied by the script.

Each action must contain a `name`, `type` and `shape`.

*   `name` Name of the action reported by the environment.
*   `min` Min value of actions.
*   `max` Max value of actions.

When specified `customDiscreteActions` is called with an array current actions
for that frame.

```lua

function api:customActionSpec()
  return {
    {name = 'SWITCH_GADGET', min = -1, max = 1},
    {name = 'SELECT_GADGET', min = 0, max = 10},
  }
end

local currentCustomActions = actions

function api:customDiscreteActions(actions)
  currentCustomActions = actions
end

function api:modifyControl(controls)
  if currentCustomActions[1] ~= 0 then
    -- SWITCH_GADGET action.
  end
  if currentCustomActions[2] ~= 0 then
    -- SELECT_GADGET action.
  end
end
```

### `customObservation`(*name*) &rarr; tensor.ByteTensor or tensor.DoubleTensor.

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

### `customObservationSpec`() &rarr; array

Called after `init`, it returns an additional array of extra observation types
supplied by the script.

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
by the engine itself. See [python_api.md](../../users/python_api.md#python-environment-api)
for settings consumed by the engine.

Example:

```lua
function api:init(settings)
  print('Script Settings')
  for k, v in pairs(settings) do
    print(k .. ' = ' .. v)
  end
  io.flush()
end
```

### `hasEpisodeFinished`(*timeInSeconds*) &rarr; boolean

Called at the end of every frame with the elapsed time as an argument to
determine if the episode has finished and the game evaluation should continue
with the next episode.

The default implementation returns `false` until 2 minutes 30 seconds have
passed.

### `nextMap`() &rarr; string

Called whenever a map needs to be loaded. Must return the name of a map
discoverable by the engine.

For more information on maps, see [Maps](#maps) below.

### `gameEvent` (*eventName*, *eventData*)

Called when a game event occurs in engine. The event data is the data associated
with an event and is event specific. May be called multiple times within a
frame.

Events generated by the engine:

> None

### `pickup`(*spawnId*, *playerId*) &rarr; number

Event handler when the item with id *spawnId* is picked up. Returns the respawn
time, although this value is ignored if the item has a non-nil `wait` spawnVar.

Will not be called if *spawnId* is not a positive integer; this is normally
ensured by `updateSpawnVars()`.

### `start`(*episode*, *seed*) &rarr; nil

The environment calls this function at the start of each episode, with:

*   A number *episode*, starting from 0.
*   A number *seed*, the random seed of the game engine. Supplying the same seed
    is intended to result in reproducible behaviour.

Example:

```lua
function api:start(episode, seed)
  print('Entering episode no. ' .. episode .. ' with seed ' .. seed)
  io.flush()
end
```

### `updateSpawnVars`(*spawnVars*) &rarr; table

Called once per `spawnVars` entry when a new map is loaded. The `spawnVars`
argument is a Lua table with the internal Quake III Arena `spawnVars` key-value
setting. Quake III Arena [maps](../creating_levels/level_generation.md) define a
list of entities (light sources, player start points, items, ...). Implementing
this function allows overriding the settings for each map entity.

Returning `nil` will have the effect of deleting the map entity.

The default implementation returns `spawnVars` unchanged.

#### spawnVars: common keys and values

*   angle - angle (in degrees) that the entity is facing. A value of 0
    corresponds to facing along the +X axis.
*   classname - Required. Determines the functionality and type of the entity.
*   id - an arbitrary identifier. If it is not a (string-encoded) integer then
    `canPikcup()` and `pickup()` will not be called.
*   model - The geometry and texture of the entity, specified by an MD3 file.
*   origin - space-delimited string with XYZ coordinate values. (+Z is up)
*   spawnflags - For most entities, setting this to `1` indicates that the
    object is free-floating, and will not attempt to place the entity on the
    ground, but respect the given Z-axis value for height.
*   wait - Delay before the entity is respawned after being picked up; -1 means
    never respawn. Leave zero (unspecified) to allow `pickup()` to specify
    respawn time.

### `extraEntities`() &rarr; table

Called once after all built-in `updateSpawnVars` are processed. Must return an
array of new entities that will be added to a scene. Each entity must be a
string-string table containing the key 'classname'. These should match internal
Quake III Arena `spawnVars` key-value settings. Note `updateSpawnVars` is not
called for the entities created explicitly.

This example will add two apples to the scene at the locations specified.

```lua
function api:extraEntities()
  return {
      {
          classname = 'apple_reward',
          model = 'models/apple.md3',
          origin = '550 450 0',
      },
      {
          classname = 'apple_reward',
          model = 'models/apple.md3',
          origin = '600 450 0',
      },
  }
end
```

### `replaceModelName`(*modelName*) &rarr; optional string, optional string

Called once per `modelName` when a new model is loaded. If a replacement name is
required return a new name. Otherwise return nil. The model can have it's
textures modified with a prefix with an additional return value. This allows us
to load the same model multiple times with different textures. Returned strings
have a maximum size and the operation will fail if set to be too long.

### `replaceTextureName`(*textureName*) &rarr; optional string

Called once per `textureName` when a new map is loaded. If a replacement name is
required return a new name. Otherwise return nil.

### `loadTexture`(*textureName*) &rarr; optional ByteTensor(H, W, 4)

Called once per `textureName` when a new map is loaded. If a replacement name
was specified then that name is used here. If we want to override the built-in
texture loading return a tensor.ByteTensor(H, W, 4) of the texture. Otherwise
return nil.

### `modifyTexture`(*textureName*, *image*)

Called once per `textureName` when a new map is loaded. `image` is a ByteTensor
of the texture loaded. The script has an opportunity to modify the contents of
the texture in-place. `image` will be invalidated after the call is complete so
a copy of image must be made if to be used outside of the callback.

The callback must return whether the texture was modified.

### `mapLoaded`()

Called when the map has finished loading.

### `spawnInventory`(*loadOut*) -> table `updateInventory`(*loadOut*) -> table

`spawnInventory` is called on each avatar when spawned. `updateInventory` is
called on each avatar every frame. Must return a same table as passed in if
fields are edited or nil. The data in the table can be updated before returning.
There is a helper table used to interact with the `loadOut`:

    local inventory = require 'common.inventory'

If the function is defined `loadOut` must be returned, which can be modified
with `inventory.View`. See [Inventory View](#inventory_view)

```lua
local inventory = require 'common.inventory'

function api:spawnInventory(loadOut)
  -- This is actually the default load out for a player.
  -- These can be adjusted per player and per respawn.
  local view = inventory.View(loadOut)
  view:setGadgets{
      inventory.GADGETS.IMPULSE,
      inventory.GADGETS.RAPID,
      -- inventory.GADGETS.ORB,
      -- inventory.GADGETS.BEAM,
      -- inventory.GADGETS.DISC,
  }

  view:setGadgetAmount(inventory.GADGETS.IMPULSE, inventory.UNLIMITED)
  view:setGadgetAmount(inventory.GADGETS.RAPID, 100)

  -- Max health of the player. If health is greater than this the health counts
  -- down to this value.
  view:setMaxHealth(100)

  -- Health counts down to loadOut.stats[inventory.STATS.MAX_HEALTH]
  view:setHealth(100)

  -- Initial armor for player to start with.
  view:setArmor(0)

  -- Must return original table.
  return view:loadOut()
end
```

#### Inventory View

Valid Gadgets:

    inventory.GADGETS.IMPULSE  -- Contact gadget.
    inventory.GADGETS.RAPID,   -- Rapid fire gadget.
    inventory.GADGETS.ORB,     -- Area damage gadget. (Knocks players)
    inventory.GADGETS.BEAM,    -- Accurate and very rapid fire beam.
    inventory.GADGETS.DISC,    -- Powerful but lond period between firing.

Valid amount are in range `[0, 999)` or `inventory.UNLIMITED`

```lua
-- Returns/sets list of gadgets.
function View:gadgets()
function View:setGadgets(gadgets)

-- Returns/sets gadget's amount.
function View:gadgetAmount(gadget)
function View:setGadgetAmount(gadget, amount)

-- Adds gadget with optional amount.
function View:addGadget(gadget, amount)
-- Removes gadget.
function View:removeGadget(gadget)

-- Returns whether powerup is active.
function View:hasPowerUp(powerUp)

-- Returns/sets player's armor
function View:armor()
function View:setArmor(amount)

-- Returns/sets player's health. If health() > maxHealth() health will reduce
-- until it matches maxHealth().
function View:health()
function View:setHealth(amount)

-- Returns/sets player's max health.
function View:maxHealth()
function View:setMaxHealth(amount)

-- Returns player's eye position.
function View:eyePos()

-- Returns players view direction in Euler angles degrees.
function View:eyeAngles()

-- Returns players gadget.
function View:gadget()

-- Returns players id.
function View:playerId()
```

### `modifyControl`(*actions*)

Called once per frame. `actions` is a lua table containing six keys:
`lookDownUp`, `lookLeftRight`, `moveBackForward`, `strafeLeftRight`,
`crouchJump` and `buttonsDown`, denoting the actions retrieved from the
controller. The script has an opportunity to modify the actions and return it to
override the actions to be applied. If nothing or nil is returned, the method
makes no effect.

### `writeProperty`(*key*, *value*)

Callback to support `RlCApi`'s `write_property`.
Return `true` if `property[key]` is assigned `value`. Return `false` if
`property[key]` cannot hold `value` and return `nil` if `property[key]` does
not exist.

### `readProperty`(*key*)

Callback to support `RlCApi`'s `read_property`.
Return value of `property[key]` if it exists otherwise return `nil`.

### `listProperty`(*key*, *callback*)

Callback to support `RlCApi`'s `list_property`.
Return true if `property[key]` is a list and call `callback(key, type)` for
each sub-property of `property[key]` where key is the full length key to the
sub-property and type a string containing any combination of 'r', 'w' and 'l'.

If the type contains:

*   'l'  - Listable and `listProperty(key)` maybe called.
*   'r'  - Readable and `readProperty(key)` maybe called.
*   'w'  - Writable and `writeProperty(key, value)` maybe called.

### `registerDynamicItems`() -> table

Called during level loading. The classname of any object which may be spawned
after the map has loaded must be returned here. See
[`pickups_spawn`](#the-pickups-spawn-module).

### `playerMover`(*kwargs*)

Called by a func_lua_mover entity. `kwargs` provides the ID and position of the
mover entity, as well as the triggering player's current position and velocity.
Returns a player position delta table and a player velocity delta table.

### `lookat`(*entityId*, *lookedAt*, *position*, *playerId*)

Called once per frame when the player focuses on a lookat trigger with id
`entityId`. Once the player looks away a final call is made with `lookedAt` set
to `false`.

The lookat position relative to the trigger's bounding box is given as
`position`.

### `newClientInfo`(*playerId*, *playerName*, *modelName*)

Called once per player joining the game (including bots).

### `rewardOverride`(*kwargs*) -> Int or nil

Called whenever there is a reward event. This allows the level script to adjust
the reward received by the engine and to propagate events externally. The
`kwargs` are:

*   score - Integer default reward associated with in-game event.
*   [reason](#reasons) - String associated with the event.
*   playerId - Player receiving the reward.
*   otherPlayerId (optional) - Other player associated with event if there is
    one.
*   location (optional) - Location of the event of there exists one.
*   team - One of "free", "red", "blue".

Return either score or nil for default behaviour.

#### Reasons

See common/rewards.lua for tools for logging these events.

*   `PICKUP_REWARD` - `playerId` touched reward pickup.
*   `PICKUP_GOAL` - `playerId` touched goal pickup.
*   `TARGET_SCORE` - Level triggered a reward at `playerId`.
*   `TAG_SELF` - `playerId` tagged self.
*   `TAG_PLAYER` - `playerId` tagged `otherPlayerId`
*   `CTF_FLAG_BONUS` - `playerId` picked up enemy flag.
*   `CTF_CAPTURE_BONUS` - `playerId` has captured the opposing team's flag.
*   `CTF_TEAM_BONUS` - `playerId` is part of a team that has captured the
    opposing team's flag.
*   `CTF_FRAG_CARRIER_BONUS` - `playerId` tagged opponent(`otherPlayerId`) flag
    carrier.
*   `CTF_RECOVERY_BONUS` - `playerId` has returned their team flag to the team's
    base.
*   `CTF_CARRIER_DANGER_PROTECT_BONUS` - `playerId` is on the same team as the
    flag carrier and tagged opponent(`otherPlayerId`) who dammaged our flag
    carrier.
*   `CTF_FLAG_DEFENSE_BONUS` - `playerId` tagged opponent(`otherPlayerId`) while
    `playerId` or `otherPlayerId` is near `playerId`'s flag.
*   `CTF_CARRIER_PROTECT_BONUS` - `playerId` tagged opponent(`otherPlayerId`)
    while `playerId` or `otherPlayerId` is near `playerId`'s flag carrier.
*   `CTF_RETURN_FLAG_ASSIST_BONUS` - `playerId` returned the team flag just
    before a payerId's team captured opponent team's flag.
*   `CTF_FRAG_CARRIER_ASSIST_BONUS` - `playerId` tagged opponent team's flag
    carrier just before a capturing event occurred.

## Lua common helper objects

A number of helper functions can be found in `game_scripts/common`. They can be
used in game scripts via Lua's `require` statement, for example:

```lua
local make_map = require 'common.make_map'

function api:nextMap()
  return make_map.makeMap('G I A P', 'my_map')
end
```

### `common.make_map`

Returns a Lua object offering a [`makeMap`](#makemapmap_text-map_name) and a
[`commandLine`](#commandlineold_commandline--string) function.

### `common.pickups`

Returns a Lua object offering a `type` and `defaults` table for defining pickup
objects.

## The game module

Game scripts can interact with *DeepMind Lab* using the `dmlab.system.game`
module, which can be loaded using `local game = require 'dmlab.system.game'`.
The module provides the following functions:

### `addScore`(*score*)

Adds a *score* to the total score of the current player.

### `addScore`(*playerId*, *score*)

Adds a *score* to the total score of the player with *playerId* (1 indexed).

### `console`(*command*)

Adds 'command' ready for processing by game engine. Console commands are
all issued directly after `api:modifyControl` callback. See Quake 3 reference
materials to for the commands available.

Here are some examples:

Command              | Effect
-------------------- | -----------------------------------------
set <cvar> <value>   | Sets any internal <cvar> to <value>.
set cg_drawFPS 1     | Draws the FPS counter to the screen
set cg_fov 70        | Sets hoizontal field of view to 70.
weapon <number>      | Selects <number> gadget.
weapon 1             | Selects first gadget.
weapon 2             | Selects second gadget.
weapnext             | Selects next gadget.
weapprev             | Selects previous gadget.
setviewpos x y z yaw | Sets the player's x y z location and yaw.

Here are some custom commands:

Command      | Effect
------------ | --------------------------------------------------
dm_pickup id | Lets the player pickup all items with the id 'id'.

### `finishMap`()

Finishes the current map.

### `updateTexture`(*name*, *tensorData*)

Allows replacing the contents of a previously-loaded texture with the image data
in `tensorData`. Raises an error if the texture cannot be found.

### `episodeTimeSeconds`()

Returns the game time spent in the current episode in seconds.

### `playerInfo`()

Returns the state of your player in a table,

```lua
{
    -- Player position in world units.
    -- In our text mazes, one grid square is 100 world units.
    pos = {forward, left, up},

    -- Player velocity in world units per second.
    vel = {forward, left, up},

    -- Player orientation in degrees.
    -- In Euler angles (see https://en.wikipedia.org/wiki/Euler_angles).
    -- The relationship between the `pos` and `vel` and `angles` is,
    --
    --                    yaw = 90 degrees
    --                      +left axis
    --                          ^
    --                          |
    --                          |
    --  yaw = 180 degrees  <----+---->  yaw = 0 degrees
    --   -forward axis          |        +forward axis
    --                          |
    --                          v
    --                   yaw = -90 degrees
    --                      -left axis
    --
    angles = {pitch, yaw, roll},

    -- Player angular velocity in degrees per second.
    anglesVel = {pitch, yaw, roll},

    -- Height of the camera above player position.
    -- A single value in world units.
    height = h,
}
```

### `tempFolder`()

Returns the temporary folder where temporary assets are built and removed when
the context is released. This folder can also be used for script temporary
storage.

### `renderCustomView`(kwargs) &rarr; ByteTensor\[height x width x 3\]

```lua
{
    width = width,    -- Required, sets width of tensor returned.
    height = height,  -- Required, sets height of tensor returned.

    -- Required, sets position in game units of camera.
    pos = {x, y, z},

    -- Sets the pitch, roll, yaw in degrees of camera.
    look = {pitch, roll, yaw},

    -- Optional default true, whether to render player entity.
    renderPlayer = true,
}
```

Returns a byte tensor containing the render of the current game at that position
and look rotation. The images's origin is top left and interlaced.
Note +ve pitch looks down.

### `runFiles`()

Returns the folder where assets are stored. Most assets are placed in a
sub-folder called 'baselab'.

### `raycast`(startPos, endPos) -> fraction (float in range \[0, 1\])

Returns 1 if 'endPos' is visible from startPos and less than 1 otherwise.

If the fraction is less than one the first point of obstruction will be:
`(endPos - startPos) * fraction + startPos`.

### `inFov`(startPos, endPos, angles, fov) -> boolean

Returns whether the vector connecting `startPos` and `endPos` falls within the
angular range spanned by `fov` with respect to the Euler frame defined by
`angles`.

### `copyFileToLocation`(fromFileName, toFileName) -> string

Sets the contents of `toFileName` with the contents of `fromFileName`. An error
is produced if the file the operation fails for any reason. (It will use
file_reader_override if provided.)

### `loadFileToString`(fileName) -> string

Returns contents of fileName in a string. An error is produced if the file
doesn't exist. (It will use file_reader_override if provided.)

### `loadFileToByteTensor`(fileName) -> tensor.ByteTensor

Returns contents of fileName in a tensor.ByteTensor. An error is produced if
the file doesn't exist. (It will use file_reader_override if provided.)

## The game_entities module

Game scripts read information about entities with *DeepMind Lab* using the
`dmlab.system.game_entities` module, which can be loaded using `local
game_entities = require 'dmlab.system.game_entities'`. The module provides the
following functions.

### `entities`\(\[\{*classname1*, *classname2*, ...\}\]\) -> *array*.

Returns a list of entities that are active that frame. The optional argument is
a list of classnames. If provided, the result is filtered such that only
entities that match one of the classnames provided are returned.

Each entity in the list returned in a table:

```lua
entity = {
  entityId = 0,             -- Internal id.
  id = 0,                   -- Id provided via spawnVars.
  type = 0,                 -- Type matching 'pickups.type' in common.pickiups.
  visible = true,           -- Whether the entity is visible. (Placed pickups
                            -- are marked invisible when picked up.)
  position = {x, y, z},     -- Location the pickup is this frame.
  classname = "classname",  -- The classname of the pickup.
}
```

Example usage:

```lua
local game_entities = require 'dmlab.system.game_entities'


local entities = game_entities:entities{'apple_reward', 'lemon_reward'}

-- Print all apple_reward and lemon_reward locations.
for _, v in ipairs(entities) do
  print(unpack(v.position))
end
```

## The events module

Game scripts can interact with *DeepMind Lab* using the `dmlab.system.events`
module, which can be loaded using `local event = require 'dmlab.system.events'`.
The module provides the following functions.

### `add`(*name*, \[observation1, \[observation2 ... \]\])

Adds events to be read by the user. Each event has a list of observations. Each
observation may be one of string, ByteTensor or DoubleTensor.

Example script:

```lua
local events = require 'dmlab.system.events'
local tensor = require 'dmlab.system.tensor'

local api = {}
function api:start(episode, seed)
  events:add('Event Name', 'Text', tensor.ByteTensor{3}, tensor.DoubleTensor{7})
end
```

## The pickups_spawn module

### `spawn`(*kwargs*)

Spawns an entity into the current scene. The entity must be a string-string
table containing the key 'classname'. They should match internal Quake III Arena
`spawnVars` key-value settings.

Example:

```lua
local pickups_spawn = require 'dmlab.system.pickups_spawn'

local function _respondToEvent()
  pickups_spawn:spawn{
      classname = 'strawberry_reward',
      origin = '750 450 30',
      count = '5',
  }
end

-- If the entity is not in the original map it must be registered; otherwise it
-- is not visible.
function api:registerDynamicItems()
  return {'strawberry_reward'}
end
```

## Factory functions

*DeepMind Lab* provides a number of Lua files with auxiliary helper functions in
the `game_scripts/helpers` directory. Among them are *factories*, which are of
the form

```lua
local factory = {}

function factory.createLevelApi(kwargs)
  local api = {}
  -- ...
  return api
end

return factory
```

For examples, see any `game_scripts/factories/*_factory.lua` file. Factory
functions can be used as a higher-level API for creating game scripts. As an
example, the `lt_chasm.lua` game script is implemented via

```lua
local factory = require 'factories.lt_factory'
return factory.createLevelApi{mapName = 'lt_chasm'}
```

## Maps

*DeepMind Lab* maps are identified by strings representing a file name. This
file must contain a Quake III Arena map, see [Level
Generation](../creating_levels/level_generation.md) for a description of those.
*DeepMind Lab* also defines a [text level](../creating_levels/text_level.md)
format which provides a simple text format for creating levels. The Lua module
`game_scripts/common/make_map.lua` provides a convenient interface for making
maps from ASCII text level strings on the fly, namely

### `makeMap`(*args*)

Generates a map with a given name from a text level description.

`args` is a table containing:

*   `mapName` - Name of the map. Acts as a file name and should be unique.
*   `mapEntityLayer` - Text level description. For details on the format, see
    [Text Levels](../creating_levels/text_level.md).
*   `mapVariationsLayer` - Optional part of the text level description, which
    allows changing the appearance of specific map locations. See [Text
    Levels](../creating_levels/text_level.md).
*   `useSkybox` - Whether to use a skybox. Ceilings are open when a skybox is
    used and closed otherwise. Default: true.
*   `theme` - Name of the texture set to use in the generated level. For a list
    of available themes see [Text Levels](../creating_levels/text_level.md).
*   `allowBots` - Whether to generate Area Awareness System (AAS) for bots to
    navigate the map. Default is false.

An example usage of `makeMap` is

```lua
local make_map = require 'common.make_map'
...
function api:nextMap()
  mapText = 'G I A P'
  api._count = api._count + 1
  return make_map.makeMap{
    mapName = 'luaMap' .. api._count,
    mapEntityLayer = mapText
  }
end
```

where `api._count` would have been set up in `start`. The
`tests/empty_room_test.lua` game script file provides a self-contained
example.

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
*   `shadow` - (default true). - Whether to add black drop shadow.
*   `rgba` - (default {1, 1, 1, 1} ). - Text color and transparency.

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

For an example, see:
[demos/screen_decoration/text.lua](../../../game_scripts/levels/demos/screen_decoration/text.lua).

### `filledRectangles`(*args*)

Called to enable the script to render a filled rectangle at arbitrary locations.

`args` is a table containing:

*   `width` - Virtual screen width. (Always 640.)
*   `height` - Virtual screen height. (Always 480.)

The user must return an array of tables. Each table shall contain:

*   `x` - X location to render the rectangle. (0 is left edge, 640 is right.)
*   `y` - Y location to render the rectangle. (0 is top edge, 480 is bottom.)
*   `width` - width to render the rectangle. (0 is left edge, 640 is right.)
*   `height` - height to render the rectangle. (0 is top edge, 480 is bottom.)
*   `rgba` - colour {R, G, B, A} to render rectangle.

```lua
function api:filledRectangles(args)
  local redCenterRect = {
      x = args.width / 2 - 30,
      y = args.height / 2 - 30,
      width = 60,
      height = 60,
      rgba = {1, 0, 0, 1},
  }
  return { redCenterRect }
end
```

For an example, see:
[demos/screen_decoration/rectangles.lua](../../../game_scripts/levels/demos/screen_decoration/rectangles.lua).
