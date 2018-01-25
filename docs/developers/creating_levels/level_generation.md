# Level generation

Levels for DeepMind Lab are _Quake III Arena_ levels. They are packaged into `.pk3`
files (which are ZIP files) and consist of a number of components, including the
following:

   * One `.bsp` file per level ("binary space partitioning"). This contains the
     level geometry (the "map").
   * Optionally an accompanying `.aas` file per level ("area awareness system").
     This contains navigation information for the in-game bots of _Quake III
     Arena_.
   * Textures. Textures are referenced by maps. DeepMind Lab ships with a number
     of default textures.

You can use your own maps, zipped up as a `.pk3` file, by placing the `.pk3`
file(s) into your base directory (e.g. `$HOME/.deepmind_lab/baselab`).

DeepMind Lab includes tools to generate maps: The `q3map2` tool converts a
`.map` file into a `.bsp` file, and the `bspc` tool generates an `.aas` file
from a `.bsp` file. The `.map` format is a human-readable text format that
describes a map as a sequence of _entities_. Map files are cumbersome to edit by
hand, but a variety of level editors are freely available
(e.g.  [GtkRadiant](https://github.com/TTimo/GtkRadiant)).

In addition to built-in and user-provided levels, DeepMind Lab offers additional
sources of levels:

   * Procedurally generated levels, which are created on the fly.
   * [Text levels](text_level.md), which are simple, human-readable text files
     that can be compiled into `.map` files. This can either be done offline,
     or programmatically inside DeepMind Lab using Lua extensions.
