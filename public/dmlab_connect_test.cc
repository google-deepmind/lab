// Copyright (C) 2019 Google LLC
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

#include "public/dmlab.h"

#include <string>

#include "gtest/gtest.h"
#include "deepmind/support/test_srcdir.h"
#include "third_party/rl_api/env_c_api.h"
#include "third_party/rl_api/env_c_api_test_suite.h"

namespace deepmind {
namespace lab {
namespace {

using rl_api::EnvCApiConformanceTestSuite;
using rl_api::EnvCApiTestParam;

int test_connect(EnvCApi* api, void** context) {
  std::string runfiles = TestSrcDir();
  DeepMindLabLaunchParams params = {};
  params.runfiles_path = runfiles.c_str();
  return dmlab_connect(&params, api, context);
}

INSTANTIATE_TEST_SUITE_P(
    DeepmindLabEnvCApiConformanceTest,
    EnvCApiConformanceTestSuite,
    testing::Values(EnvCApiTestParam{test_connect, {
        {"levelName", "lt_chasm"},
#ifndef __has_feature
#  define __has_feature(x) 0
#endif
#if __has_feature(thread_sanitizer)
        {"vmMode", "interpreted"},
#endif
    }}));

}  // namespace
}  // namespace lab
}  // namespace deepmind
