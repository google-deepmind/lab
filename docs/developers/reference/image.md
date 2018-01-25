# Image

Functions for interacting with PNG images.

Underlying C++ code is in `deepmind/engine/lua_image.cc`

## `load`(*path*)

Loads a PNG image into a tensor.

The shape of the tensor will be either {height, width, 3} or {height, width, 4}
depending whether the png has 3 or 4 channels. It doesn't support paletted PNGs.

```lua
local image = require 'dmlab.system.image'
local helpers = require 'common.helpers'

local FILE_NAME = ...

local image = image.load(helpers.dirname(FILE_NAME) .. "path/to/image.png")
```

## `scale`(*src*, *tgt_height*, *tgt_width*)

Scales an image tensor, using bilinear interpolation for upsampling and the
average of the minified pixels when downsampling.

The shape of the scaled tensor will be {tgt_rows, tgt_cols, ChannelCount}, where
ChannelCount is the number of channels used by src.

```lua
local image = require 'dmlab.system.image'
local tensor = require 'dmlab.system.tensor'

> src = tensor.ByteTensor{
  {{255, 0, 0}, {170, 85, 0}, {85, 170, 0}, {0, 255, 0}},
  {{170, 0, 85}, {141, 85, 85}, {113, 170, 85}, {85, 255, 85}},
  {{85, 0, 170}, {113, 85, 170}, {141, 170, 170}, {170, 255, 170}},
  {{0, 0, 255}, {85, 85, 255}, {170, 170, 255}, {255, 255, 255}}
}
> tgt = image.scale(src, 2, 6)
> tgt
Shape: [2, 6, 3]
[[[212,   0,  42]
  [178,  51,  42]
  [143, 102,  42]
  [110, 153,  42]
  [ 76, 204,  42]
  [ 42, 255,  42]]
 [[ 42,   0, 212]
  [ 76,  51, 212]
  [110, 102, 212]
  [144, 153, 212]
  [178, 204, 212]
  [212, 255, 212]]]
```
