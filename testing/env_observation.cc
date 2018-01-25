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

#include <cassert>
#include <functional>
#include <numeric>

#include "public/dmlab.h"
#include "testing/env_observation.h"

namespace deepmind {
namespace lab {
namespace internal {
namespace {

int PayloadLength(const EnvCApi_Observation& obs) {
  return std::accumulate(obs.spec.shape, obs.spec.shape + obs.spec.dims,
                         obs.spec.dims ? 1 : 0, std::multiplies<int>());
}

}  // namespace

template<> std::vector<double> ReadPayload(const EnvCApi_Observation& obs) {
  assert(obs.spec.type == EnvCApi_ObservationDoubles);
  return std::vector<double>(obs.payload.doubles,
                             obs.payload.doubles + PayloadLength(obs));
}

template<> std::vector<unsigned char> ReadPayload(
    const EnvCApi_Observation& obs) {
  assert(obs.spec.type == EnvCApi_ObservationBytes);
  return std::vector<unsigned char>(obs.payload.bytes,
                                    obs.payload.bytes + PayloadLength(obs));
}

}  // namespace internal
}  // namespace lab
}  // namespace deepmind
