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
#include <string>

#include "deepmind/support/commandlineflags.h"
#include "deepmind/support/logging.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "deepmind/support/test_srcdir.h"

DEFINE_FLAG(
    string, map_outfile, "",
    "If this argument is provided, write the .map output of the test map to "
    "the specified file. Useful for testing other parts of the pipeline.");

namespace deepmind {
namespace lab {
namespace {

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
  std::string actual = TranslateTextLevel(kEntities, kVariations, &rng, NoOp);

  ASSERT_THAT(actual, testing::HasSubstr("\"classname\" \"info_player_start\""));
  ASSERT_THAT(actual, testing::HasSubstr("\"classname\" \"worldspawn\""));
  ASSERT_THAT(actual, testing::HasSubstr("\"classname\" \"light\""));
  ASSERT_THAT(actual, testing::HasSubstr("\"classname\" \"func_door\""));
  ASSERT_THAT(actual, testing::HasSubstr("\"classname\" \"trigger_multiple\""));

  if (base::SpecifiedOnCommandLine("map_outfile")) {
    std::string filename = base::GetFlag(FLAGS_map_outfile);
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
  std::string actual = TranslateTextLevel(kEntities, kVariations, &rng, NoOp);

  std::ifstream golden(
      TestSrcDir() +
      "/deepmind/level_generation/text_level/"
      "translate_text_level_test.golden_output");
  QCHECK(golden) << "Failed to open golden data file.";

  std::string expected(std::istreambuf_iterator<char>(golden), {});
  EXPECT_EQ(expected, actual);
}

TEST(TranslateTextLevel, Custom) {
  auto callback = [](std::size_t i, std::size_t j, char ent,
                     const MapSnippetEmitter& em,
                     std::vector<std::string>* out) {
    if (ent != 'x') return false;
    out->push_back(em.AddEntity(i, j, "XyzzyEntity", {{"a", "1"}, {"b", "2"}}));
    return true;
  };

  std::mt19937_64 rng(123);
  std::string actual = TranslateTextLevel(
      kEntities, kVariations, &rng, callback);

  EXPECT_THAT(actual, testing::HasSubstr(
      "{\n"
      "  \"classname\" \"XyzzyEntity\"\n"
      "  \"a\" \"1\"\n"
      "  \"b\" \"2\"\n"
      "  \"origin\" \"650 350 30\"\n"
      "}"));
}

}  // namespace
}  // namespace lab
}  // namespace deepmind
