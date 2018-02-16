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

#include "deepmind/engine/lua_image.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

#include "absl/strings/str_cat.h"
#include "deepmind/lua/bind.h"
#include "deepmind/lua/push.h"
#include "deepmind/lua/read.h"
#include "deepmind/lua/table_ref.h"
#include "deepmind/tensor/lua_tensor.h"
#include "deepmind/util/files.h"
#include "png.h"

namespace deepmind {
namespace lab {
namespace {

std::vector<unsigned char> PngParsePixels(png_structp png_ptr,
                                          png_infop info_ptr,
                                          const tensor::ShapeVector& shape) {
  std::vector<unsigned char> bytes;
  bytes.reserve(shape[0] * shape[1] * shape[2]);
  const png_uint_32 bytesPerRow = png_get_rowbytes(png_ptr, info_ptr);
  std::unique_ptr<unsigned char[]> row_data(new unsigned char[bytesPerRow]);
  auto bytes_inserter = std::back_inserter(bytes);
  for (std::size_t rowIdx = 0; rowIdx < shape[0]; ++rowIdx) {
    png_read_row(png_ptr, row_data.get(), nullptr);
    std::copy_n(row_data.get(), shape[1] * shape[2], bytes_inserter);
  }
  return bytes;
}

struct Reader {
  std::string contents;
  std::size_t location;
};

extern "C" {
static void PngReadContents(png_structp png_ptr, png_bytep out_bytes,
                            png_size_t count) {
  Reader* reader = static_cast<Reader*>(png_get_io_ptr(png_ptr));
  if (count + reader->location <= reader->contents.size()) {
    std::copy_n(reader->contents.begin() + reader->location, count, out_bytes);
  }
  reader->location += count;
}
}  // extern "C"

lua::NResultsOr LoadPng(lua_State* L, std::string contents) {
  Reader reader{std::move(contents), 0};
  constexpr std::size_t png_header_size = 8;
  if (reader.contents.size() < png_header_size) {
    return "Invalid format. Contents too short.";
  }

  if (!png_check_sig(reinterpret_cast<png_bytep>(&reader.contents[0]),
                     png_header_size))
    return "Invalid format. Unrecognised signature.";

  reader.location += png_header_size;

  struct Png {
    png_structp ptr;
    png_infop info_ptr;
    ~Png() {
      png_destroy_read_struct(ptr ? &ptr : nullptr,
                              info_ptr ? &info_ptr : nullptr, nullptr);
    }
  };

  Png png = {};

  png.ptr =
      png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
  if (!png.ptr) return "Internal error.";

  png.info_ptr = png_create_info_struct(png.ptr);
  if (!png.info_ptr) return "Internal error.";

  png_set_read_fn(png.ptr, &reader, &PngReadContents);

  png_set_sig_bytes(png.ptr, png_header_size);
  png_read_info(png.ptr, png.info_ptr);
  png_uint_32 width = 0;
  png_uint_32 height = 0;
  int bitDepth = 0;
  int colorType = -1;
  png_uint_32 retval =
      png_get_IHDR(png.ptr, png.info_ptr, &width, &height, &bitDepth,
                   &colorType, nullptr, nullptr, nullptr);

  if (retval != 1) return "Invalid format. Corrupted header.";
  if (bitDepth != 8)
    return "Unsupported format. Image must have 8-bit channels.";

  tensor::ShapeVector shape = {height, width, 0};

  switch (colorType) {
    case PNG_COLOR_TYPE_GRAY:
      shape[2] = 1;
      break;
    case PNG_COLOR_TYPE_GRAY_ALPHA:
      shape[2] = 2;
      break;
    case PNG_COLOR_TYPE_RGB:
      shape[2] = 3;
      break;
    case PNG_COLOR_TYPE_RGB_ALPHA:
      shape[2] = 4;
      break;
    default:
      return "Unsupported format. Image must not be paletted.";
  }

  auto bytes = PngParsePixels(png.ptr, png.info_ptr, shape);
  if (reader.location > reader.contents.size())
    return "Invalid format. Contents too short.";
  tensor::LuaTensor<unsigned char>::CreateObject(L, std::move(shape),
                                                 std::move(bytes));
  return 1;
}

lua::NResultsOr Load(lua_State* L) {
  std::string file_name;
  if (!lua::Read(L, 1, &file_name) || file_name.size() < 4) {
    std::string error = absl::StrCat("[image.load] - \"", lua::ToString(L, 1),
                                     "\" - Invalid name");
    return error;
  }

  std::string contents;

  if (file_name.compare(0, file_name.length() - 4, "content:") == 0) {
    if (!lua::Read(L, 2, &contents)) {
      std::string error = absl::StrCat("[image.load] - Missing contents.");
      return error;
    }
  } else if (!util::GetContents(file_name, &contents)) {
    std::string error =
        absl::StrCat("[image.load] - \"", file_name, "\" could not be read.");
    return error;
  }

  if (file_name.compare(file_name.length() - 4, 4, ".png") == 0) {
    auto status = LoadPng(L, std::move(contents));
    if (!status.ok()) {
      std::string error = absl::StrCat("[image.load] (PNG) - \"", file_name,
                                       "\" - ", status.error());
      return error;
    }
    return status;
  }
  std::string error = absl::StrCat("[image.load] - \"", file_name,
                                   "\" - Unsupported file type.");
  return error;
}

class LinearMagnifier {
 public:
  // Given an image with 'num_channels' channels, scales up the
  // scanline pointed to by 'source' from 'source_len' elements to 'target_len',
  // storing the resulting pixels in 'target'. Such scaling is performed using
  // linear interpolation.
  // Returns the iterator position past the last computed pixel.
  template <typename Iter1, typename Iter2>
  Iter2 operator()(              //
      std::size_t num_channels,  //
      std::size_t source_len,    //
      Iter1 source,              //
      std::size_t target_len,    //
      Iter2 target) const {
    assert(source_len > 1);
    assert(source_len < target_len);
    // Supersampling: linearly interpolate source pixels.
    std::array<double, 4> low, top;
    double stride = (target_len - 1) / static_cast<double>(source_len - 1);
    double top_idx_f = 0.0;
    std::size_t top_idx_i = 0;
    std::copy_n(source, num_channels, top.begin());
    for (std::size_t i = 1; i < source_len; ++i) {
      std::size_t low_idx_i = top_idx_i;
      std::swap(low, top);
      top_idx_f += stride;
      top_idx_i = top_idx_f + 0.5;
      std::size_t k = i * num_channels;
      std::copy_n(source + k, num_channels, top.begin());
      double rcp_stride = 1.0 / (top_idx_i - low_idx_i);
      for (std::size_t j = low_idx_i; j < top_idx_i; ++j) {
        double low_wgt = (top_idx_i - j) * rcp_stride;
        double top_wgt = 1.0 - low_wgt;
        for (std::size_t c = 0; c < num_channels; ++c) {
          *target = low_wgt * low[c] + top_wgt * top[c];
          ++target;
        }
      }
    }
    return std::copy_n(top.begin(), num_channels, target);
  }
};

class NearestMagnifier {
 public:
  // Given an image with 'num_channels' channels, scales up the scanline pointed
  // to by 'source' from 'source_len' elements to 'target_len', storing the
  // resulting pixels in 'target'. Such scaling is performed by sampling the
  // nearest source pixel.
  // Returns the iterator position past the last computed pixel.
  template <typename Iter1, typename Iter2>
  Iter2 operator()(              //
      std::size_t num_channels,  //
      std::size_t source_len,    //
      Iter1 source,              //
      std::size_t target_len,    //
      Iter2 target) const {
    assert(source_len > 1);
    assert(source_len < target_len);
    // Supersampling: sample nearest source pixel.
    double stride = target_len / static_cast<double>(source_len);
    double top_idx_f = 0.0;
    std::size_t top_idx_i = 0;
    for (std::size_t i = 0; i < source_len; ++i) {
      std::size_t low_idx_i = top_idx_i;
      top_idx_f += stride;
      top_idx_i = top_idx_f + 0.5;
      std::size_t k = i * num_channels;
      for (std::size_t j = low_idx_i; j < top_idx_i; ++j) {
        target = std::copy_n(source + k, num_channels, target);
      }
    }
    return target;
  }
};

// Given an image with 'num_channels' channels, scales down the scanline
// pointed to by 'source' from 'source_len' elements to 'target_len', storing
// the resulting pixels in 'target'. Such pixels are computed as  the average of
// all the 'source' pixels mapped onto a single 'target' pixel, accounting for
// 'source' pixels which are split between adjacent 'target' pixels. Returns the
// iterator position past the last computed pixel.
template <typename Iter1, typename Iter2>
Iter2 averagingMinify(         //
    std::size_t num_channels,  //
    std::size_t source_len,    //
    Iter1 source,              //
    std::size_t target_len,    //
    Iter2 target) {
  std::array<double, 4> acc;
  double stride = source_len / static_cast<double>(target_len);
  double top_idx_f = 0.0;
  std::size_t top_idx_i = 0;
  for (std::size_t i = 0; i < target_len; ++i) {
    double low_idx_f = top_idx_f;
    std::size_t low_idx_i = top_idx_i;
    top_idx_f += stride;
    top_idx_i = static_cast<std::size_t>(top_idx_f);
    double low_wgt = 1.0 - (low_idx_f - low_idx_i);
    std::size_t k = low_idx_i * num_channels;
    for (std::size_t c = 0; c < num_channels; ++c) {
      acc[c] = low_wgt * source[k + c];
    }
    for (std::size_t j = low_idx_i + 1; j < top_idx_i; j++) {
      std::size_t k = j * num_channels;
      for (std::size_t c = 0; c < num_channels; ++c) {
        acc[c] += source[k + c];
      }
    }
    if (top_idx_f > top_idx_i) {
      double top_wgt = top_idx_f - top_idx_i;
      std::size_t k = std::min(top_idx_i, source_len - 1) * num_channels;
      for (std::size_t c = 0; c < num_channels; ++c) {
        acc[c] += top_wgt * source[k + c];
      }
    }
    for (std::size_t c = 0; c < num_channels; ++c) {
      *target = acc[c] / stride;
      ++target;
    }
  }
  return target;
}

// Given an single-pixel scanline using 'num_channels' channels, repeats that
// pixel 'target_len' times and stores the result in 'target'
// Returns the iterator position past the last computed pixel.
template <typename Iter1, typename Iter2>
Iter2 replicate(               //
    std::size_t num_channels,  //
    Iter1 source,              //
    std::size_t target_len,    //
    Iter2 target) {
  for (std::size_t i = 0; i < target_len; ++i) {
    target = std::copy_n(source, num_channels, target);
  }
  return target;
}

// Given an image with 'num_channels' channels, scales the scanline pointed to
// by 'source' from 'source_len' elements to 'target_len', using functor
// 'Magnifier' for supersampling and averagingMinify for subsampling.
// The resulting pixels are stored in 'target'.
// Returns the iterator position past the last computed pixel.
template <typename Iter1, typename Iter2, typename Magnifier>
Iter2 scaleRow(                  //
    std::size_t num_channels,    //
    std::size_t source_len,      //
    Iter1 source,                //
    const Magnifier& magnifier,  //
    std::size_t target_len,      //
    Iter2 target) {
  assert(num_channels <= 4);
  if (source_len > 1) {
    if (source_len < target_len) {
      target = magnifier(num_channels, source_len, source, target_len, target);
    } else if (target_len > 0) {
      target =
          averagingMinify(num_channels, source_len, source, target_len, target);
    }
  } else {
    target = replicate(num_channels, source, target_len, target);
  }
  return target;
}

// Scales all 'source_rows' rows of input image 'source' to the width given by
// 'target_cols', by repeatedly calling 'scaleRows' on each of them.
// Returns the iterator position past the last computed pixel.
template <typename Iter1, typename Iter2, typename Magnifier>
Iter2 scaleRows(                 //
    std::size_t num_channels,    //
    std::size_t source_rows,     //
    std::size_t source_cols,     //
    Iter1 source,                //
    const Magnifier& magnifier,  //
    std::size_t target_cols,     //
    Iter2 target) {
  for (std::size_t r = 0; r < source_rows; ++r) {
    target = scaleRow(num_channels, source_cols, source, magnifier, target_cols,
                      target);
    source += source_cols * num_channels;
  }
  return target;
}

// Variant of linearScaleRows which stores the results in a transposed layout.
// Returns the iterator position past the last computed pixel.
template <typename Iter1, typename Iter2, typename Magnifier>
Iter2 scaleRowsTransposed(       //
    std::size_t num_channels,    //
    std::size_t source_rows,     //
    std::size_t source_cols,     //
    Iter1 source,                //
    const Magnifier& magnifier,  //
    std::size_t target_cols,     //
    Iter2 target) {
  using T = typename std::iterator_traits<Iter2>::value_type;
  // TODO: remove the explicit transpose and copy below by storing the results
  // of scaleRows in transposed fashion, using a strided output iterator.
  std::vector<T> target_trp;
  target_trp.reserve(source_rows * target_cols * num_channels);
  scaleRows(num_channels, source_rows, source_cols, source, magnifier,
            target_cols, std::back_inserter(target_trp));
  if (target_trp.empty()) {
    return target;
  }
  tensor::TensorView<T> target_trp_mat(
      tensor::Layout({source_rows, target_cols, num_channels}), &target_trp[0]);
  target_trp_mat.Transpose(0, 1);
  target_trp_mat.ForEach([&target](T val) { *target++ = val; });
  return target;
}

// Compute an scaled image by sucessively scaling rows and columns.
// Returns the iterator position past the last computed pixel.
template <typename Iter1, typename Iter2, typename Magnifier>
Iter2 scaleImage(                //
    std::size_t num_channels,    //
    std::size_t source_rows,     //
    std::size_t source_cols,     //
    Iter1 source,                //
    const Magnifier& magnifier,  //
    std::size_t target_rows,     //
    std::size_t target_cols,     //
    Iter2 target) {
  // Scale rows and transpose.
  std::vector<double> tmp(source_rows * target_cols * num_channels);
  if (tmp.begin() == scaleRowsTransposed(num_channels, source_rows, source_cols,
                                         source, magnifier, target_cols,
                                         tmp.begin())) {
    return target;
  }
  // Scale columns and transpose.
  return scaleRowsTransposed(num_channels, target_cols, source_rows,
                             tmp.begin(), magnifier, target_rows, target);
}

lua::NResultsOr Scale(lua_State* L) {
  // Validate input parameters.
  auto* source = tensor::LuaTensor<unsigned char>::ReadObject(L, 1);
  if (source == nullptr || !source->tensor_view().IsContiguous() ||
      source->tensor_view().shape().size() != 3 ||
      source->tensor_view().shape()[2] > 4) {
    std::string error = absl::StrCat("[image.scale] - \"", lua::ToString(L, 1),
                                     "\" - Invalid source image");
    return error;
  }
  std::size_t target_rows;
  if (!lua::Read(L, 2, &target_rows)) {
    std::string error = absl::StrCat("[image.scale] - \"", lua::ToString(L, 2),
                                     "\" - Invalid number of output rows");
    return error;
  }
  std::size_t target_cols;
  if (!lua::Read(L, 3, &target_cols)) {
    std::string error = absl::StrCat("[image.scale] - \"", lua::ToString(L, 3),
                                     "\" - Invalid number of output columns");
    return error;
  }
  std::string mode = "bilinear";
  lua::Read(L, 4, &mode);
  std::size_t source_rows = source->tensor_view().shape()[0];
  std::size_t source_cols = source->tensor_view().shape()[1];
  std::size_t num_channels = source->tensor_view().shape()[2];

  // Compute the scaled image.
  std::vector<unsigned char> res(target_cols * target_rows * num_channels);
  if (mode == "bilinear") {
    if (res.begin() == scaleImage(num_channels, source_rows, source_cols,
                                  source->tensor_view().storage(),
                                  LinearMagnifier(), target_rows, target_cols,
                                  res.begin())) {
      return 0;
    }
  } else if (mode == "nearest") {
    if (res.begin() == scaleImage(num_channels, source_rows, source_cols,
                                  source->tensor_view().storage(),
                                  NearestMagnifier(), target_rows, target_cols,
                                  res.begin())) {
      return 0;
    }
  } else {
    std::string error = absl::StrCat("[image.scale] - \"", lua::ToString(L, 4),
                                     "\" - Unsupported scaling mode");
  }

  // Construct contiguous tensor and return it on the stack.
  tensor::ShapeVector res_shape = {target_rows, target_cols, num_channels};
  tensor::LuaTensor<unsigned char>::CreateObject(L, std::move(res_shape),
                                                 std::move(res));
  return 1;
}

}  // namespace

int LuaImageRequire(lua_State* L) {
  auto table = lua::TableRef::Create(L);
  table.Insert("load", &lua::Bind<Load>);
  table.Insert("scale", &lua::Bind<Scale>);
  lua::Push(L, table);
  return 1;
}

}  // namespace lab
}  // namespace deepmind
