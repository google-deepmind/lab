// Copyright (C) 2021 Google Inc.
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

#include "testing/test_renderer.h"

#ifndef DML_TEST_RENDERER
#error "You must define DML_TEST_RENDERER"
#endif

#define DML_QUOTE(X) X
#define DML_CAT_AUX(X, Y) X ## Y
#define DML_CAT(X, Y) DML_CAT_AUX(X, Y)

namespace deepmind {
namespace lab {

const DeepMindLabRenderer_Enum kTestRenderer_you_must_have_only_one =
    DML_CAT(DeepMindLabRenderer_, DML_TEST_RENDERER);

}  // namespace lab
}  // namespace deepmind
