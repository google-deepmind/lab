// Copyright (C) 2017 Google Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
////////////////////////////////////////////////////////////////////////////////
//
// Expose a function 'load' that accepts a path to an image file and returns a
// tensor containing the pixel data. Currently supports a sub set of the PNG
// format.

#ifndef DML_DEEPMIND_ENGINE_LUA_IMAGE_H_
#define DML_DEEPMIND_ENGINE_LUA_IMAGE_H_

#include "deepmind/lua/lua.h"

namespace deepmind {
namespace lab {

// Returns a table of image related functions:
// * load(path): [-1, +1, e]
//   Loads supported PNG format with the suffix '.png' and returns a byte tensor
//   with a shape {H, W, ChannelCount}. ChannelCount depends on the type of the
//   PNG being loaded.
//   Supports only non-paletted images with 8 bits per channel.
// * scale(src, tgt_height, tgt_width): [-3, +1, e]
//   Scales the image contained in byte tensor 'src' to shape {tgt_height,
//   tgt_width, ChannelCount}, where ChannelCount is the number of channels used
//   by 'src'. Returns the scaled image.
//   Supports only contiguous tensors as input.
// Must be called with Lua upvalue pointing to a DeepMindReadOnlyFileSystem.

// [0, +1, -]
int LuaImageRequire(lua_State* L);

}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_ENGINE_LUA_IMAGE_H_
