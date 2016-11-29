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

#ifndef DML_DEEPMIND_LUA_CALL_H_
#define DML_DEEPMIND_LUA_CALL_H_

#include "deepmind/lua/lua.h"
#include "deepmind/lua/n_results_or.h"

namespace deepmind {
namespace lab {
namespace lua {

// Calls a Lua function. The Lua stack should be arranged such a way that
// there is a function and nargs items after it:
//
//     ... [function] [arg1] [arg2] ... [argn]
//
// Returns the number of results or an error. (Backtrace generation can be
// suppressed by setting with_traceback to false.)
//
// [-(nargs + 1), +(nresults|0), -]
NResultsOr Call(lua_State* L, int nargs, bool with_traceback = true);

}  // namespace lua
}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_LUA_CALL_H_
