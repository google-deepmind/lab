// Copyright (C) 2018 Google Inc.
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

#include "deepmind/util/file_reader.h"

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <string>

#include "absl/log/check.h"
#include "deepmind/util/default_read_only_file_system.h"
#include "deepmind/util/files.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "public/file_reader_types.h"

namespace deepmind {
namespace lab {
namespace util {
namespace {

class FileReaderTest : public ::testing::Test {
 protected:
  FileReaderTest() {
    std::string temp_dir = GetTempDirectory();
    root_path_ = temp_dir + "/dmlab_filereader_test";
    CHECK(MakeDirectory(root_path_));
    std::string content;
    contents_.reserve(30);
    contents_.append(10, '1');
    contents_.append(10, '2');
    contents_.append(10, '3');
    path_ = root_path_ + "/content";
    SetContents(path_, contents_, temp_dir.c_str());
  }
  ~FileReaderTest() {
    RemoveDirectory(root_path_);
  }
  std::string path_;
  std::string root_path_;
  std::string contents_;
};

TEST_F(FileReaderTest, ReadWholeFile) {
  FileReader file(DefaultReadOnlyFileSystem(), path_.c_str());
  ASSERT_TRUE(file.Success()) << file.Error();
  std::size_t size;
  ASSERT_TRUE(file.GetSize(&size)) << file.Error();
  EXPECT_EQ(size, 30);
  std::string result(size, '\0');
  EXPECT_TRUE(file.Read(0, size, &result[0]));
  EXPECT_EQ(contents_, result);
}

TEST_F(FileReaderTest, ReadFirst10Bytes) {
  FileReader file(DefaultReadOnlyFileSystem(), path_.c_str());
  ASSERT_TRUE(file.Success()) << file.Error();
  std::size_t size;
  ASSERT_TRUE(file.GetSize(&size)) << file.Error();
  EXPECT_EQ(size, 30);
  std::string result;
  result.resize(10, '\0');
  EXPECT_TRUE(file.Read(0, 10, &result[0]));
  EXPECT_EQ(contents_.substr(0, 10), result);
}

TEST_F(FileReaderTest, ReadMiddle10Bytes) {
  FileReader file(DefaultReadOnlyFileSystem(), path_.c_str());
  ASSERT_TRUE(file.Success()) << file.Error();
  std::size_t size;
  ASSERT_TRUE(file.GetSize(&size)) << file.Error();
  EXPECT_EQ(size, 30);
  std::string result;
  result.resize(10, '\0');
  EXPECT_TRUE(file.Read(10, 10, &result[0]));
  EXPECT_EQ(contents_.substr(10, 10), result);
}

TEST_F(FileReaderTest, ReadLast10Bytes) {
  FileReader file(DefaultReadOnlyFileSystem(), path_.c_str());
  ASSERT_TRUE(file.Success()) << file.Error();
  std::size_t size;
  ASSERT_TRUE(file.GetSize(&size)) << file.Error();
  EXPECT_EQ(size, 30);
  std::string result;
  result.resize(10, '\0');
  EXPECT_TRUE(file.Read(20, 10, &result[0]));
  EXPECT_EQ(contents_.substr(20, 10), result);
}

TEST_F(FileReaderTest, DeleteFileBefore) {
  std::remove(path_.c_str());
  FileReader file(DefaultReadOnlyFileSystem(), path_.c_str());
  ASSERT_FALSE(file.Success());
  EXPECT_THAT(std::string(file.Error()),
              testing::HasSubstr("Failed to open file"));
}

TEST_F(FileReaderTest, DeleteFileAfterOpen) {
  FileReader file(DefaultReadOnlyFileSystem(), path_.c_str());
  ASSERT_TRUE(file.Success());
  std::remove(path_.c_str());
  // Expect file handle to hold file.
  std::size_t size;
  EXPECT_TRUE(file.GetSize(&size)) << file.Error();
}

TEST_F(FileReaderTest, ReadTooManyBytes) {
  FileReader file(DefaultReadOnlyFileSystem(), path_.c_str());
  ASSERT_TRUE(file.Success());
  std::size_t size;
  EXPECT_TRUE(file.GetSize(&size)) << file.Error();
  std::string result;
  result.resize(size + 1, '\0');
  EXPECT_FALSE(file.Read(1, size, &result[0]));
  EXPECT_THAT(std::string(file.Error()),
              testing::HasSubstr("Failed to read from"));
}

extern "C" {

static bool fake_file_system_open(const char* filename,
                                  DeepMindReadOnlyFileHandle* handle) {
  *handle = 0;
  return true;
}

static bool fake_file_system_get_size(DeepMindReadOnlyFileHandle handle,
                                      size_t* size) {
  *size = 10;
  return true;
}

static bool fake_file_system_read(DeepMindReadOnlyFileHandle handle,
                                  size_t offset, size_t count,
                                  char* dest_buff) {
  if (offset + count <= 10) {
    std::generate_n(dest_buff, count, [] { return 'a'; });
    return true;
  } else {
    return false;
  }
}
static const char* fake_file_system_error(DeepMindReadOnlyFileHandle handle) {
  return "";
}

static void fake_file_system_close(DeepMindReadOnlyFileHandle* handle) {}

}  // extern "C"

constexpr DeepMindReadOnlyFileSystem kReadOnlyFileSystem = {
    fake_file_system_open,      //
    fake_file_system_get_size,  //
    fake_file_system_read,      //
    fake_file_system_error,     //
    fake_file_system_close,     //
};

TEST_F(FileReaderTest, FakeFsReader) {
  {
    FileReader file(&kReadOnlyFileSystem, path_.c_str());
    ASSERT_TRUE(file.Success()) << file.Error();
    std::size_t size;
    ASSERT_TRUE(file.GetSize(&size)) << file.Error();
    EXPECT_EQ(size, 10);
    std::string result;
    result.resize(10, '\0');
    EXPECT_TRUE(file.Read(0, 10, &result[0]));
    EXPECT_EQ(result, std::string(10, 'a'));
  }
  {
    FileReader file(DefaultReadOnlyFileSystem(), path_.c_str());
    ASSERT_TRUE(file.Success()) << file.Error();
    std::size_t size;
    ASSERT_TRUE(file.GetSize(&size)) << file.Error();
    EXPECT_EQ(size, 30);
    std::string result;
    result.resize(10, '\0');
    EXPECT_TRUE(file.Read(0, 10, &result[0]));
    EXPECT_EQ(result, std::string(10, '1'));
  }
}

extern "C" {

static bool fake_file_system_size_error_get_size(
    DeepMindReadOnlyFileHandle handle, size_t* size) {
  return false;
}

static const char* fake_file_system_size_error_error(
    DeepMindReadOnlyFileHandle handle) {
  return "Test error message - size error!";
}

}  // extern "C"

constexpr DeepMindReadOnlyFileSystem kFakeFileSystemSizeError = {
    fake_file_system_open,                 //
    fake_file_system_size_error_get_size,  //
    fake_file_system_read,                 //
    fake_file_system_size_error_error,     //
    fake_file_system_close,                //
};

TEST_F(FileReaderTest, TestSizeError) {
  FileReader file(&kFakeFileSystemSizeError, path_.c_str());
  ASSERT_TRUE(file.Success());
  std::size_t size;
  EXPECT_FALSE(file.GetSize(&size));
  EXPECT_THAT(std::string(file.Error()),
              testing::HasSubstr("Test error message - size error!"));
}

}  // namespace
}  // namespace util
}  // namespace lab
}  // namespace deepmind
