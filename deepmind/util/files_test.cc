#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <string>

#include "gtest/gtest.h"
#include "deepmind/util/files.h"

namespace deepmind {
namespace lab {
namespace util {
namespace {

std::string TestName() {
  return ::testing::UnitTest::GetInstance()->current_test_info()->name();
}

TEST(FilesTest, RemovesDirectory) {
  // Ensures RemoveDirectory removes directories.
  std::string root_path(GetTempDirectory() + "/" + TestName());
  mkdir(root_path.c_str(), 0666);
  std::string new_path(root_path + "/b");
  mkdir(new_path.c_str(), 0666);
  new_path += "/c";
  creat(new_path.c_str(), 0666);

  EXPECT_EQ(access(root_path.c_str(), F_OK), 0);

  RemoveDirectory(root_path);

  EXPECT_EQ(access(root_path.c_str(), F_OK), -1);
}

TEST(FilesTest, RemovesDirectoryNoSymlink) {
  // Ensures RemoveDirectory does not follow symlinks.
  std::string temp_directory = GetTempDirectory();
  std::string root_path(GetTempDirectory() + "/" + TestName());
  mkdir(root_path.c_str(), 0666);
  std::string new_path(root_path + "/b");
  mkdir(new_path.c_str(), 0666);
  new_path += "/c";
  creat(new_path.c_str(), 0666);
  std::string link_path(temp_directory + "/l");
  symlink(root_path.c_str(), link_path.c_str());
  RemoveDirectory(link_path);
  EXPECT_EQ(access(root_path.c_str(), F_OK), 0);
  RemoveDirectory(root_path);
}

TEST(FilesTest, CreateAndRemoveDirectory) {
  // Ensures RemoveDirectory does not follow symlinks.
  std::string root_path(GetTempDirectory() + "/" + TestName());
  std::string new_path(root_path + "/b/b/b");
  EXPECT_TRUE(MakeDirectory(new_path));
  RemoveDirectory(root_path);
}

TEST(FilesTest, CreateFileWithContents) {
  std::string temp_dir = GetTempDirectory();
  std::string root_path(temp_dir + "/dmlab_files_test");
  ASSERT_TRUE(MakeDirectory(root_path));

  std::string contents(1000000, '@');
  SetContents(root_path + "/content", contents, temp_dir.c_str());

  std::string result;
  GetContents(root_path + "/content", &result);

  // Don't print large file.
  EXPECT_TRUE(result == contents);

  RemoveDirectory(root_path);
}

}  // namespace
}  // namespace util
}  // namespace lab
}  // namespace deepmind
