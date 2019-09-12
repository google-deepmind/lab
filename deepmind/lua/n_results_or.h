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

#ifndef DML_DEEPMIND_LUA_N_RESULTS_OR_H_
#define DML_DEEPMIND_LUA_N_RESULTS_OR_H_

#include <string>
#include <utility>

namespace deepmind {
namespace lab {
namespace lua {

// Used for returning the number of return values from a Lua function call or an
// error message. Implicit construction aids readability in call sites.
class NResultsOr {
 public:
  // Function call completed without error and n return values on the stack.
  NResultsOr(int n_results) : n_results_(n_results) {}

  // Function call failled with error message `error`.
  // Make sure no results are left on the stack, so that n_results() can be
  // popped regardless of error.
  NResultsOr(std::string error) : n_results_(0), error_(std::move(error)) {
    if (error_.empty()) {
      error_ = "(nil)";
    }
  }
  NResultsOr(const char* error) : NResultsOr(std::string(error)) {}

  int n_results() const { return n_results_; }
  bool ok() const { return error_.empty(); }
  const std::string& error() const { return error_; }

 private:
  int n_results_;
  std::string error_;  // If empty no error;
};

}  // namespace lua
}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_LUA_N_RESULTS_OR_H_
