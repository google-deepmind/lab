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
//
// A minimal replacement for "glog"-like functionality. Does not provide output
// in a separate thread nor backtracing.

#ifndef DEEPMIND_SUPPORT_LOGGING_H
#define DEEPMIND_SUPPORT_LOGGING_H

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>

#ifdef __GNUC__
#  define PREDICT_TRUE(x) (__builtin_expect(!!(x), 1))
#  define PREDICT_FALSE(x) (__builtin_expect(x, 0))
#  define NORETURN __attribute__((noreturn))
#else
#  define PREDICT_TRUE(x) (x)
#  define PREDICT_FALSE(x) (x)
#  define NORETURN
#endif

namespace deepmind {
namespace internal {

struct CheckOpString {
  explicit CheckOpString(std::string* str) : str_(str) { }
  explicit operator bool() const { return PREDICT_FALSE(str_ != nullptr); }
  std::string* const str_;
};

template <typename T1, typename T2>
CheckOpString MakeCheckOpString(const T1& v1, const T2& v2,
                                const char* exprtext) {
  std::ostringstream oss;
  oss << exprtext << " (" << v1 << " vs. " << v2 << ")";
  return CheckOpString(new std::string(oss.str()));
}

#define DEFINE_CHECK_OP_IMPL(name, op)                                         \
  template <typename T1, typename T2>                                          \
  inline CheckOpString name##Impl(const T1& v1, const T2& v2,                  \
                                  const char* exprtext) {                      \
    if (PREDICT_TRUE(v1 op v2)) return CheckOpString(nullptr);                 \
    else return (MakeCheckOpString)(v1, v2, exprtext);                         \
  }                                                                            \
  inline CheckOpString name##Impl(int v1, int v2, const char* exprtext) {      \
    return (name##Impl<int, int>)(v1, v2, exprtext);                           \
  }
DEFINE_CHECK_OP_IMPL(Check_EQ, ==)
DEFINE_CHECK_OP_IMPL(Check_NE, !=)
DEFINE_CHECK_OP_IMPL(Check_LE, <=)
DEFINE_CHECK_OP_IMPL(Check_LT, < )
DEFINE_CHECK_OP_IMPL(Check_GE, >=)
DEFINE_CHECK_OP_IMPL(Check_GT, > )
#undef DEFINE_CHECK_OP_IMPL

class LogMessage {
 public:
  LogMessage(const char* file, int line) {
    std::clog << "[" << file << ":" << line << "] ";
  }

  std::ostream& stream() && { return std::clog; }
};

class LogMessageFatal {
 public:
  LogMessageFatal(const char* file, int line) {
    stream_ << "[" << file << ":" << line << "] ";
  }

  LogMessageFatal(const char* file, int line, const CheckOpString& result) {
    stream_ << "[" << file << ":" << line << "] Check failed: " << *result.str_;
  }

  ~LogMessageFatal() NORETURN;

  std::ostream& stream() && { return stream_; }

 private:
  std::ostringstream stream_;
};

inline LogMessageFatal::~LogMessageFatal() {
  std::cerr << stream_.str() << std::endl;
  std::abort();
}

struct NullStream {};

template <typename T>
NullStream&& operator<<(NullStream&& s, T&&) { return std::move(s); }

enum class LogLevel {
  kFatal,
  kNonFatal,
};

LogMessage LogStream(std::integral_constant<LogLevel, LogLevel::kNonFatal>);
LogMessageFatal LogStream(std::integral_constant<LogLevel, LogLevel::kFatal>);

}  // namespace internal
}  // namespace deepmind


#define CHECK_OP_LOG(name, op, val1, val2, log)         \
  while (::deepmind::internal::CheckOpString _result =  \
         ::deepmind::internal::name##Impl(              \
              val1, val2, #val1 " " #op " " #val2))     \
    log(__FILE__, __LINE__, _result).stream()

#define CHECK_OP(name, op, val1, val2) \
  CHECK_OP_LOG(name, op, val1, val2, ::deepmind::internal::LogMessageFatal)

#define CHECK_EQ(val1, val2) CHECK_OP(Check_EQ, ==, val1, val2)
#define CHECK_NE(val1, val2) CHECK_OP(Check_NE, !=, val1, val2)
#define CHECK_LE(val1, val2) CHECK_OP(Check_LE, <=, val1, val2)
#define CHECK_LT(val1, val2) CHECK_OP(Check_LT, <,  val1, val2)
#define CHECK_GE(val1, val2) CHECK_OP(Check_GE, >=, val1, val2)
#define CHECK_GT(val1, val2) CHECK_OP(Check_GT,  >, val1, val2)

#define QCHECK_EQ(val1, val2) CHECK_OP(Check_EQ, ==, val1, val2)
#define QCHECK_NE(val1, val2) CHECK_OP(Check_NE, !=, val1, val2)
#define QCHECK_LE(val1, val2) CHECK_OP(Check_LE, <=, val1, val2)
#define QCHECK_LT(val1, val2) CHECK_OP(Check_LT, <,  val1, val2)
#define QCHECK_GE(val1, val2) CHECK_OP(Check_GE, >=, val1, val2)
#define QCHECK_GT(val1, val2) CHECK_OP(Check_GT,  >, val1, val2)

#define CHECK(condition)                                                       \
  while (auto _result = ::deepmind::internal::CheckOpString(                   \
             (condition) ? nullptr : new std::string(#condition)))             \
    ::deepmind::internal::LogMessageFatal(__FILE__, __LINE__, _result).stream()

#define QCHECK(condition) CHECK(condition)

#define FATAL   ::deepmind::internal::LogLevel::kFatal
#define QFATAL  ::deepmind::internal::LogLevel::kFatal
#define INFO    ::deepmind::internal::LogLevel::kNonFatal
#define WARNING ::deepmind::internal::LogLevel::kNonFatal
#define ERROR   ::deepmind::internal::LogLevel::kNonFatal

#define LOG(level)                                                             \
  decltype(::deepmind::internal::LogStream(                                    \
      std::integral_constant<::deepmind::internal::LogLevel, level>()))        \
  (__FILE__, __LINE__).stream()

#define VLOG(level) ::deepmind::internal::NullStream()

#endif  // DEEPMIND_SUPPORT_LOGGING_H
