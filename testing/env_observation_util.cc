// Copyright (C) 2016 Google Inc.
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

// jpeglib requires FILE to be defined prior to inclusion.
#include <cstdio>
#include <jpeglib.h>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>
#include <numeric>
#include <vector>

#include "testing/env_observation_util.h"

namespace deepmind {
namespace lab {
namespace {

// Resamples an interlaced observation of byte format to grid_size x grid_size.
std::vector<double> ResampleInterlaced(
    const EnvObservation<unsigned char>& obs, int grid_size) {
  assert(obs.shape().size() == 3);
  const int height = obs.shape()[0];
  const int width = obs.shape()[1];
  const int colours = obs.shape()[2];
  const int row_size = width * colours;

  std::vector<double> buckets(grid_size * grid_size * colours, 0);
  std::vector<double> buckets_norm(buckets.size(), 0);

  int count = obs.payload().size();
  for (int loc = 0; loc < count; ++loc) {
    int row = grid_size * (loc / row_size) / height;
    int col = grid_size * (loc % row_size) / row_size;
    int colour = loc % colours;
    int bucket = (row * colours * grid_size) + (col * colours) + colour;
    buckets[bucket] += obs.payload()[loc];
    buckets_norm[bucket] += 1;
  }

  std::transform(buckets.begin(), buckets.end(), buckets_norm.begin(),
                 buckets.begin(), std::divides<double>());
  return buckets;
}

}  // namespace

double CompareInterlacedObservations(const EnvObservation<unsigned char>& lhs,
                                     const EnvObservation<unsigned char>& rhs,
                                     int grid_size) {
  std::vector<double> histogram_l(ResampleInterlaced(lhs, grid_size));
  std::vector<double> histogram_r(ResampleInterlaced(rhs, grid_size));

  return std::inner_product(
      histogram_l.begin(), histogram_l.end(), histogram_r.begin(), 0.0,
      std::plus<double>(), [](double a, double b) { return std::abs(a - b); });
}

void SaveInterlacedRGBObservationToJpg(
    const EnvObservation<unsigned char>& obs, const std::string& path) {
  unsigned char* bytes = const_cast<unsigned char*>(&obs.payload()[0]);
  int width = obs.shape()[1];
  int height = obs.shape()[0];

  const int kColourComponents = 3;
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;
  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_compress(&cinfo);
  cinfo.image_width = width;
  cinfo.image_height = height;
  cinfo.input_components = kColourComponents;
  cinfo.in_color_space = JCS_RGB;
  std::FILE* outfile = std::fopen(path.c_str(), "wb");
  jpeg_stdio_dest(&cinfo, outfile);
  jpeg_set_defaults(&cinfo);
  jpeg_start_compress(&cinfo, TRUE);
  while (cinfo.next_scanline < cinfo.image_height) {
    JSAMPROW samp = &bytes[cinfo.next_scanline * kColourComponents * width];
    jpeg_write_scanlines(&cinfo, &samp, 1);
  }
  jpeg_finish_compress(&cinfo);
  fclose(outfile);
  jpeg_destroy_compress(&cinfo);
}

}  // namespace lab
}  // namespace deepmind
