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

#ifndef DML_TESTING_ENV_OBSERVATION_H_
#define DML_TESTING_ENV_OBSERVATION_H_

#include <vector>

#include "public/dmlab.h"
#include "third_party/rl_api/env_c_api.h"

namespace deepmind {
namespace lab {
namespace internal {

// ReadPayload returns a copy of the payload from the observation provided.
template<typename T> std::vector<T> ReadPayload(const EnvCApi_Observation& obs);
template<> std::vector<double> ReadPayload(const EnvCApi_Observation& obs);
template<> std::vector<unsigned char> ReadPayload(
    const EnvCApi_Observation& obs);

}  // namespace internal

// A wrapper for EnvCApi_Observation that copies and saves the observation.
template <typename T>
class EnvObservation {
 public:
  explicit EnvObservation(const EnvCApi_Observation& obs) :
    payload_(internal::ReadPayload<T>(obs)),
    shape_(obs.spec.shape, obs.spec.shape + obs.spec.dims) {}

  const std::vector<T>& payload() const { return payload_; }
  const std::vector<int>& shape() const { return shape_; }

 private:
  std::vector<T> payload_;
  std::vector<int> shape_;
};

}  // namespace lab
}  // namespace deepmind

#endif  // DML_TESTING_ENV_OBSERVATION_H_
