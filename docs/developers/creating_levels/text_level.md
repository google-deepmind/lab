# Text Levels

Text levels are DeepMind Lab levels that are compiled from a simple text format,
together with a set of user-provided hooks that describe the domain-specific
logic for interpreting the text format.

The result of compiling a text level is a `.map` file, which can be compiled
into an actual DeepMind Lab level with the `q3map2` and `bspc` tools (see [level
generation basics](level_generation.md)).

## Text level format

A text level describes a particularly simple kind of map, namely a map
consisting of a rectangular grid of square cells. The level is provided
in two parts: The _entity layer_ and the _variation layer_. The textual
representation of the entity layer is interpreted as follows:

   * The level text consists of a number of _lines_. Lines are separated by a
     platform-dependent newline separator and are interpreted as sequences of
     single-byte characters. ASCII-encoding is recommended. The _length_ of a
     line is the number of bytes excluding the newline separator.
   * Lines of length zero are ignored.
   * The number of non-empty lines is the _height_ `H` of the map.
   * The length of the longest line is the _width_ `W` of the map.
   * Each character corresponds to a grid cell. The grid coordinate (0, 0) is at
     the top-left, and the value of character `j` of line `i` is the value of
     the grid cell (`i`, `j`).
   * The following cell values (in the system's encoding) are special:
     "<code> </code>" (space), which represents an empty cell, and "`*`"
     (asterisk) which represents a wall. All other cell values should be
     printable characters.
   * Cell values that are not explicitly specified by the level text are implied
     to be wall cells.

The interpretation of the variation layer is similar, with the following
differences:

   * All character values mean "default" except for the values
     `'A'`&ndash;`'Z'`, which denote, respectively, one of 26 named
     variations.
   * Variation layer cells that are either outside the cell grid or at the
     location of a wall cell are ignored. Only non-wall cells inside the cell
     grid can have a variation.

The variation layer is optional; for example, an empty string would simply
represent "no variation" for every cell.

## Translation

A text level is translated into `.map` data as follows: Two adjacent entities
that are not walls have an *opening* between them in the corresponding
direction. In this way, every non-wall cell has 0 to 4 openings, and for every
direction in which there is no opening, a wall entity is added to the output.

```
              ^ opening (N)
          ....|......
         #    |     :
         #          :
Wall (W) #        ----> opening (E)
         #          :
         #          :
         ############
           Wall (S)
```

Each non-wall cell can optionally have a named variation, as determined by the
corresponding value of the variation layer. This determines the basic geometry
and looks of the level.

Next, user cell values are processed. To this end, the text level compiler
calls back a user-provided function for every grid cell that is neither
empty nor a wall. The user-provided function is called with arguments
`(i, j, c)`, where (`i`, `j`) is the position of the cell and `c` is its
value. The function shall indicate whether it was able to process the user
value. If yes, it shall return a (possibly empty) list of entity strings,
which are appended to the resulting map. If not, then a default action may
be executed for the cell value.

We currently plan to allow user callbacks that are provided as Lua code. The
callback returns `nil` to signal failure and an array of strings otherwise.

## Global settings

Map generation from text levels has a number of parameters, which may or may not
be configurable or extensible.

   * A *theme*, a collection of textures used to decorate the map.
   * Decal frequencies: Random decals are added to adorn the map at a given
     rate.
   * A *skybox*, a textured cube which is always rendered as if its faces were
     at an infinite distance from the agent. Whenever the skybox is present the
     ceiling of the cells is open.

### Themes

These are the themes currently supported:

   * MISHMASH (default)
   * TRON
   * MINESWEEPER
   * TETRIS
   * GO
   * PACMAN
   * INVISIBLE_WALLS

## Randomness

A number of map features are selected randomly.

   * For each variation, a random texture is assigned from the pool of textures
     in the map theme.
   * Decals are added to walls and floors randomly (at a configurable rate).

During the translation, a source of randomness is required to make those
selections. The user needs to provide a random number generator as the source
of that randomness. It is recommended to use a seedable pseudo-random number
generator for reproducibility.

## Default actions

The following default cell values are recognized:

   * `P`: the default action creates a spawn point at this cell.
   * `H`: the default action creates a door that is traversed in up-down
     direction (that is, a door at (`i`, `j`) connects (`i` &pm; 1, `j`).
   * `I`: the default action creates a door that is traversed in left-right
     direction (that is, a door at (`i`, `j`) connects (`i`, `j` &pm; 1).


## Example

**Entity layer:**

```
********
*a * x *****
**     *   *
 ****  I   *
    *  *   *
    *  *****
    *   *
******H*******
*        I P *
**************
```

**Variation layer:**

```


        AAA
        AAA
        AAA



 CCCCCCCC BBB
```

This example shows a small map with four rooms separated by doors. The top-left
room has no variation, and the other three rooms have variations `A`, `B`
and `C`, respectively. There is a spawn point in the bottom-right room (which
has variation `B`), assuming the default actions were used for the entity values
`P`, `H` and `I`. There are additional entities of values `a` and `x` in the
room at the top-left. (There is also a fifth, inaccessible "room" at the
middle-left that is not walled off from the edge of the map.)

The entity layer is equivalent to the one represented in the  following
"properly rectangular" form:

```
**************
*a * x *******
**     *   ***
 ****  I   ***
    *  *   ***
    *  *******
    *   ******
******H*******
*        I P *
**************
```

In the representation of the variation layer, all characters other than
`'A'`&ndash;`'Z'` are equivalent. For example, the following equivalent
representation may be easier to read:

```
..............
..............
........AAA...
........AAA...
........AAA...
..............
..............
..............
.CCCCCCCC.BBB.
..............
```
