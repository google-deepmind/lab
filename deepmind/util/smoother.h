// Copyright (C) 2018 Google Inc.
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

#ifndef DML_DEEPMIND_UTIL_SMOOTHER_H_
#define DML_DEEPMIND_UTIL_SMOOTHER_H_

#include <cmath>
#include <cstdlib>
#include <iostream>

namespace deepmind {
namespace lab {
namespace util {
namespace internal {

// Generic smoothing class. Specific implementation specified by SmoothFunctor.
template <typename T, typename DiffT, typename SmoothFunctor>
class Smoother {
 public:
  // 'smooth_time' is used by the SmoothFunctor as part of the smoothing
  // process. Larger means converges faster. 'start' is the starting value for
  // the smoother.
  Smoother(double smooth_time, const T& start)
      : target_(start),
        current_(start),
        velocity_(start - start),  // Start with zero velocity.
        smooth_functor_(smooth_time) {}

  // Set target to smooth towards.
  void set_target(T target) { target_ = target; }

  // Current position of smoother. Updated during 'Update' to move towards
  // target().
  const T& current() const { return current_; }

  void set_velocity(const DiffT& velocity) { velocity_ = velocity; }
  void set_current(T current) { current_ = current; }
  const T& target() const { return target_; }
  const DiffT& velocity() const { return velocity_; }

  // Sets current to target and clears velocity;
  void SnapToTarget() {
    current_ = target_;
    velocity_ = current_ - current_;
  }

  // Update current_ to target_.
  void Update(double delta_time) {
    smooth_functor_.Update(delta_time, target_, &velocity_, &current_);
  }

 private:
  T target_;        // Value smoothing towards.
  T current_;       // Smoothed value
  DiffT velocity_;  // velocity of current_ towards target_.
  SmoothFunctor smooth_functor_;
};

class CriticalDamper {
 public:
  // 'smooth_time' is used to calculate the spring strength.
  // If 'smooth_time' is greater than zero the spring strength will be:
  // (2.0 * mass / smooth_time)^2.
  // Otherwise a special case is observed and 'current()' will be set to the
  // 'target()' and 'velocity()' will be set to zero instantly after calls to
  // 'Update(delta_time)' no matter the 'delta_time'.
  explicit CriticalDamper(double smooth_time)
      : omega_(smooth_time > 0.0 ? 2.0 / smooth_time : 0) {}
  // Models a critically damped spring with omega_ (equal to sqrt(k/m)).
  template <typename T, typename DiffT>
  void Update(double delta_time, const T& target, DiffT* velocity, T* current) {
    if (omega_ <= 0) {
      *current = target;
      *velocity = target - target;  // Zero the velocity.
    } else if (delta_time > 0.0) {
      DiffT gap = *current - target;
      DiffT temp = (*velocity + omega_ * gap) * delta_time;
      double e = std::exp(-omega_ * delta_time);
      *velocity = (*velocity - omega_ * temp) * e;
      *current = target + (gap + temp) * e;
    }
  }

 private:
  double omega_;
};

// Frame-rate independent model of approaching target by taking the gap and
// gap *= 1.0 - 1.0 / (smooth_time + 1.0) every frame at 60 fps.
class ExponentialDecayAt60 {
 public:
  explicit ExponentialDecayAt60(double smooth_time) {
    if (smooth_time > 0.0) {
      double ratio = 1.0 - 1.0 / (smooth_time + 1.0);
      decay_rate_ = -60.0 * std::log(ratio);
      inv_at_60_decay_ = 1.0 / ratio;
    } else {
      decay_rate_ = 0;
      inv_at_60_decay_ = 0;
    }
  }

  template <typename T, typename DiffT>
  void Update(double delta_time, const T& target, DiffT* velocity, T* current) {
    if (decay_rate_ <= 0) {
      *current = target;
      *velocity = target - target;
    } else if (delta_time > 0.0) {
      double decay = std::exp(-delta_time * decay_rate_);
      DiffT gap = *current - target;
      DiffT new_gap = decay * gap;
      DiffT prev_gap_at60 = new_gap * inv_at_60_decay_;
      *velocity = (new_gap - prev_gap_at60) * 60.0;
      *current = target + new_gap;
    }
  }

 private:
  double decay_rate_;
  double inv_at_60_decay_;
};

}  // namespace internal

inline double ConvertExpAt60FpsToSmoothTime(double e) { return (1.0 - e) / e; }

// 'smooth_time' must be greater or equal to 0.
template <typename T, typename DiffT = T>
using SmoothCriticalDamped =
    internal::Smoother<T, DiffT, internal::CriticalDamper>;

// 'smooth_time' must be in range [0, 1].
template <typename T, typename DiffT = T>
using SmoothExponentialDecayAt60 =
    internal::Smoother<T, DiffT, internal::ExponentialDecayAt60>;

}  // namespace util
}  // namespace lab
}  // namespace deepmind

#endif  // DML_DEEPMIND_UTIL_SMOOTHER_H_
