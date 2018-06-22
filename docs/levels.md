# Levels

DeepMind Lab comes with several levels, each designed to test specific mental
and cognitive abilities.

The levels are written in Lua. They can be instantiated with either the [Lua
API](/docs/developers/reference/lua_api.md) or
the [Python API](/docs/users/python_api.md).

Most levels have level-specific configuration options. There are also some
standard options, listed here.

Key                    | Default | Description
:--------------------- | :-----: | :----------
`episodeLengthSeconds` | 90      | An episode (one run with potentially many level restarts) ends after this many seconds.
`gadgetSelect`         | false   | Enables gadget selection. See [actions](/docs/users/actions.md)
`gadgetSwitch`         | false   | Enables gadget switching. See [actions](/docs/users/actions.md)
`allowHoldOutLevels`   | false   | Enables hold-out levels that have been decorated with test_only.
`logLevel`             | INFO    | Controls the verbosity of log messages. Must be one of NEVER, ERROR, WARNING, INFO, or any positive integer. See game_scripts/common/log.lua.

The levels that are shipped with DeepMind Lab can be found in
[game_scripts/levels](../game_scripts/levels).
