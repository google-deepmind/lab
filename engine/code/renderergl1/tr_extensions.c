// Copyright (C) 2017 Google Inc.
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

#include "tr_local.h"

#define GLE(ret, name, ...) name##proc *qgl##name = NULL;
QGL_1_3_PROCS;
QGL_1_5_PROCS;
#undef GLE

void GLimp_InitExtraExtensions() {
#define GLE(ret, name, ...)                                     \
  qgl##name = (name##proc *)GLimp_GetProcAddress("gl" #name);
  QGL_1_3_PROCS;
  QGL_1_5_PROCS;
#undef GLE

}
