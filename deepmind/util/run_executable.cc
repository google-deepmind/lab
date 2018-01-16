#include "deepmind/util/run_executable.h"

#include <sys/types.h>
#include <sys/wait.h>

#include <array>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string>

#include "deepmind/support/logging.h"
#include "absl/strings/str_cat.h"

namespace {

// Produce a human-readable description of the platform-dependent result of
// running the command line on the system. On our Linux, uses the semantics of
// wait(2).
bool ParseStatus(int s, std::string* msg) {
  if (s == -1) {
    LOG(QFATAL) << "Failed to call the system. " << std::strerror(errno);
  } else if (WIFEXITED(s)) {
    int retval = WEXITSTATUS(s);
    if (retval == 0) {
      *msg = "exited successfully (return value 0)";
      return true;
    } else if (retval == 127) {
      *msg = absl::StrCat("system() failed to run command. ", retval);
      return false;
    } else {
      *msg = absl::StrCat("exited with failure, return value ", retval);
      return false;
    }
  } else if (WIFSIGNALED(s)) {
    int signum = WTERMSIG(s);
    *msg = absl::StrCat("exited with signal ", signum);
    return false;
  } else {
    LOG(QFATAL) << "The system returned something implausible.";
  }
}

}  // namespace

namespace deepmind {
namespace lab {
namespace util {

bool RunExecutable(const char* command_line, std::string* message) {
  CHECK(command_line != nullptr) << "Must provide command_line!";
  LOG(INFO) << "Running command:\n" << command_line << "\n";
  return ParseStatus(std::system(command_line), message);
}

bool RunExecutableWithOutput(const char* command_line, std::string* message,
                             std::string* output) {
  CHECK(command_line != nullptr) << "Must provide command_line!";
  int& err = errno;
  err = 0;
  if (FILE* pipe = popen(command_line, "r")) {
    std::array<char, 4096> buffer;
    while (size_t read = std::fread(buffer.data(), 1, buffer.size(), pipe)) {
      output->append(buffer.data(), read);
    }
    return ParseStatus(pclose(pipe), message);
  } else {
    *message = "Failed to run command!\n";
    if (err != 0) {
      *message += std::strerror(err);
    }
    return false;
  }
}

}  // namespace util
}  // namespace lab
}  // namespace deepmind
