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

#include "deepmind/level_generation/text_level/translate_text_level.h"

#include <fstream>
#include <iterator>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include "absl/flags/flag.h"
#include "absl/log/check.h"
#include "absl/log/log.h"
#include "absl/strings/string_view.h"
#include "deepmind/level_generation/text_level/text_level_settings.h"
#include "deepmind/support/test_srcdir.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

ABSL_FLAG(
    std::string, map_outfile, "",
    "If this argument is provided, write the .map output of the test map to "
    "the specified file. Useful for testing other parts of the pipeline.");

namespace deepmind {
namespace lab {
namespace {

using ::testing::AnyOf;
using ::testing::Eq;

constexpr char kEntities[] =
    "   ******\n"
    " ***    *********\n"
    " *    x    I    *\n"
    " ** P ***H****H**\n"
    "  *   *     *   *\n"
    "  ***************\n";

constexpr char kVariations[] =
    ".........\n"
    "....AAAAAAAAAAAAAA\n"
    ".\n"
    ".CCCCCCCCCCCCCCCCCCCCCCCCCC\n"
    ".CCCCCCCCCCCCCCCCCCCCCCC\n"
    "$%^&*()\n"
    ".............\n";

bool NoOp(std::size_t, std::size_t, char, const MapSnippetEmitter&,
          std::vector<std::string>*) {
  return false;
}

TEST(TranslateTextLevel, Simple) {
  std::mt19937_64 rng(123);
  TextLevelSettings settings;
  std::string actual = TranslateTextLevel(kEntities, kVariations, &rng, NoOp,
                                          &settings);

  ASSERT_THAT(actual,
              testing::HasSubstr("\"classname\" \"info_player_start\""));
  ASSERT_THAT(actual, testing::HasSubstr("\"classname\" \"worldspawn\""));
  ASSERT_THAT(actual, testing::HasSubstr("\"classname\" \"light\""));
  ASSERT_THAT(actual, testing::HasSubstr("\"classname\" \"func_door\""));
  ASSERT_THAT(actual, testing::HasSubstr("\"classname\" \"trigger_multiple\""));

  if (FLAGS_map_outfile.IsSpecifiedOnCommandLine()) {
    std::string filename = absl::GetFlag(FLAGS_map_outfile);
    if (std::ofstream(filename) << actual) {
      LOG(INFO) << "Test map written to '" << filename << "'.";
    } else {
      LOG(ERROR) << "Failed to write map file to '" << filename << "'.";
    }
  }
}

// The utility of this test is somewhat limited: The output depends on the
// specific implementation of various random algorithms, and thus it is
// really only reliable on a fixed platform. When moving platforms, you will
// most likely need to generate a new golden output file.
TEST(TranslateTextLevel, CompareWithGolden) {
  std::mt19937_64 rng(123);
  TextLevelSettings settings;
  std::string actual = TranslateTextLevel(kEntities, kVariations, &rng, NoOp,
                                          &settings);

  std::ifstream golden_libcxx(
      TestSrcDir() +
      "/deepmind/level_generation/text_level/"
      "translate_text_level_test_libc++.golden_output");

  QCHECK(golden_libcxx) << "Failed to open golden libc data file.";
  std::ifstream golden_libstdcxx(
      TestSrcDir() +
      "/deepmind/level_generation/text_level/"
      "translate_text_level_test_libstdc++.golden_output");
  QCHECK(golden_libstdcxx) << "Failed to open golden libstdc data file.";

  std::string expected_libc(std::istreambuf_iterator<char>(golden_libcxx), {});
  std::string expected_libstdc(std::istreambuf_iterator<char>(golden_libstdcxx),
                                                              {});
  EXPECT_THAT(actual, AnyOf(Eq(expected_libc), Eq(expected_libstdc)));
}

TEST(TranslateTextLevel, Custom) {
  auto callback = [](std::size_t i, std::size_t j, char ent,
                     const MapSnippetEmitter& em,
                     std::vector<std::string>* out) {
    if (ent != 'x') return false;
    out->push_back(
        em.AddEntity(i, j, 0, "XyzzyEntity", {{"a", "1"}, {"b", "2"}}));
    return true;
  };

  std::mt19937_64 rng(123);

  TextLevelSettings settings;
  std::string actual = TranslateTextLevel(
      kEntities, kVariations, &rng, callback, &settings);

  EXPECT_THAT(actual, testing::HasSubstr(
      "{\n"
      "  \"classname\" \"XyzzyEntity\"\n"
      "  \"a\" \"1\"\n"
      "  \"b\" \"2\"\n"
      "  \"origin\" \"650 350 30\"\n"
      "}"));
}

TEST(TranslateTextLevel, SkyBox) {
  std::mt19937_64 rng(123);
  TextLevelSettings exporter_settings;
  exporter_settings.skybox_texture_name = "map/lab_games/sky/lg_sky_01";
  std::string actual = TranslateTextLevel(kEntities, kVariations, &rng, NoOp,
                                          &exporter_settings);

  // Test skybox brushes
  EXPECT_THAT(actual, testing::HasSubstr(
      "  {\n"
      "    ( -626 0 0 ) ( -626 32 0 ) ( -626 0 32 ) "
      "map/lab_games/sky/lg_sky_01_up 378 0 0 -0.375 0.03125 0 0 0\n"
      "    ( -242 0 0 ) ( -242 0 32 ) ( -242 32 0 ) "
      "map/lab_games/sky/lg_sky_01_up 378 0 0 -0.375 0.03125 0 0 0\n"
      "    ( 0 -626 0 ) ( 0 -626 32 ) ( 32 -626 0 ) "
      "map/lab_games/sky/lg_sky_01_up 378 0 0 -0.375 0.03125 0 0 0\n"
      "    ( 0 -242 0 ) ( 32 -242 0 ) ( 0 -242 32 ) "
      "map/lab_games/sky/lg_sky_01_up 378 0 0 -0.375 0.03125 0 0 0\n"
      "    ( 0 0 192 ) ( 32 0 192 ) ( 0 32 192 ) "
      "map/lab_games/sky/lg_sky_01_up 378 378 0 -0.375 0.375 0 0 0\n"
      "    ( 0 0 224 ) ( 0 32 224 ) ( 32 0 224 ) "
      "map/lab_games/sky/lg_sky_01_up 378 378 0 -0.375 0.375 0 0 0\n"
      "  }\n"
      "  {\n"
      "    ( -626 0 0 ) ( -626 32 0 ) ( -626 0 32 ) "
      "map/lab_games/sky/lg_sky_01_dn 378 0 0 -0.375 0.03125 0 0 0\n"
      "    ( -242 0 0 ) ( -242 0 32 ) ( -242 32 0 ) "
      "map/lab_games/sky/lg_sky_01_dn 378 0 0 -0.375 0.03125 0 0 0\n"
      "    ( 0 -626 0 ) ( 0 -626 32 ) ( 32 -626 0 ) "
      "map/lab_games/sky/lg_sky_01_dn 378 0 0 -0.375 0.03125 0 0 0\n"
      "    ( 0 -242 0 ) ( 32 -242 0 ) ( 0 -242 32 ) "
      "map/lab_games/sky/lg_sky_01_dn 378 0 0 -0.375 0.03125 0 0 0\n"
      "    ( 0 0 -224 ) ( 32 0 -224 ) ( 0 32 -224 ) "
      "map/lab_games/sky/lg_sky_01_dn 378 378 0 -0.375 0.375 0 0 0\n"
      "    ( 0 0 -192 ) ( 0 32 -192 ) ( 32 0 -192 ) "
      "map/lab_games/sky/lg_sky_01_dn 378 378 0 -0.375 0.375 0 0 0\n"
      "  }\n"
      "  {\n"
      "    ( -658 0 0 ) ( -658 32 0 ) ( -658 0 32 ) "
      "map/lab_games/sky/lg_sky_01_lf 378 512 0 -0.375 0.375 0 0 0\n"
      "    ( -626 0 0 ) ( -626 0 32 ) ( -626 32 0 ) "
      "map/lab_games/sky/lg_sky_01_lf 378 512 0 -0.375 0.375 0 0 0\n"
      "    ( 0 -626 0 ) ( 0 -626 32 ) ( 32 -626 0 ) "
      "map/lab_games/sky/lg_sky_01_lf 448 512 0 -0.03125 0.375 0 0 0\n"
      "    ( 0 -242 0 ) ( 32 -242 0 ) ( 0 -242 32 ) "
      "map/lab_games/sky/lg_sky_01_lf 448 512 0 -0.03125 0.375 0 0 0\n"
      "    ( 0 0 -192 ) ( 32 0 -192 ) ( 0 32 -192 ) "
      "map/lab_games/sky/lg_sky_01_lf 448 378 0 -0.03125 0.375 0 0 0\n"
      "    ( 0 0 192 ) ( 0 32 192 ) ( 32 0 192 ) "
      "map/lab_games/sky/lg_sky_01_lf 448 378 0 -0.03125 0.375 0 0 0\n"
      "  }\n"
      "  {\n"
      "    ( -242 0 0 ) ( -242 32 0 ) ( -242 0 32 ) "
      "map/lab_games/sky/lg_sky_01_rt 378 512 0 -0.375 0.375 0 0 0\n"
      "    ( -210 0 0 ) ( -210 0 32 ) ( -210 32 0 ) "
      "map/lab_games/sky/lg_sky_01_rt 378 512 0 -0.375 0.375 0 0 0\n"
      "    ( 0 -626 0 ) ( 0 -626 32 ) ( 32 -626 0 ) "
      "map/lab_games/sky/lg_sky_01_rt 448 512 0 -0.03125 0.375 0 0 0\n"
      "    ( 0 -242 0 ) ( 32 -242 0 ) ( 0 -242 32 ) "
      "map/lab_games/sky/lg_sky_01_rt 448 512 0 -0.03125 0.375 0 0 0\n"
      "    ( 0 0 -192 ) ( 32 0 -192 ) ( 0 32 -192 ) "
      "map/lab_games/sky/lg_sky_01_rt 448 378 0 -0.03125 0.375 0 0 0\n"
      "    ( 0 0 192 ) ( 0 32 192 ) ( 32 0 192 ) "
      "map/lab_games/sky/lg_sky_01_rt 448 378 0 -0.03125 0.375 0 0 0\n"
      "  }\n"
      "  {\n"
      "    ( -626 0 0 ) ( -626 32 0 ) ( -626 0 32 ) "
      "map/lab_games/sky/lg_sky_01_ft 448 512 0 -0.03125 0.375 0 0 0\n"
      "    ( -242 0 0 ) ( -242 0 32 ) ( -242 32 0 ) "
      "map/lab_games/sky/lg_sky_01_ft 448 512 0 -0.03125 0.375 0 0 0\n"
      "    ( 0 -658 0 ) ( 0 -658 32 ) ( 32 -658 0 ) "
      "map/lab_games/sky/lg_sky_01_ft 378 512 0 -0.375 0.375 0 0 0\n"
      "    ( 0 -626 0 ) ( 32 -626 0 ) ( 0 -626 32 ) "
      "map/lab_games/sky/lg_sky_01_ft 378 512 0 -0.375 0.375 0 0 0\n"
      "    ( 0 0 -192 ) ( 32 0 -192 ) ( 0 32 -192 ) "
      "map/lab_games/sky/lg_sky_01_ft 378 448 0 -0.375 0.03125 0 0 0\n"
      "    ( 0 0 192 ) ( 0 32 192 ) ( 32 0 192 ) "
      "map/lab_games/sky/lg_sky_01_ft 378 448 0 -0.375 0.03125 0 0 0\n"
      "  }\n"
      "  {\n"
      "    ( -626 0 0 ) ( -626 32 0 ) ( -626 0 32 ) "
      "map/lab_games/sky/lg_sky_01_bk 448 512 0 -0.03125 0.375 0 0 0\n"
      "    ( -242 0 0 ) ( -242 0 32 ) ( -242 32 0 ) "
      "map/lab_games/sky/lg_sky_01_bk 448 512 0 -0.03125 0.375 0 0 0\n"
      "    ( 0 -242 0 ) ( 0 -242 32 ) ( 32 -242 0 ) "
      "map/lab_games/sky/lg_sky_01_bk 378 512 0 -0.375 0.375 0 0 0\n"
      "    ( 0 -210 0 ) ( 32 -210 0 ) ( 0 -210 32 ) "
      "map/lab_games/sky/lg_sky_01_bk 378 512 0 -0.375 0.375 0 0 0\n"
      "    ( 0 0 -192 ) ( 32 0 -192 ) ( 0 32 -192 ) "
      "map/lab_games/sky/lg_sky_01_bk 378 448 0 -0.375 0.03125 0 0 0\n"
      "    ( 0 0 192 ) ( 0 32 192 ) ( 32 0 192 ) "
      "map/lab_games/sky/lg_sky_01_bk 378 448 0 -0.375 0.03125 0 0 0\n  }"));
  // Test skybox entity
  EXPECT_THAT(actual, testing::HasSubstr(
      "{\n"
      "  \"classname\" \"_skybox\"\n"
      "  \"origin\" \"-434 -434 0\"\n"
      "}"));
}

TEST(TranslateTextLevel, MakeFenceDoor) {
  auto callback = [](std::size_t i, std::size_t j, char ent,
                     const MapSnippetEmitter& em,
                     std::vector<std::string>* out) {
    if (ent == 'I' || ent == 'H') {
      out->push_back(em.AddFenceDoor(i, j, ent));
      return true;
    }
    return false;
  };

  std::mt19937_64 rng(123);
  TextLevelSettings settings;
  std::string actual = TranslateTextLevel(
      kEntities, kVariations, &rng, callback, &settings);

  // Test the first vertical fence door.
  EXPECT_THAT(actual, testing::HasSubstr(
      "\"angle\" \"90\"\n  \"targetname\" \"door_11_3\"\n"));
  EXPECT_THAT(actual, testing::HasSubstr(
      "( 1146 0 0 ) ( 1146 32 0 ) ( 1146 0 32 ) door_placeholder:door_11_3"));
  // Test the first horizontal fence door.
  EXPECT_THAT(actual, testing::HasSubstr(
      "\"angle\" \"0\"\n  \"targetname\" \"door_9_2\"\n"));
  EXPECT_THAT(actual, testing::HasSubstr(
      "( 901 0 0 ) ( 901 32 0 ) ( 901 0 32 ) door_placeholder:door_9_2"));
}

}  // namespace
}  // namespace lab
}  // namespace deepmind
