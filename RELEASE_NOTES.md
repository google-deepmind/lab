# DeepMind Lab Release Notes

## Current Release

### New Features:

1.  Add the ability to spawn pickups dynamically at arbitrary locations.

### Bug Fixes:

1.  Fix playerId and otherPlayerId out by one errors in 'game_rewards.lua'.
2.  Require playerId passed to `game:addScore` to be one indexed instead of zero
    indexed and allow `game:addScore` to be used without a playerId.
3.  `game:renderCustomView` now renders the view with top-left as the origin.
    The previous behaviour can be achieved by calling reverse(1) on the returned
    tensor.
4.  Fix a bug in image.scale whereby the offset into the data was erroneously
    ignored.
5.  Fix a typo in a `require` statement in visual_search_factory.lua.
6.  Fix a few erroneous dependencies on Lua dictionary iteration order.
7.  `game:AddScore` now works even on the final frame of an episode.

### Minor Improvements:

1.  Moved .map files into assets/maps/src and .bsp files into assets/maps/built.
    Added further pre-built maps, which removes the need for the expensive
    :map_assets build step.
2.  Allow game to be rendered with top-left as origin instead of bottom-left.
3.  Add 'mixerSeed' setting to change behaviour of all random number generators.
4.  Support for BGR_INTERLEAVED and BGRD_INTERLEAVED observation formats.
5.  Add a Lua API to load PNGs from file contents.
6.  Add 'eyePos' to playerInfo() for a more accurate eye position of player.
    Used in place of player pos + height.
7.  Add support for absl::string_view to lua::Push and lua::Read.
8.  Allow player model to be overridden via 'playerModel' callback.
9.  Add ability to specify custom actions via 'customDiscreteActionSpec' and
    'customDiscreteAction' callbacks.
10. Add `game:console` command to issue Quake 3 console commands directly.
11. Add `clamp` to tensor operations.
12. Add new callback `api:newClientInfo`, allowing each client to intercept
    when players are loading.

### Deprecated Features:

1.  Observation format names `RGB_INTERLEAVED` and `RGBD_INTERLEAVED` replace
    `RGB_INTERLACED` and `RGBD_INTERLACED`, respectively. The old format names
    are deprecated and will be removed in a future release.

## release-2018-02-07 February 2018 release

### New Levels:

1.  DMLab-30.

     1.  contributed/dmlab30/rooms_collect_good_objects_{test,train}
     2.  contributed/dmlab30/rooms_exploit_deferred_effects_{test,train}
     3.  contributed/dmlab30/rooms_select_nonmatching_object
     4.  contributed/dmlab30/rooms_watermaze
     5.  contributed/dmlab30/rooms_keys_doors_puzzle
     6.  contributed/dmlab30/language_select_described_object
     7.  contributed/dmlab30/language_select_located_object
     8.  contributed/dmlab30/language_execute_random_task
     9.  contributed/dmlab30/language_answer_quantitative_question
    10.  contributed/dmlab30/lasertag_one_opponent_small
    11.  contributed/dmlab30/lasertag_three_opponents_small
    12.  contributed/dmlab30/lasertag_one_opponent_large
    13.  contributed/dmlab30/lasertag_three_opponents_large
    14.  contributed/dmlab30/natlab_fixed_large_map
    15.  contributed/dmlab30/natlab_varying_map_regrowth
    16.  contributed/dmlab30/natlab_varying_map_randomized
    17.  contributed/dmlab30/skymaze_irreversible_path_hard
    18.  contributed/dmlab30/skymaze_irreversible_path_varied
    19.  contributed/dmlab30/psychlab_sequential_comparison
    20.  contributed/dmlab30/psychlab_visual_search
    21.  contributed/dmlab30/explore_object_locations_small
    22.  contributed/dmlab30/explore_object_locations_large
    23.  contributed/dmlab30/explore_obstructed_goals_small
    24.  contributed/dmlab30/explore_obstructed_goals_large
    25.  contributed/dmlab30/explore_goal_locations_small
    26.  contributed/dmlab30/explore_goal_locations_large
    27.  contributed/dmlab30/explore_object_rewards_few
    28.  contributed/dmlab30/explore_object_rewards_many

### New Features:

1.  Basic support for demo recording and playback.

### Minor Improvements:

1.  Add a mechanism to build DeepMind Lab as a PIP package.
2.  Extend basic testing to all levels under game_scripts/levels.
3.  Add settings `minimalUI` and `reducedUI` to avoid rendering parts of the
    HUD.
4.  Add `teleported` flag to `game:playerInfo()` to tell whether a player has
    teleported that frame.
5.  Add Lua functions `countEntities` and `countVariations` to the maze
    generation API to count the number of occurrences of a specific entity or
    variation, respectively.
6.  Add ability to switch/select gadget via actions. Actions are available when
    enabling flags `gadgetSelect` and/or `gadgetSwitch`.

### Bug Fixes:

1.  Fix out-of-bounds access in Lua 'image' library.
2.  Fix off-by-one error in renderergl1 grid mesh rendering.

## release-2018-01-26 January 2018 release

### New Levels:

1.  Psychlab, a platform for implementing classical experimental paradigms from
    cognitive psychology.

    1.  contributed/psychlab/sequential_comparison
    2.  contributed/psychlab/visual_search

### New Features:

1.  Extend functionality of the built-in `tensor` Lua library.
2.  Add built-in `image` Lua library for loading and scaling PNGs.
3.  Add error handling to the env_c_api (version 1.1).
4.  Add ability to create events from Lua scripts.
5.  Add ability to retrieve game entity from Lua scripts.
6.  Add ability create pickup models during level load.
7.  Add ability to update textures from script after the level has loaded.
8.  Add Lua customisable themes. Note: This change renames helpers in
    `maze_generation` to be in lowerCamelCase (e.g. `MazeGeneration` ->
    `mazeGeneration`).
9.  The directory `game_scripts` has moved out of the `assets` directory, and
    level scripts now live separately from the library code in the `levels`
    subdirectory.

### Minor Improvements:

1.  Remove unnecessary dependency of map assets on Lua scripts, preventing
    time-consuming rebuilding of maps when scripts are modified.
2.  Add ability to disable bobbing of reward and goal pickups.
3.  The setting `controls` (with values `internal`, `external`) has been renamed
    to `nativeApp` (with values `true`, `false`, respectively). When set to
    `true`, programs linked against `game_lib_sdl` will use the native SDL input
    devices.
4.  Change LuaSnippetEmitter methods to use table call conventions.
5.  Add config variable for monochromatic lightmaps ('r_monolightmaps'). Enabled
    by default.
6.  Add config variable to limit texture size ('r_textureMaxSize').
7.  api:modifyTexture must now return whether the texture was modified.
8.  Add ability to adjust rewards.
9.  Add ability to raycast between different points on the map.
10. Add ability to test whether a view vector is within an angle range within a
    oriented view frame.

### Bug Fixes:

1.  Increase current score storage from short to long.
2.  Fix ramp jump velocity in level lt_space_bounce_hard.
3.  Fix Lua function 'addScore' from module 'dmlab.system.game' to allow
    negative scores added to a player.
4.  Remove some undefined behaviour in the engine.
5.  Reduce inaccuracies related to angle conversion and normalization.
6.  Behavior of team spawn points now matches that of player spawn points.
    'randomAngleRange' spawnVar must be set to 0 to match previous behavior.

## release-2016-12-06 Initial release
