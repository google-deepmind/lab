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

## `load`('content:.png', content)

Loads a PNG image into a tensor from the bytes of a PNG file.

The shape of the tensor will be either {height, width, 3} or {height, width, 4}
depending whether the png has 3 or 4 channels. It doesn't support paletted PNGs.

```lua
local image = require 'dmlab.system.image'
local game = require 'dmlab.system.game'
local helpers = require 'common.helpers'

local filename = helpers.dirname(FILE_NAME) .. "path/to/image.png"
local pngContents = game:loadFileToString(filename)
local image = image.load('content:.png', pngContents)
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

## `setHue`(*img*, *hue*)

Given an RGB image tensor (`img`), set its hue to a specified value (`hue`).
This is done by converting the image into Hue, Saturation, and Lightness
colorspace, setting the Hue to `hue`, and converting back again. The operation
is done in-place on the supplied image tensor.

Arguments:

*   img - A contiguous ByteTensor with 3 or 4 channels in last dimension.
*   hue - Desired hue (in degrees).

Notes:

As the conversion is done via only modifying the hue in the HSL colour space,
saturation and lightness remain unaltered (i.e., whites remain whites and blacks
remain blacks).

Returns updated `img`.

```lua
local image = require 'dmlab.system.image'
local tensor = require 'dmlab.system.tensor'

> src = tensor.ByteTensor{
      {255, 0, 0},
      {127, 0, 0},
      {255, 127, 127},
      {127, 64, 64},
      {255, 255, 255},
      {0, 0, 0},
  }

> greenHue = 120
> image.setHue(src, greenHue)
> src
Shape: [6, 3]
[[  0, 255,   0]
 [  0, 127,   0]
 [127, 255, 127]
 [ 64, 127,  64]
 [255, 255, 255]
 [  0,   0,   0]]
```

## `setMaskedPattern`(*img*, *pattern*, *color1*, *color2*)

Overlays a colored pattern onto a source image using its alpha channel as an
opacity mask.

Arguments:

1.  `img` - ByteTensor HxWx4 contiguous. Source image to apply pattern to.
2.  `pattern` - ByteTensor HxWx? contiguous (assumed to be grayscale so only the
    first channel used).
3.  `color1` - `{R, G, B}` White in pattern image is mapped to `color1`
4.  `color2` - `{R, G, B}` Black in pattern image is mapped to `color2`.

R, G, and B must be integers in the range \[0, 255\]. The pattern color is the
linear interpolation of `color1` and `color2` according to the `pattern` image's
first channel. The linear interpolation is between `color2` for pattern value 0
and `color1` for pattern value 255.

Each pixel in the source image `img` is interpolated between the corresponding
pattern color as defined above and its own original color. The linear
interpolation is between source image colour where the source image's alpha is 0
and the pattern color where the source image's alpha is 255. The source image's
alpha channel is then set to 255.

Returns the modified source image `img`.

```lua
local image = require
'dmlab.system.image' local tensor = require 'dmlab.system.tensor'

> img = tensor.ByteTensor{ {{255, 0, 0, 0}, {255, 0, 0, 128}, {255, 0, 0, 255}},
> {{255, 0, 0, 0}, {255, 0, 0, 128}, {255, 0, 0, 255}}, {{255, 0, 0, 0}, {255,
> 0, 0, 128}, {255, 0, 0, 255}}, }
>
> pat = tensor.ByteTensor{ {{255}, {255}, {255}}, {{128}, {128}, {128}}, {{ 0},
> { 0}, { 0}}, }
>
> image.setMaskedPattern(img, pat, {0, 255, 0}, {0, 0, 255})
>
> target = tensor.ByteTensor{ {{255, 0, 0, 255}, {127, 128, 0, 255}, { 0, 255,
> 0, 255}}, {{255, 0, 0, 255}, {127, 64, 64, 255}, { 0, 128, 127, 255}}, {{255,
> 0, 0, 255}, {127, 0, 128, 255}, { 0, 0, 255, 255}}, } assert(img == target,
> tostring(img))
```

It is functionally equivalent to:

```lua
function image.setMaskedPattern(src, pat, color1, color2)
  local hSrc, wSrc, cSrc = unpack(src:shape())
  local hPat, wPat, cPat = unpack(pat:shape())
  assert(hSrc * wSrc == hPat * wPat)
  assert(cSrc == 4)
  assert(cPat > 0)
  local rC1, gC1, bC1 = unpack(color1)
  local rC2, gC2, bC2 = unpack(color2)
  for i = 1, hSrc do
    local rowSrc = src(i)
    local rowPat = pat(i)
    for j = 1, wSrc do
      local rSrc, gSrc, bSrc, aSrc = unpack(rowSrc(j):val())
      local aPat = cPat > 1 and unpack(rowPat(j):val()) or rowPat(j):val()
      local rPat = math.floor((rC1 * aPat + (255 - aPat) * rC2 + 127) / 255)
      local gPat = math.floor((gC1 * aPat + (255 - aPat) * gC2 + 127) / 255)
      local bPat = math.floor((bC1 * aPat + (255 - aPat) * bC2 + 127) / 255)
      rowSrc(j):val{
          math.floor((rSrc * (255 - aSrc) + rPat * aSrc + 127) / 255),
          math.floor((gSrc * (255 - aSrc) + gPat * aSrc + 127) / 255),
          math.floor((bSrc * (255 - aSrc) + bPat * aSrc + 127) / 255),
          255
      }
    end
  end
end
```
