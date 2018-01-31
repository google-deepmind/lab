# DeepMind Lab Release Notes

## Current Release

### Bug Fixes:

1.  Fix out-of-bounds access in Lua 'image' library.

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
