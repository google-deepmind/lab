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

#include "deepmind/level_generation/compile_map.h"

#include <libgen.h>

#include <algorithm>
#include <array>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

#include "absl/log/check.h"
#include "absl/log/log.h"
#include "absl/strings/str_cat.h"
#include "deepmind/util/files.h"
#include "deepmind/util/run_executable.h"
#include "public/level_cache_types.h"
#include "third_party/md/md5.h"

namespace deepmind {
namespace lab {
namespace {

constexpr char kScript[] = "deepmind/level_generation/compile_map.sh";

// Converts an md5 `digest` to a hex string.
std::string ToHex(const std::array<unsigned char, MD5_DIGEST_LENGTH>& digest) {
  std::string result;
  result.reserve(digest.size() * 2);
  static const char digits[] = "0123456789abcdef";
  for (unsigned char d : digest) {
    result.push_back(digits[d / 16]);
    result.push_back(digits[d % 16]);
  }
  return result;
}

}  // namespace

namespace internal {

std::string CalculateMd5(const std::string& file_name) {
  std::array<unsigned char, MD5_DIGEST_LENGTH> digest;
  MD5_CTX md5_state;
  MD5_Init(&md5_state);
  std::ifstream stream(file_name);
  do {
    std::array<char, 2048> read_buffer;
    stream.read(read_buffer.data(), read_buffer.size());
    MD5_Update(&md5_state, reinterpret_cast<unsigned char*>(read_buffer.data()),
               stream.gcount());
  } while (stream);

  MD5_Final(digest.data(), &md5_state);
  return ToHex(digest);
}

}  // namespace internal

bool RunMapCompileFor(const std::string& rundir, const std::string& base,
                      const MapCompileSettings& compile_settings) {
  std::vector<std::string> paths;
  if (compile_settings.use_local_level_cache) {
    paths.emplace_back(
        absl::StrCat(util::GetTempDirectory(), "/dmlab_level_cache"));
  }


  std::vector<const char*> c_paths;
  c_paths.reserve(paths.size());
  std::transform(paths.begin(), paths.end(), std::back_inserter(c_paths),
                 [](const std::string& s) { return s.c_str(); });

  const std::string map_filename = compile_settings.map_source_location.empty()
                                       ? std::string(absl::StrCat(base, ".map"))
                                       : compile_settings.map_source_location;
  const std::string md5 = internal::CalculateMd5(map_filename);

  std::string base_cpy(base);
  const std::string map_basename = basename(&base_cpy[0]);
  const std::string key = absl::StrCat(
      map_basename, compile_settings.generate_aas ? "_aas_" : "_noaas_", md5);

  const std::string pk3_filename = absl::StrCat(base, ".pk3");

  if (compile_settings.level_cache_params.fetch_level_from_cache &&
      compile_settings.level_cache_params.fetch_level_from_cache(
          compile_settings.level_cache_params.context, c_paths.data(),
          c_paths.size(), key.c_str(), pk3_filename.c_str())) {
    return true;
  }

  // Level was not found in the cache; build it and write it to the cache (if
  // enabled).
  std::string cmd = absl::StrCat(rundir, "/", kScript);

  if (compile_settings.generate_aas) {
    cmd += " -a";
  }

  if (!compile_settings.map_source_location.empty()) {
    cmd += " -m " + compile_settings.map_source_location;
  }

  cmd += " \"" + base + "\"";
  std::string msg;
  std::string output;

  bool res = util::RunExecutableWithOutput(cmd.c_str(), &msg, &output);

  if (res) {
    if (compile_settings.level_cache_params.write_level_to_cache) {
      compile_settings.level_cache_params.write_level_to_cache(
          compile_settings.level_cache_params.context, c_paths.data(),
          c_paths.size(), key.c_str(), pk3_filename.c_str());
    }
  } else {
    LOG(ERROR) << output;
  }
  return res;
}

}  // namespace lab
}  // namespace deepmind
