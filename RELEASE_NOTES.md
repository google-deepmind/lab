# DeepMind Lab Release Notes

## Current Release

### New Features:

1.  Extended functionality of the built-in Tensor Lua library.
2.  Add Image Lua library for loading and scaling PNGs.
3.  Add error handling to the env_c_api.
4.  Add ability to create events from Lua scripts.
5.  Add ability to retrieve game entity from Lua scripts.
6.  Add ability create pickup models during level load.

### Minor Improvements:

1.  Remove unnecessary dependency of map assets on Lua scripts, preventing
    time-consuming rebuilding of maps when scripts are modified.
2.  Add ability to disable bobbing of reward and goal pickups.
3.  The setting `controls` (with values `internal`, `external`) has been renamed
    to `nativeApp` (with values `true`, `false`, respectively). When set to
    `true`, programs linked against `game_lib_sdl` will use the native SDL input
    devices.

### Bug Fixes:

1.  Increase current score storage from short to long.

## release-2016-12-06 Initial release
