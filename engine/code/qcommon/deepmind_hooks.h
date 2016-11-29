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
// DeepMind hook into the game engine, used for per-level customization.

#ifndef DML_ENGINE_CODE_QCOMMON_DEEPMIND_HOOKS_H_
#define DML_ENGINE_CODE_QCOMMON_DEEPMIND_HOOKS_H_

#include "q_shared.h"

// Entry point for the "DeepMind callback" syscall from the VMs. This is a
// low-level service routine that uses dm_callnum to multiplex a diverse set
// of facilities onto a single function call. Twelve generic arguments are
// supported.
int dmlab_callback(
    int dm_callnum,
    intptr_t a1,  intptr_t a2,  intptr_t a3,  intptr_t a4,
    intptr_t a5,  intptr_t a6,  intptr_t a7,  intptr_t a8,
    intptr_t a9, intptr_t a10, intptr_t a11, intptr_t a12);

#endif  // DML_ENGINE_CODE_QCOMMON_DEEPMIND_HOOKS_H_
