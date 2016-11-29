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

#ifndef DEEPMIND_SUPPORT_STR_CAT_H
#define DEEPMIND_SUPPORT_STR_CAT_H

#include <sstream>
#include <string>
#include <utility>

using std::string;

template <typename ...Args>
inline std::string StrCat(const Args&... args) {
  std::ostringstream oss;
  int dummy[] = { 1, (oss << args, 0)... };
  static_cast<void>(dummy);
  return oss.str();
}

template <typename ...Args>
inline void StrAppend(std::string* out, const Args&... args) {
  std::ostringstream oss;
  int dummy[] = { 1, (oss << args, 0)... };
  static_cast<void>(dummy);
  out->append(oss.str());
}

namespace strings {
namespace internal {

template <typename F1, typename F2>
class PairFormatterImpl {
 public:
  PairFormatterImpl(F1 f1, F2 f2, std::string sep)
      : f1_(std::move(f1)),
        f2_(std::move(f2)),
        sep_(std::move(sep)) {}

  template <typename T>
  void operator()(std::string* out, const T& p) {
    f1_(out, p.first);
    out->append(sep_);
    f2_(out, p.second);
  }

 private:
  F1 f1_;
  F2 f2_;
  std::string sep_;
};

}  // namespace internal

template <typename F1, typename F2>
internal::PairFormatterImpl<F1, F2> PairFormatter(
    F1 f1, std::string sep, F2 f2) {
  return internal::PairFormatterImpl<F1, F2>(
      std::move(f1), std::move(f2), std::move(sep));
}

}  // namespace strings

#endif  // DEEPMIND_SUPPORT_STR_CAT_H
