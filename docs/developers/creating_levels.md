# Creating Levels

Levels are Lua scripts loaded from the `game_scripts/levels` directory.

## Design Pattern

All logic is put into a `factory` in `game_scripts/factories`. That factory is
then instantiated with a small, data-only script.

For example, `game_scripts/levels/lt_horseshoe_color.lua` instantiates the logic in
`assets/games_scripts/factories/lt_factory.lua` with `botCount = 4`.

```lua
local factory = require 'factories.lt_factory'

return factory.createLevelApi{
    mapName = 'lt_horseshoe_color',
    color = true,
    botCount = 4,
    camera = {-50, 450, 900},
}
```

## Tutorial

A [narrative
tutorial](/docs/developers/minimal_level_tutorial.md)
walks through the initial steps in creating a simple level.

## Details

*   [Text Levels](/docs/developers/creating_levels/text_level.md)
    for how to convert ASCII text maps into levels.
*   [Model Generation](/docs/developers/creating_levels/model_generation.md)
    for how models and their variants are placed in levels.
*   [Level Generation](/docs/developers/creating_levels/level_generation.md)
    for details on the levels generation pipeline.
