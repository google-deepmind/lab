// Copyright (C) 2017-2019 Google Inc.
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
#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <iterator>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/string_view.h"
#include "deepmind/lua/bind.h"
#include "deepmind/lua/n_results_or.h"
#include "deepmind/lua/push.h"
#include "deepmind/lua/read.h"
#include "deepmind/lua/table_ref.h"
#include "deepmind/tensor/lua_tensor.h"
#include "deepmind/tensor/tensor_view.h"
#include "deepmind/util/file_reader.h"
#include "public/file_reader_types.h"
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
  absl::string_view contents;
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

lua::NResultsOr LoadPng(lua_State* L, absl::string_view contents) {
  Reader reader{contents, 0};
  constexpr std::size_t png_header_size = 8;
  if (reader.contents.size() < png_header_size) {
    return "Invalid format. Contents too short.";
  }

  if (!png_check_sig(reinterpret_cast<png_const_bytep>(&reader.contents[0]),
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
  const DeepMindReadOnlyFileSystem* fs = nullptr;
  if (IsTypeMismatch(lua::Read(L, lua_upvalueindex(1), &fs))) {
    return "[image.load] Invalid filesystem in upvalue";
  }
  if (fs == nullptr) {
    return "[image.load] Internal error - missing DeepMindReadOnlyFileSystem.";
  }
  std::string file_name;
  if (!lua::Read(L, 1, &file_name) || file_name.size() < 4) {
    return absl::StrCat("[image.load] - \"", lua::ToString(L, 1),
                        "\" - Invalid name");
  }

  absl::string_view contents;
  std::unique_ptr<char[]> data;
  if (file_name.compare(0, file_name.length() - 4, "content:") == 0) {
    if (!lua::Read(L, 2, &contents)) {
      return "[image.load] - Missing contents.";
    }
  } else {
    util::FileReader reader(fs, file_name.c_str());
    if (![&reader, &data, &contents] {
          if (!reader.Success()) return false;
          std::size_t size;
          if (!reader.GetSize(&size)) return false;
          data.reset(new char[size]);
          contents = {data.get(), size};
          return reader.Read(0, size, data.get());
        }()) {
      return absl::StrCat("[image.load] - \"", file_name,
                          "\" could not be read.");
    }
  }

  if (file_name.compare(file_name.length() - 4, 4, ".png") == 0) {
    auto status = LoadPng(L, contents);
    if (!status.ok()) {
      return absl::StrCat("[image.load] (PNG) - \"", file_name, "\" - ",
                          status.error());
    }
    return status;
  }
  return absl::StrCat("[image.load] - \"", file_name,
                      "\" - Unsupported file type.");
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
      for (std::size_t c = 0; c < num_channels; ++c) {
        acc[c] += source[j * num_channels + c];
      }
    }
    if (top_idx_f > top_idx_i) {
      for (std::size_t c = 0; c < num_channels; ++c) {
        acc[c] +=
            (top_idx_f - top_idx_i) *
            source[std::min(top_idx_i, source_len - 1) * num_channels + c];
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
    return absl::StrCat("[image.scale] - \"", lua::ToString(L, 1),
                        "\" - Invalid source image");
  }
  std::size_t target_rows;
  if (!lua::Read(L, 2, &target_rows)) {
    return absl::StrCat("[image.scale] - \"", lua::ToString(L, 2),
                        "\" - Invalid number of output rows");
  }
  std::size_t target_cols;
  if (!lua::Read(L, 3, &target_cols)) {
    return absl::StrCat("[image.scale] - \"", lua::ToString(L, 3),
                        "\" - Invalid number of output columns");
  }
  std::string mode = "bilinear";
  lua::Read(L, 4, &mode);
  const auto& view = source->tensor_view();
  std::size_t source_rows = view.shape()[0];
  std::size_t source_cols = view.shape()[1];
  std::size_t num_channels = view.shape()[2];

  // Compute the scaled image.
  const unsigned char* tensor_start = &view.storage()[view.start_offset()];
  std::vector<unsigned char> res(target_cols * target_rows * num_channels);
  if (mode == "bilinear") {
    if (res.begin() == scaleImage(num_channels, source_rows, source_cols,
                                  tensor_start, LinearMagnifier(), target_rows,
                                  target_cols, res.begin())) {
      return 0;
    }
  } else if (mode == "nearest") {
    if (res.begin() == scaleImage(num_channels, source_rows, source_cols,
                                  tensor_start, NearestMagnifier(), target_rows,
                                  target_cols, res.begin())) {
      return 0;
    }
  } else {
    return absl::StrCat("[image.scale] - \"", lua::ToString(L, 4),
                        "\" - Unsupported scaling mode");
  }

  // Construct contiguous tensor and return it on the stack.
  tensor::ShapeVector res_shape = {target_rows, target_cols, num_channels};
  tensor::LuaTensor<unsigned char>::CreateObject(L, std::move(res_shape),
                                                 std::move(res));
  return 1;
}

// Converts RGB to S and L components of HSL.
void RgbToSl(unsigned char r, unsigned char g, unsigned char b, double* s,
             double* l) {
  auto min_max = std::minmax({r, g, b});
  double hmin = (0.5 / 255.0) * min_max.first;
  double hmax = (0.5 / 255.0) * min_max.second;
  *l = hmax + hmin;
  double hdif = hmax - hmin;
  if (min_max.first == min_max.second) {
    *s = 0;
  } else if (*l > 0.5) {
    *s = hdif / (1.0 - *l);
  } else {
    *s = hdif / *l;
  }
}

// Converts Hue Saturation and Lightness to RGB.
// Hue specific calculations are performed at construction time.
class HslToRgb {
 public:
  // 'hue' Must be an angle in degrees.
  explicit HslToRgb(double hue) {
    hue_prime_ = hue / 60.0;
    if (!(0 <= hue_prime_ && hue_prime_ < 6.0)) {
      hue_prime_ -= 6.0 * std::floor(hue_prime_ / 6.0);
    }
    double hue_mod_2 = hue_prime_ - 2.0 * (std::floor(hue_prime_ / 2.0));
    hue_partial_ = 1.0 - std::abs(hue_mod_2 - 1.0);
    hue_case_ = static_cast<int>(hue_prime_);
  }

  // Perform the conversion. 'l' and 's' must be in the range [0, 1].
  void operator()(double s, double l, unsigned char* r, unsigned char* g,
                  unsigned char* b) {
    double c = s * (1.0 - std::abs(2.0 * l - 1.0));
    double x = c * hue_partial_;
    double m = l - 0.5 * c;
    c += m;
    x += m;
    m *= 255.0;
    x *= 255.0;
    c *= 255.0;
    switch (hue_case_) {
      default:
      case 0:
        *r = static_cast<unsigned char>(c);
        *g = static_cast<unsigned char>(x);
        *b = static_cast<unsigned char>(m);
        break;
      case 1:
        *r = static_cast<unsigned char>(x);
        *g = static_cast<unsigned char>(c);
        *b = static_cast<unsigned char>(m);
        break;
      case 2:
        *r = static_cast<unsigned char>(m);
        *g = static_cast<unsigned char>(c);
        *b = static_cast<unsigned char>(x);
        break;
      case 3:
        *r = static_cast<unsigned char>(m);
        *g = static_cast<unsigned char>(x);
        *b = static_cast<unsigned char>(c);
        break;
      case 4:
        *r = static_cast<unsigned char>(x);
        *g = static_cast<unsigned char>(m);
        *b = static_cast<unsigned char>(c);
        break;
      case 5:
        *r = static_cast<unsigned char>(c);
        *g = static_cast<unsigned char>(m);
        *b = static_cast<unsigned char>(x);
        break;
    }
  }

 private:
  int hue_case_;
  double hue_prime_;
  double hue_partial_;
};

// Sets hue of an RGB image.
//
// Lua Arguments:
//
// 1.  image - A contiguous ByteTensor with 3 or 4 channels in last dimension.
// 2.  hue - Desired hue (in degrees).
//
// Given an RGB image tensor (`img`), set its hue to a specified value (`hue`).
// This is done by converting the image into Hue, Saturation, and Lightness
// colorspace, setting the Hue to `hue`, and converting back again. The
// operation is done in-place on the supplied image tensor.
//
// Notes:
//
// As the conversion is done via only modifying the hue in the HSL colour space,
// saturation and lightness remain unaltered (i.e., whites remain whites and
// blacks remain blacks).
//
// Returns updated `img`.
// [2, 1, e]
lua::NResultsOr SetHue(lua_State* L) {
  // Validate input parameters.
  auto* source = tensor::LuaTensor<unsigned char>::ReadObject(L, 1);
  if (source == nullptr) {
    return absl::StrCat("[image.setHue] - \"", lua::ToString(L, 1),
                        "\" - Invalid source image");
  }
  auto* view = source->mutable_tensor_view();
  if (view->shape().empty() ||
      (view->shape().back() != 3 && view->shape().back() != 4)) {
    return absl::StrCat(
        "[image.setHue] - Image shape does not have correct channel count");
  }
  if (!source->tensor_view().IsContiguous()) {
    return "[image.setHue] - Image not contiguous!";
  }
  double hue;
  if (!IsFound(lua::Read(L, 2, &hue))) {
    return "[image.setHue] - missing arg2 - hue";
  }

  HslToRgb sl_to_rgb(hue);
  std::size_t num_elements = view->num_elements();
  std::size_t num_channels = view->shape().back();
  unsigned char* tensor_start = &view->mutable_storage()[view->start_offset()];
  for (std::size_t i = 0; i < num_elements; i += num_channels) {
    unsigned char* r = &tensor_start[i + 0];
    unsigned char* g = &tensor_start[i + 1];
    unsigned char* b = &tensor_start[i + 2];
    double s, l;
    RgbToSl(*r, *g, *b, &s, &l);
    sl_to_rgb(s, l, r, g, b);
  }
  return 1;
}

// Careful benchmarking is required when refactoring this code.
void ApplyPattern(unsigned char* source_start,
                          const unsigned char* pattern_start,
                          std::size_t pattern_span, std::size_t pixel_count,
                          unsigned char r1, unsigned char g1, unsigned char b1,
                          unsigned char r2, unsigned char g2,
                          unsigned char b2) {
  for (std::size_t pixel_idx = 0; pixel_idx < pixel_count; ++pixel_idx) {
    unsigned char* source = &source_start[pixel_idx * 4];
    const unsigned char pattern = pattern_start[pixel_idx * pattern_span];
    const int pattern_w_0 = pattern;
    const int pattern_w_1 = 255 - pattern;
    const int pattern_0 = (pattern_w_0 * r1 + pattern_w_1 * r2 + 127) / 255;
    const int pattern_1 = (pattern_w_0 * g1 + pattern_w_1 * g2 + 127) / 255;
    const int pattern_2 = (pattern_w_0 * b1 + pattern_w_1 * b2 + 127) / 255;

    const int source_w_0 = source[3];
    const int source_w_1 = 255 - source_w_0;
    source[0] = (source_w_0 * pattern_0 + source_w_1 * source[0] + 127) / 255;
    source[1] = (source_w_0 * pattern_1 + source_w_1 * source[1] + 127) / 255;
    source[2] = (source_w_0 * pattern_2 + source_w_1 * source[2] + 127) / 255;
    source[3] = 255;
  }
}

// Overlays a colored pattern onto a source image using its alpha channel as an
// opacity mask.
//
// function image.setMaskedPattern(img, pattern, color1, color2)
//
// Lua Arguments:
//
// 1.  'img' - ByteTensor HxWx4 contiguous. Source image to apply pattern to.
// 2.  'pattern' - ByteTensor HxWx? contiguous (assumed to be grayscale so
//     only the first channel used).
// 3.  'color1' - {R, G, B} White in pattern image is mapped to color1.
// 4.  'color2' - {R, G, B} Black in pattern image is mapped to color2.
//
// R, G, and B must be integers in the range [0, 255].
// The pattern color is the linear interpolation of color1 and color2 according
// to the 'pattern' image's first channel. The linear interpolation is between
// color2 for pattern value 0 and color1 for pattern value 255.
//
// Each pixel in the source image 'img' is interpolated between the
// corresponding pattern color as defined above and its own original color. The
// linear interpolation is between source image colour where the source image's
// alpha is 0 and the pattern color where the source image's alpha is 255. The
// source image's alpha channel is then set to 255.
//
// Returns the modified source image.
lua::NResultsOr SetMaskedPattern(lua_State* L) {
  if (lua_gettop(L) != 4) {
    return absl::StrCat(
        "[image.setMaskedPattern] - Incorrect number of arguments. Expected: "
        "4, received: ",
        lua_gettop(L));
  }

  auto* source = tensor::LuaTensor<unsigned char>::ReadObject(L, 1);
  if (source == nullptr) {
    return absl::StrCat("[image.setMaskedPattern] - Arg1 \"",
                        lua::ToString(L, 1), "\" - Invalid source image");
  }
  const auto* pattern = tensor::LuaTensor<unsigned char>::ReadObject(L, 2);
  if (pattern == nullptr) {
    return absl::StrCat("[image.setMaskedPattern] - Arg2 \"",
                        lua::ToString(L, 2), "\" - Invalid pattern image");
  }

  std::array<unsigned char, 3> color1;
  std::array<unsigned char, 3> color2;
  if (!IsFound(lua::Read(L, 3, &color1))) {
    return absl::StrCat("[image.setMaskedPattern] - Arg 3 \"",
                        lua::ToString(L, 3), "\" - Invalid color1.");
  }
  if (!IsFound(lua::Read(L, 4, &color2))) {
    return absl::StrCat("[image.setMaskedPattern] - Arg 4 \"",
                        lua::ToString(L, 3), "\" - Invalid color2.");
  }

  auto* source_view = source->mutable_tensor_view();
  const auto& pattern_view = pattern->tensor_view();

  if (source_view->shape().size() != pattern_view.shape().size()) {
    return absl::StrCat(
        "[image.setMaskedPattern] Arg1 (source) must have the same number of "
        "dimensions as Arg2 (pattern)",
        "Arg 1 (source) shape : [", absl::StrJoin(source_view->shape(), ", "),
        "] Arg 2 (pattern) shape : [",
        absl::StrJoin(pattern_view.shape(), ", "), "]");
  }

  // Must have the same major dimensions.
  for (std::size_t i = 0; i + 1 < source_view->shape().size(); ++i) {
    if (source_view->shape()[i] != pattern_view.shape()[i]) {
      return absl::StrCat(
          "[image.setMaskedPattern] Arg1 (source) must have the same major "
          "dimensions as Arg2 (pattern).",
          "Arg 1 (source) shape : [", absl::StrJoin(source_view->shape(), ", "),
          "] Arg 2 (pattern) shape : [",
          absl::StrJoin(pattern_view.shape(), ", "), "]");
    }
  }

  if (source_view->num_elements() == 0 || source_view->shape().back() != 4) {
    return "[image.setMaskedPattern] Arg1 (source) shape must have 4 channels.";
  }

  std::size_t number_pixels = source_view->num_elements() / 4;
  int pattern_span = pattern_view.shape().back();
  unsigned char* source_start =
      &source_view->mutable_storage()[source_view->start_offset()];
  const unsigned char* pattern_start =
      &pattern_view.storage()[pattern_view.start_offset()];
  ApplyPattern(source_start, pattern_start, pattern_span, number_pixels,
               color1[0], color1[1], color1[2], color2[0], color2[1],
               color2[2]);
  lua_settop(L, 1);
  return 1;
}

}  // namespace

int LuaImageRequire(lua_State* L) {
  auto table = lua::TableRef::Create(L);
  void* fs = nullptr;
  lua::Read(L, lua_upvalueindex(1), &fs);
  lua_pushlightuserdata(L, fs);
  lua_pushcclosure(L, &lua::Bind<Load>, 1);
  table.InsertFromStackTop("load");

  table.Insert("scale", &lua::Bind<Scale>);
  table.Insert("setHue", &lua::Bind<SetHue>);
  table.Insert("setMaskedPattern", &lua::Bind<SetMaskedPattern>);
  lua::Push(L, table);
  return 1;
}

}  // namespace lab
}  // namespace deepmind
