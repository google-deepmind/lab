#ifndef DML_DEEPMIND_UTIL_RUN_EXECUTABLE_H_
#define DML_DEEPMIND_UTIL_RUN_EXECUTABLE_H_

#include <string>

namespace deepmind {
namespace lab {
namespace util {

// Runs the provided 'command_line' on the system.
// 'message' will contain a human-readable description of the platform-dependent
// result of running the command. On our Linux, uses the semantics of
// wait(2).
// Returns whether the command ran successfully.
bool RunExecutable(const char* command_line, std::string* message);

// Runs the provided 'command_line' on the system.
// 'output' will be appended with the output of stdout.
// 'message' will contain a human-readable description of the platform-dependent
// result of running the command. On our Linux, uses the semantics of
// wait(2).
// Returns whether the command ran successfully.
bool RunExecutableWithOutput(const char* command_line, std::string* message,
                             std::string* output);

}  // namespace util
}  // namespace lab
}  // namespace deepmind
#endif  // DML_DEEPMIND_UTIL_RUN_EXECUTABLE_H_
