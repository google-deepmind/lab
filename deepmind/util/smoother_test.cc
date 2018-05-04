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

#include "deepmind/util/smoother.h"

#include "gtest/gtest.h"
#include "Eigen/Dense"

namespace deepmind {
namespace lab {
namespace util {
namespace {

TEST(SmootherTest, TimeStepSizeInvariant) {
  for (int test = 0; test < 1; ++test) {
    double target = std::pow(2.0, test);
    SmoothCriticalDamped<double> damper1(/*smooth_time=*/2.0 / 0.5,
                                         /*start=*/0.0);
    damper1.set_target(target);
    SmoothCriticalDamped<double> damper2 = damper1;
    damper1.Update(10.0);
    for (int i = 0; i < 100; ++i) {
      damper2.Update(0.1);
    }
    EXPECT_NEAR(damper1.current(), damper2.current(), 1e-5);
    EXPECT_NEAR(damper1.velocity(), damper2.velocity(), 1e-5);
  }
}

TEST(SmootherTest, TimeStepSizeInvariantExpDecay) {
  for (int test = 0; test < 1; ++test) {
    double target = std::pow(2.0, test);
    SmoothExponentialDecayAt60<double> damper1(
        /*smooth_time=*/ConvertExpAt60FpsToSmoothTime(0.1), /*start=*/0.0);
    damper1.set_target(target);
    SmoothExponentialDecayAt60<double> damper2 = damper1;
    damper1.Update(10.0);
    for (int i = 0; i < 100; ++i) {
      damper2.Update(0.1);
    }
    EXPECT_NEAR(damper1.current(), damper2.current(), 1e-5);
    EXPECT_NEAR(damper1.velocity(), damper2.velocity(), 1e-5);
  }
}

TEST(SmootherTest, ApproachesTarget) {
  for (int test = 0; test < 1; ++test) {
    double target = std::pow(2.0, test);
    SmoothCriticalDamped<double> damper(/*smooth_time=*/4.0, /*start=*/0.0);
    damper.set_target(target);
    double old_current = damper.current();
    for (int i = 0; i < 200; ++i) {
      damper.Update(0.1);
      EXPECT_LT(old_current, damper.current()) << i;
      old_current = damper.current();
    }
    EXPECT_NEAR(damper.current(), target, 1e-3 * target);
  }
}

TEST(SmootherTest, SnapToTarget) {
  double target = 20.0;
  SmoothCriticalDamped<double> snap(/*smooth_time=*/0.0, /*start=*/0.0);
  SmoothCriticalDamped<double> damper(/*smooth_time=*/2.0, /*start=*/0.0);
  damper.set_target(target);
  for (int i = 0; i < 200; ++i) {
    damper.Update(0.1);
    snap.set_target(damper.current());
    EXPECT_NE(damper.current(), snap.current());
    snap.Update(0.1);
    EXPECT_EQ(damper.current(), snap.current());
  }
}

TEST(SmootherTest, PreCalculated) {
  SmoothCriticalDamped<double> damper(/*smooth_time=*/2.0 / 6.0, /*start=*/0.0);
  damper.set_target(1.0);
  damper.set_velocity(6.0);
  double dt = 1.0 / 60.0;
  damper.Update(dt);
  EXPECT_NEAR(damper.current(), 0.09516258196, 1e-9);
  damper.Update(dt);
  EXPECT_NEAR(damper.current(), 0.1812692469, 1e-9);
}

TEST(SmootherTest, PreCalculatedExpDecayOneStep) {
  SmoothExponentialDecayAt60<double> damper(
      /*smooth_time=*/ConvertExpAt60FpsToSmoothTime(0.1), /*start=*/0.0);
  damper.set_target(1.0);
  damper.set_velocity(0.0);
  double dt2 = 2.0 / 60.0;
  damper.Update(dt2);
  EXPECT_NEAR(damper.velocity(), 0.18 / dt2, 1e-9);
  EXPECT_NEAR(damper.current(), 1.0 - 0.81, 1e-9);
}

TEST(SmootherTest, PreCalculatedExpDecayTwoStep) {
  SmoothExponentialDecayAt60<double> damper(
      /*smooth_time=*/ConvertExpAt60FpsToSmoothTime(0.1), /*start=*/0.0);
  damper.set_target(1.0);
  damper.set_velocity(0.0);
  double dt = 1.0 / 60.0;
  damper.Update(dt);
  EXPECT_NEAR(damper.current(), 0.1, 1e-9);
  damper.Update(dt);
  EXPECT_NEAR(damper.velocity(), 0.18 / (dt * 2.0), 1e-9);
  EXPECT_NEAR(damper.current(), 1.0 - 0.81, 1e-9);
}

TEST(SmootherTest, Vector3d) {
  SmoothCriticalDamped<Eigen::Vector3d> damper(2.0 / 6.0, {0.0, 0.0, 0.0});
  damper.set_target({1.0, 2.0, 3.0});
  damper.set_velocity({6.0, 6.0, 6.0});
  damper.Update(1.0 / 60.0);
  EXPECT_NEAR(damper.current()[0], 0.09516258196, 1e-9);
}

TEST(SmootherTest, VectorExp3d) {
  SmoothExponentialDecayAt60<Eigen::Vector3d> damper(
      /*smooth_time=*/ConvertExpAt60FpsToSmoothTime(0.1),
      /*start=*/{0.0, 0.0, 0.0});
  damper.set_target({1.0, 2.0, 3.0});
  damper.Update(1.0 / 60.0);
  EXPECT_NEAR(damper.current()[0], 0.1, 1e-9);
}

// Angle class in degrees representing the range [-180, 180).
class AngleDegrees {
 public:
  explicit AngleDegrees(double angle) : angle_(Normalize180(angle)) {}

  // Shotest angular path favouring -180.0.
  double operator-(AngleDegrees rhs) const {
    return Normalize180(angle_ - rhs.angle_);
  }

  AngleDegrees operator-(double deltaAngle) const {
    return AngleDegrees(angle_ - deltaAngle);
  }

  AngleDegrees operator+(double deltaAngle) const {
    return AngleDegrees(angle_ + deltaAngle);
  }

  double angle() const { return angle_; }

 private:
  // Returns value in range [-180, 180)
  static double Normalize180(double in) {
    return in - 360 * std::floor((in + 180.0) / 360.0);
  }

  double angle_;
};

TEST(SmootherTest, AngleDegrees) {
  SmoothCriticalDamped<AngleDegrees, double> damper(2.0 / 6.0,
                                                    AngleDegrees(160.0));
  damper.set_target(AngleDegrees(-160));
  for (int i = 0; i < 100; ++i) {
    damper.Update(1.0 / 60.0);
    double val = damper.current().angle();
    if (val > 0) {
      EXPECT_GT(val, 160);
    } else {
      EXPECT_LT(val, -160);
    }
  }
}

}  // namespace
}  // namespace util
}  // namespace lab
}  // namespace deepmind
